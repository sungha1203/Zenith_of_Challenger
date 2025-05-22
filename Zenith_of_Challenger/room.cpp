#include "room.h"

void Room::Init(int room_id)
{
	m_room_id = room_id;
	m_clients.clear();
}

void Room::ResetRoom()
{
	m_IsGaming = false;
	m_RoomState = Stage::LOBBY;         // 초기 상태로
	m_clients.clear();                  // 클라이언트 목록 제거
	m_inventory = InventoryItem();

	m_skipTimer = false;
	m_skipButton = false;
	m_playerNum = 0;
	m_enterClientNum = 0;
	m_enterZenithNum = 0;

	for (auto& monster : m_monsters)
		monster.Reset();
	m_MonsterNum = 0;;
	m_timer.join();
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
	//std::this_thread::sleep_for(std::chrono::milliseconds(500));
	g_network.SendInitialState(GetClients());					// 게임방 안에 본인 포함 모두한테 초기 좌표 패킷 보내기
	//std::this_thread::sleep_for(std::chrono::milliseconds(500));
	g_network.SendInitMonster(GetClients(),m_monsters);			// 게임방 안의 모든 몬스터 초기화
	//std::this_thread::sleep_for(std::chrono::milliseconds(500));
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
	for (int i = 0; i < CHALLENGE_TIME; ++i) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		if (m_stopTimer) return;
		if (m_skipTimer) break;
	}
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
	for (int i = 0; i < GetClientsNum(); ++i) {		// 시작의 땅이 어딘데
		g_client[m_clients[i]].SetRepairCoord(i);
	}
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

void Room::SetStopTimer()
{
	m_stopTimer = true;
}

void Room::SetSkipTimer()
{
	m_skipTimer = true;
}

void Room::SetSkipButton(bool check)
{
	m_skipButton = check;
}


// 인벤토리
void Room::AddGold(int plusgold)
{
	std::lock_guard<std::mutex> lock(m_inventoryMx);
	m_inventory.gold += plusgold;
	g_network.SendUpdateGold(GetClients());
	// update packet
}

void Room::SpendGold(int minusgold)
{
	std::lock_guard<std::mutex> lock(m_inventoryMx);
	if (m_inventory.gold > 0)
		m_inventory.gold -= minusgold;
	g_network.SendUpdateGold(GetClients());
	//else
		// 돈 다 써서 강화 더 못해
	// update packet
}

void Room::ADDJobWeapon(int weapon)			
{
	std::lock_guard<std::mutex> lock(m_inventoryMx);
	++m_inventory.JobWeapons[static_cast<JobWeapon>(weapon - 1)];
	// update packet
}

void Room::DecideJobWeapon(int weapon)
{
	std::lock_guard<std::mutex> lock(m_inventoryMx);
	--m_inventory.JobWeapons[static_cast<JobWeapon>(weapon - 1)];
	// update packet
}

void Room::AddJobDocument(int job)			
{
	std::lock_guard<std::mutex> lock(m_inventoryMx);
	++m_inventory.JobDocuments[static_cast<JobDocument>(job - 4)];
	// update packet
}

void Room::DecideJobDocument(int job)		// 클라 코드에서 한번 정했으면 자기 직업 정해지니까 바꾸기 못함을 적어야 할듯. (다른 직업이 클릭이 안되게 막음)
{													// 그리고 직업1 전직서가 없으면 클릭조차 안되게 클라자체에서 막자. (서버에서 해야할 일을 애초에 줄이자)
	std::lock_guard<std::mutex> lock(m_inventoryMx);
	--m_inventory.JobDocuments[static_cast<JobDocument>(job - 4)];
	// update packet
}

// 몬스터
void Room::InitChallengeMonsters()
{
	int monsterID = 0;

	// Mushroom
	{
		m_monsters[monsterID].SetMonster( monsterID, NormalMonsterType::Mushroom,-197.f, 2.f, -134.f);
		m_monsters[++monsterID].SetMonster( monsterID, NormalMonsterType::Mushroom,-193.f, 2.f, -175.f);
		m_monsters[++monsterID].SetMonster( monsterID, NormalMonsterType::Mushroom,-144.f, 2.f, -175.f);
		m_monsters[++monsterID].SetMonster( monsterID, NormalMonsterType::Mushroom, 51.f, 2.f, -170.f);
		m_monsters[++monsterID].SetMonster( monsterID, NormalMonsterType::Mushroom,-123.f, 2.f, -107.f);
		m_monsters[++monsterID].SetMonster( monsterID, NormalMonsterType::Mushroom,-89.f, 2.f, -199.f);
		m_monsters[++monsterID].SetMonster( monsterID, NormalMonsterType::Mushroom,-60.f, 2.f, -166.f);
		m_monsters[++monsterID].SetMonster( monsterID, NormalMonsterType::Mushroom,-75.f, 2.f, -115.f);
		m_monsters[++monsterID].SetMonster( monsterID, NormalMonsterType::Mushroom,-4.f, 2.f, -161.f);
		m_monsters[++monsterID].SetMonster( monsterID, NormalMonsterType::Mushroom,-71.f, 2.f, -76.f);
	}
	// Fight Fly
	{
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly,-210.f, 5.f, -0.f );
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly,-165.f, 5.f, -3.f );
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly,-128.f, 5.f, -38.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly,-78.f, 5.f, -37.f );
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly,-190.f, 5.f, 28.f );
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly,-92.f, 5.f, 47.f );
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly,-177.f, 5.f, -34.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly,-212.f, 5.f, -42.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly,-166.f, 5.f, -74.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly,-198.f, 5.f, -82.f);
	}
	// Plant Dionaea
	{
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea,11.f, 2.f, 92.f );
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea,44.f, 2.f, 105.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea,91.f, 2.f, 108.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea,68.f, 2.f, 186.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea,133.f, 2.f, 169.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea,147.f, 2.f, 110.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea,186.f, 2.f, 150.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea,185.f, 2.f, -9.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea,94.f, 2.f, 24.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea,136.f, 2.f, 61.f);
	}
	// Pea Shooter
	/* {
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PeaShooter,-60.f, 5.f, 60.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PeaShooter,-60.f, 5.f, 60.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PeaShooter,-60.f, 5.f, 60.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PeaShooter,-60.f, 5.f, 60.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PeaShooter,-60.f, 5.f, 60.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PeaShooter,-60.f, 5.f, 60.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PeaShooter,-60.f, 5.f, 60.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PeaShooter,-60.f, 5.f, 60.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PeaShooter,-60.f, 5.f, 60.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PeaShooter,-60.f, 5.f, 60.f };
	}*/
	// Plant Venus
	{
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus,3.f, 2.f, -97.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus,53.f, 2.f, -119.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus,95.f, 2.f, -138.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus,91.f, 2.f, -73.f );
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus,105.f, 2.f, -27.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus,27.f, 2.f, -49.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus,-39.f, 2.f, 5.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus,0.f, 2.f, 40.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus,-56.f, 2.f, 71.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, -44.f, 2.f, 135.f);
	}
	// Flower Fairy
	{
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy,143.f, 5.f, -158.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy,153.f, 5.f, -209.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy,187.f, 5.f, -212.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy,218.f, 5.f, -197.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy,220.f, 5.f, -153.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy,154.f, 5.f, -85.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy,179.f, 5.f, -57.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy,159.f, 5.f, -135.f); 
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy,186.f, 5.f, -182.f);
		m_monsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy,189.f, 5.f, -149.f);
	}
	m_MonsterNum = monsterID;
}