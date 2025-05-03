#include "room.h"

void Room::Init(int room_id)
{
	m_room_id = room_id;
	m_clients.clear();
}

void Room::AddClient(int client_id)
{
	m_clients.push_back(client_id);

	int idx = m_clients.size() - 1;
	g_client[client_id].SetSpawnCoord(idx);
}

void Room::RemoveClient(int client_id)
{
	m_clients.erase(remove(m_clients.begin(), m_clients.end(), client_id), m_clients.end());
	std::lock_guard<std::mutex> lock(m_PlayerMx);
	--m_enterClientNum;
}

void Room::AllPlayerNum(int playerNum)
{
	m_playerNum = playerNum;
}

void Room::PlusPlayerReadyNum()
{
	std::lock_guard<std::mutex> lock(m_PlayerMx);
	++m_enterClientNum;
}

void Room::PlusPlayerZenithReadyNum()
{
	std::lock_guard<std::mutex> lock(m_PlayerMx);
	++m_enterZenithNum;
}

void Room::PushStartGameButton(int RoomMasterID)
{
	if (RoomMasterID != GetRoomMasterID()) {
		std::cout << "[ERROR] 클라이언트 [" << RoomMasterID << "] 는 방장이 아니여서 시작할 수 없습니다.\n";
		// TODO : 클라한테 너 방장 아니여서 시작 못한다는 거 말해줘야 할듯.(나중에)
		return;
	}

	InitChallengeMonsters();									// 도전 스테이지 몬스터 초기화

	g_network.SendWhoIsMyTeam(GetClients());					// 아이디 값 보내주기
	g_network.SendInitialState(GetClients());					// 게임방 안에 본인 포함 모두한테 초기 좌표 패킷 보내기
	g_network.SendInitMonster(GetClients(),GetMonsters());		// 게임방 안의 모든 몬스터 초기화
	Sleep(2000);

	g_network.SendGameStart(GetClients());						// 게임방 안에 본인 포함 모두한테 게임시작 패킷 보내기
	AllPlayerNum(m_clients.size());								// 게임에 입장한 플레이어가 몇명이야?

	m_IsGaming = true;
	std::cout << "[INFO] 방[" << m_room_id << "] 게임을 시작하였습니다!\n";
}

void Room::PushStartZenithButton(int RoomMasterID)
{
	if (RoomMasterID != GetRoomMasterID()) {
		std::cout << "[ERROR] 클라이언트 [" << RoomMasterID << "] 는 방장이 아니여서 시작할 수 없습니다.\n";
		// TODO : 클라한테 너 방장 아니여서 시작 못한다는 거 말해줘야 할듯.(나중에)
		return;
	}
	g_network.SendStartZenithStage(GetClients());			// 게임방 안에 본인 포함 모두한테 게임시작 패킷 보내기
}

void Room::StartGame()
{
	m_RoomState = Stage::CHALLENGE_STAGE;
	m_timer = std::thread(&Room::ChallengeTimerThread, this);
}

void Room::ChallengeTimerThread()
{
	// 도전 스테이지 8분
	std::this_thread::sleep_for(std::chrono::seconds(CHALLENGE_TIME));
	if (m_stopTimer) return;
	RepairTime();
}

void Room::StartZenithStage()
{
	if (m_timer.joinable()) 
		m_timer.join();

	m_RoomState = Stage::ZENITH_STAGE;
	m_timer = std::thread(&Room::ZenithTimerThread, this);
}

void Room::ZenithTimerThread() {
	// 정점 스테이지 5분
	std::this_thread::sleep_for(std::chrono::seconds(ZENITH_TIME));
	if (m_stopTimer) return;
	EndGame();
}

void Room::RepairTime()
{
	m_RoomState = Stage::REPAIR_TIME;
	g_network.SendStartRepairTime(GetClients());	// 다들 시작의 땅으로 이동하자
}

void Room::EndGame()
{
	if (m_timer.joinable()) {
		m_timer.join();
	}
	m_IsGaming = false;
	m_RoomState = Stage::LOBBY;

	std::cout << "[INFO][" << m_room_id << "]방 게임을 종료하였습니다!\n";
	// TODO
}


// 인벤토리
void Room::AddGold(int plusgold)
{
	std::lock_guard<std::mutex> lock(m_inventoryMx);
	m_inventory.gold += plusgold;
	g_network.SendUpdateInventory(GetClients());
	// update packet
}

void Room::SpendGold(int minusgold)
{
	std::lock_guard<std::mutex> lock(m_inventoryMx);
	if (m_inventory.gold > 0)
		m_inventory.gold -= minusgold;
	//else
		// 돈 다 써서 강화 더 못해
	// update packet
}

void Room::ADDJobWeapon(JobWeapon weapon)			
{
	std::lock_guard<std::mutex> lock(m_inventoryMx);
	++m_inventory.JobWeapons[weapon];
	// update packet
}

void Room::DecideJobWeapon(JobWeapon weapon)
{
	std::lock_guard<std::mutex> lock(m_inventoryMx);
	--m_inventory.JobWeapons[weapon];
	// update packet
}

void Room::AddJobDocument(JobDocument job)			
{
	std::lock_guard<std::mutex> lock(m_inventoryMx);
	++m_inventory.JobDocuments[job];
	// update packet
}

void Room::DecideJobDocument(JobDocument job)		// 클라 코드에서 한번 정했으면 자기 직업 정해지니까 바꾸기 못함을 적어야 할듯. (다른 직업이 클릭이 안되게 막음)
{													// 그리고 직업1 전직서가 없으면 클릭조차 안되게 클라자체에서 막자. (서버에서 해야할 일을 애초에 줄이자)
	std::lock_guard<std::mutex> lock(m_inventoryMx);
	--m_inventory.JobDocuments[job];
	// update packet
}

// 몬스터
void Room::InitChallengeMonsters()
{
	m_monsters.clear();
	int monsterID = 0;

	// Mushroom
	{
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::Mushroom,-180.f, 5.f, -160.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::Mushroom,-170.f, 5.f, -160.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::Mushroom,-160.f, 5.f, -160.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::Mushroom,-150.f, 5.f, -160.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::Mushroom,-140.f, 5.f, -160.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::Mushroom,-130.f, 5.f, -160.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::Mushroom,-120.f, 5.f, -160.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::Mushroom,-110.f, 5.f, -160.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::Mushroom,-100.f, 5.f, -160.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::Mushroom,-90.f, 5.f, -160.f };
	}
	// Fight Fly
	{
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FightFly,-220.f, 5.f, -23.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FightFly,-210.f, 5.f, -23.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FightFly,-200.f, 5.f, -23.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FightFly,-190.f, 5.f, -23.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FightFly,-180.f, 5.f, -23.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FightFly,-170.f, 5.f, -23.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FightFly,-160.f, 5.f, -23.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FightFly,-150.f, 5.f, -23.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FightFly,-140.f, 5.f, -23.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FightFly,-130.f, 5.f, -23.f };
	}
	// Plant Dionaea
	{
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantDionaea,80.f, 5.f, 75.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantDionaea,90.f, 5.f, 75.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantDionaea,100.f, 5.f, 75.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantDionaea,110.f, 5.f, 75.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantDionaea,120.f, 5.f, 75.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantDionaea,130.f, 5.f, 75.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantDionaea,140.f, 5.f, 75.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantDionaea,150.f, 5.f, 75.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantDionaea,160.f, 5.f, 75.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantDionaea,170.f, 5.f, 75.f };
	}
	// Pea Shooter
	/* {
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PeaShooter,-60.f, 0.f, 60.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PeaShooter,-60.f, 0.f, 60.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PeaShooter,-60.f, 0.f, 60.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PeaShooter,-60.f, 0.f, 60.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PeaShooter,-60.f, 0.f, 60.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PeaShooter,-60.f, 0.f, 60.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PeaShooter,-60.f, 0.f, 60.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PeaShooter,-60.f, 0.f, 60.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PeaShooter,-60.f, 0.f, 60.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PeaShooter,-60.f, 0.f, 60.f };
	}*/
	// Plant Venus
	{
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantVenus,0.f, 5.f, -35.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantVenus,10.f, 5.f, -35.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantVenus,20.f, 5.f, -35.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantVenus,30.f, 5.f, -35.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantVenus,40.f, 5.f, -35.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantVenus,50.f, 5.f, -35.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantVenus,60.f, 5.f, -35.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantVenus,70.f, 5.f, -35.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantVenus,80.f, 5.f, -35.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantVenus,90.f, 5.f, -35.f };
	}
	// Flower Fairy
	{
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FlowerFairy,160.f, 5.f, -190.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FlowerFairy,165.f, 5.f, -190.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FlowerFairy,170.f, 5.f, -190.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FlowerFairy,175.f, 5.f, -190.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FlowerFairy,180.f, 5.f, -190.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FlowerFairy,185.f, 5.f, -190.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FlowerFairy,190.f, 5.f, -190.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FlowerFairy,195.f, 5.f, -190.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FlowerFairy,200.f, 5.f, -190.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FlowerFairy,205.f, 5.f, -190.f };
	}
	m_MonsterNum = monsterID;
}