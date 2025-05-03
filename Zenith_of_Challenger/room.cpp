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
		std::cout << "[ERROR] Ŭ���̾�Ʈ [" << RoomMasterID << "] �� ������ �ƴϿ��� ������ �� �����ϴ�.\n";
		// TODO : Ŭ������ �� ���� �ƴϿ��� ���� ���Ѵٴ� �� ������� �ҵ�.(���߿�)
		return;
	}

	InitChallengeMonsters();									// ���� �������� ���� �ʱ�ȭ

	g_network.SendWhoIsMyTeam(GetClients());					// ���̵� �� �����ֱ�
	g_network.SendInitialState(GetClients());					// ���ӹ� �ȿ� ���� ���� ������� �ʱ� ��ǥ ��Ŷ ������
	g_network.SendInitMonster(GetClients(),GetMonsters());		// ���ӹ� ���� ��� ���� �ʱ�ȭ
	Sleep(2000);

	g_network.SendGameStart(GetClients());						// ���ӹ� �ȿ� ���� ���� ������� ���ӽ��� ��Ŷ ������
	AllPlayerNum(m_clients.size());								// ���ӿ� ������ �÷��̾ ����̾�?

	m_IsGaming = true;
	std::cout << "[INFO] ��[" << m_room_id << "] ������ �����Ͽ����ϴ�!\n";
}

void Room::PushStartZenithButton(int RoomMasterID)
{
	if (RoomMasterID != GetRoomMasterID()) {
		std::cout << "[ERROR] Ŭ���̾�Ʈ [" << RoomMasterID << "] �� ������ �ƴϿ��� ������ �� �����ϴ�.\n";
		// TODO : Ŭ������ �� ���� �ƴϿ��� ���� ���Ѵٴ� �� ������� �ҵ�.(���߿�)
		return;
	}
	g_network.SendStartZenithStage(GetClients());			// ���ӹ� �ȿ� ���� ���� ������� ���ӽ��� ��Ŷ ������
}

void Room::StartGame()
{
	m_RoomState = Stage::CHALLENGE_STAGE;
	m_timer = std::thread(&Room::ChallengeTimerThread, this);
}

void Room::ChallengeTimerThread()
{
	// ���� �������� 8��
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
	// ���� �������� 5��
	std::this_thread::sleep_for(std::chrono::seconds(ZENITH_TIME));
	if (m_stopTimer) return;
	EndGame();
}

void Room::RepairTime()
{
	m_RoomState = Stage::REPAIR_TIME;
	g_network.SendStartRepairTime(GetClients());	// �ٵ� ������ ������ �̵�����
}

void Room::EndGame()
{
	if (m_timer.joinable()) {
		m_timer.join();
	}
	m_IsGaming = false;
	m_RoomState = Stage::LOBBY;

	std::cout << "[INFO][" << m_room_id << "]�� ������ �����Ͽ����ϴ�!\n";
	// TODO
}


// �κ��丮
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
		// �� �� �Ἥ ��ȭ �� ����
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

void Room::DecideJobDocument(JobDocument job)		// Ŭ�� �ڵ忡�� �ѹ� �������� �ڱ� ���� �������ϱ� �ٲٱ� ������ ����� �ҵ�. (�ٸ� ������ Ŭ���� �ȵǰ� ����)
{													// �׸��� ����1 �������� ������ Ŭ������ �ȵǰ� Ŭ����ü���� ����. (�������� �ؾ��� ���� ���ʿ� ������)
	std::lock_guard<std::mutex> lock(m_inventoryMx);
	--m_inventory.JobDocuments[job];
	// update packet
}

// ����
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