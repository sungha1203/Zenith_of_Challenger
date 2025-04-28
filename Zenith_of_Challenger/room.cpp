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
	g_client[client_id].SetRoomIdx(idx);
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
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	g_network.SendInitialState(GetClients());					// ���ӹ� �ȿ� ���� ���� ������� �ʱ� ��ǥ ��Ŷ ������
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	g_network.SendInitMonster(GetClients(),GetMonsters());		// ���ӹ� ���� ��� ���� �ʱ�ȭ
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
	// ���� �������� 5��
	std::this_thread::sleep_for(std::chrono::seconds(ZENITH_TIME));
	if (m_stopTimer) return;
	EndGame();
}

void Room::RepairTime()
{
	m_RoomState = Stage::REPAIR_TIME;
	for (int i = 0; i < GetClientsNum(); ++i) {		// ������ ���� ���
		g_client[m_clients[i]].SetRepairCoord(i);
	}
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

void Room::SetSkipTimer()
{
	m_skipTimer = true;
}

void Room::SetSkipButton(bool check)
{
	m_skipButton = check;
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
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::Mushroom,-197.f, 0.f, -134.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::Mushroom,-193.f, 0.f, -175.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::Mushroom,-144.f, 0.f, -175.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::Mushroom, 51.f, 0.f, -170.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::Mushroom,-123.f, 0.f, -107.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::Mushroom,-89.f, 0.f, -199.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::Mushroom,-60.f, 0.f, -166.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::Mushroom,-75.f, 0.f, -115.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::Mushroom,-4.f, 0.f, -161.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::Mushroom,-71.f, 0.f, -76.f };
	}
	// Fight Fly
	{
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FightFly,-210.f, 0.f, -0.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FightFly,-165.f, 0.f, -3.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FightFly,-128.f, 0.f, -38.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FightFly,-78.f, 0.f, -37.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FightFly,-190.f, 0.f, 28.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FightFly,-92.f, 0.f, 47.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FightFly,-177.f, 0.f, -34.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FightFly,-212.f, 0.f, -42.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FightFly,-166.f, 0.f, -74.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FightFly,-198.f, 0.f, -82.f };
	}
	// Plant Dionaea
	{
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantDionaea,11.f, 0.f, 92.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantDionaea,44.f, 0.f, 105.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantDionaea,91.f, 0.f, 108.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantDionaea,68.f, 0.f, 186.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantDionaea,133.f, 0.f, 169.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantDionaea,147.f, 0.f, 110.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantDionaea,186.f, 0.f, 150.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantDionaea,185.f, 0.f, -9.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantDionaea,94.f, 0.f, 24.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantDionaea,136.f, 0.f, 61.f };
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
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantVenus,3.f, 0.f, -97.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantVenus,53.f, 0.f, -119.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantVenus,95.f, 0.f, -138.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantVenus,91.f, 0.f, -73.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantVenus,105.f, 0.f, -27.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantVenus,27.f, 0.f, -49.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantVenus,-39.f, 0.f, 5.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantVenus,0.f, 0.f, 40.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantVenus,-56.f, 0.f, 71.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::PlantVenus, -44.f, 0.f, 135.f };
	}
	// Flower Fairy
	{
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FlowerFairy,143.f, -5.f, -158.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FlowerFairy,153.f, -5.f, -209.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FlowerFairy,187.f, -5.f, -212.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FlowerFairy,218.f, -5.f, -197.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FlowerFairy,220.f, -5.f, -153.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FlowerFairy,154.f, -5.f, -85.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FlowerFairy,179.f, -5.f, -57.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FlowerFairy,159.f, -5.f, -135.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FlowerFairy,186.f, -5.f, -182.f };
		m_monsters[++monsterID] = Monster{ monsterID, NormalMonsterType::FlowerFairy,189.f, -5.f, -149.f };
	}
	m_MonsterNum = monsterID;
}