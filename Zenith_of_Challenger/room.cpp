#include "room.h"

void Room::Init(int room_id)
{
	m_room_id = room_id;
	m_clients.clear();
}

void Room::ResetRoom()
{
	m_IsGaming = false;
	m_RoomState = Stage::LOBBY;         // �ʱ� ���·�
	m_clients.clear();                  // Ŭ���̾�Ʈ ��� ����
	m_inventory = InventoryItem();

	SetStopTimer(false);
	SetSkipTimer(false);
	SetSkipButton(false);
	m_playerNum = 0;
	m_enterClientNum = 0;
	m_enterZenithNum = 0;

	for (auto& monster : m_Cmonsters)
		monster.Reset();
	m_CMonsterNum = 0;
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
		std::cout << "[ERROR] Ŭ���̾�Ʈ [" << RoomMasterID << "] �� ������ �ƴϿ��� ������ �� �����ϴ�.\n";
		// TODO : Ŭ������ �� ���� �ƴϿ��� ���� ���Ѵٴ� �� ������� �ҵ�.(���߿�)
		return;
	}

	InitChallengeMonsters();									// ���� �������� ���� �ʱ�ȭ
	InitZenithMonsters();										// ���� �������� ���� �ʱ�ȭ (���� ���� ����)
	InitZMonsterFirstLastCoord();								// ���� �������� ���� ���� ��Ʈ ��ǥ ����

	g_network.SendWhoIsMyTeam(GetClients());					// ���̵� �� �����ֱ�
	g_network.SendInitialState(GetClients());					// ���ӹ� �ȿ� ���� ���� ������� �ʱ� ��ǥ ��Ŷ ������
	g_network.SendInitMonster(GetClients(), m_Cmonsters);		// ���ӹ� ���� ���� ���� �ʱ�ȭ
	g_network.SendZenithMonster(GetClients(), m_Zmonsters);		// ���ӹ� ���� ���� ���� �ʱ�ȭ
	g_network.SendGameStart(GetClients());						// ���ӹ� �ȿ� ���� ���� ������� ���ӽ��� ��Ŷ ������

	AllPlayerNum(m_clients.size());								// ���ӿ� ������ �÷��̾ ����̾�?

	m_IsGaming = true;
	std::cout << "[INFO] ��[" << m_room_id << "] ������ �����Ͽ����ϴ�!\n";
}

void Room::PushStartZenithButton(int RoomMasterID)
{
	if (RoomMasterID != GetRoomMasterID() || m_RoomState != Stage::REPAIR_TIME) {
		std::cout << "[ERROR] Ŭ���̾�Ʈ [" << RoomMasterID << "] �� ������ �ƴϿ��� ������ �� �����ϴ�.\n";
		// TODO : Ŭ������ �� ���� �ƴϿ��� ���� ���Ѵٴ� �� ������� �ҵ�.(���߿�)
		return;
	}

	for (int i = 0; i < GetClientsNum(); ++i) {		// ���� �������� ���� ��ġ�� ���
		g_client[m_clients[i]].SetZenithCoord(i);
	}

	g_network.SendZenithState(GetClients());				// ���� �������� Ŭ���̾�Ʈ �ʱ� ��ǥ ��Ŷ ������
	g_network.SendStartZenithStage(GetClients());			// ���ӹ� �ȿ� ���� ���� ������� ���ӽ��� ��Ŷ ������

	std::cout << "[INFO] ��[" << m_room_id << "] ���� ���������� �����Ͽ����ϴ�!\n";
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
		UpdateMonsterTargetList();
	}
	RepairTime();
}

void Room::StartZenithStage()
{
	if (m_timer.joinable())
		m_timer.join();

	m_RoomState = Stage::ZENITH_STAGE;

	m_timer = std::thread(&Room::ZenithTimerThread, this);
	m_ZmonsterPosTimer = std::thread(&Room::m_ZmonsterPosTimerThread, this);
}

void Room::ZenithTimerThread()
{
	// ���� �������� 5��
	for (int i = 0; i < ZENITH_TIME; ++i) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		if (m_stopTimer) return;
		UpdateMonsterTargetList();
		UpdateMonsterAggroList();
	}
	EndGame();
}

void Room::m_ZmonsterPosTimerThread()
{
	using namespace std::chrono;

	auto prev = steady_clock::now();
	while (!m_stopMonsterPosThread)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(30));

		for (int i = 0; i < m_ZMonsterNum; ++i)
		{
			if (m_Cmonsters[i].GetLived())  // ���������
			{
				m_Zmonsters[i].Move();
				// ���� ��ǥ Ŭ���̾�Ʈ�� ����
				BroadcastMonsterPosition(i);
			}
		}
	}
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

void Room::UpdateMonsterTargetList()
{
	static auto lastUpdate = std::chrono::steady_clock::now();
	auto now = std::chrono::steady_clock::now();
	float elapsed = std::chrono::duration<float>(now - lastUpdate).count();
	if (elapsed < AGGRO_N_TARGET_UPDATE_TIME) return;  // 3�ʸ��� ����
	lastUpdate = now;

	if (m_RoomState == Stage::CHALLENGE_STAGE) {		// ������������ ����
		for (int i = 0; i < m_CMonsterNum; ++i)
		{
			if (m_Cmonsters[i].GetLived())
			{
				int targetID = m_Cmonsters[i].UpdateTargetList();

				// Ŭ���̾�Ʈ���� �ٶ󺸴� ��� ��Ŷ ����
				SC_Packet_CMonsterTarget pkt;
				pkt.type = SC_PACKET_CMONSTERTARGET;
				pkt.size = sizeof(pkt);
				pkt.monsterID = i;
				pkt.targetID = targetID;

				for (int id : m_clients)
				{
					if (g_network.clients[id].m_used)
						g_network.clients[id].do_send(pkt);
				}
			}
		}
	}
	//else if (m_RoomState == Stage::ZENITH_STAGE) {		// ������������ ����
	//	for (int i = 0; i < m_ZMonsterNum; ++i)
	//	{
	//		if (m_Zmonsters[i].GetLived())
	//		{
	//			int targetID = m_Zmonsters[i].UpdateTargetList();

	//			// Ŭ���̾�Ʈ���� �ٶ󺸴� ��� ��Ŷ ����
	//			SC_Packet_ZMonsterTarget pkt;
	//			pkt.type = SC_PACKET_ZMONSTERTARGET;
	//			pkt.size = sizeof(pkt);
	//			pkt.monsterID = i;
	//			pkt.targetID = targetID;

	//			for (int id : m_clients)
	//			{
	//				if (g_network.clients[id].m_used)
	//					g_network.clients[id].do_send(pkt);
	//			}
	//		}
	//	}
	//}

}

void Room::UpdateMonsterAggroList()
{
	auto now = std::chrono::steady_clock::now();
	float elapsed = std::chrono::duration<float>(now - m_UpdatelastAggro).count();
	if (elapsed < AGGRO_N_TARGET_UPDATE_TIME) return;  // 0.3�ʸ��� ����

	// �÷��̾� ��ǥ �ʱ�ȭ �� ���� --> �÷��̾ 3���� ������ �ƴϿ��� �׳� �о������ ���� ����.
	m_PlayerCoord.clear();
	for (int id : m_clients)
	{
		float x = g_client[id].GetX();
		float z = g_client[id].GetZ();
		m_PlayerCoord.push_back({ id, x, z });
	}

	// ���Ϳ� �÷��̾� ��ǥ ������Ʈ ����Ʈ ����
	for (int i = 0; i < m_Zmonsters.size(); ++i) {
		if (m_Zmonsters[i].GetLived() == false)
			continue;
		m_Zmonsters[i].UpdateAggroList(m_PlayerCoord);
	}
}

void Room::SetStopTimer(bool check)
{
	m_stopTimer = check;
}

void Room::SetSkipTimer(bool check)
{
	m_skipTimer = check;
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
		// �� �� �Ἥ ��ȭ �� ����
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

void Room::DecideJobDocument(int job)		// Ŭ�� �ڵ忡�� �ѹ� �������� �ڱ� ���� �������ϱ� �ٲٱ� ������ ����� �ҵ�. (�ٸ� ������ Ŭ���� �ȵǰ� ����)
{													// �׸��� ����1 �������� ������ Ŭ������ �ȵǰ� Ŭ����ü���� ����. (�������� �ؾ��� ���� ���ʿ� ������)
	std::lock_guard<std::mutex> lock(m_inventoryMx);
	--m_inventory.JobDocuments[static_cast<JobDocument>(job - 4)];
	// update packet
}

// ����
void Room::InitChallengeMonsters()
{
	int monsterID = 0;

	// Mushroom
	{
		m_Cmonsters[monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, -197.f, 2.f, -134.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, -193.f, 2.f, -175.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, -144.f, 2.f, -175.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, 51.f, 2.f, -170.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, -123.f, 2.f, -107.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, -89.f, 2.f, -199.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, -60.f, 2.f, -166.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, -75.f, 2.f, -115.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, -4.f, 2.f, -161.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, -71.f, 2.f, -76.f);
	}
	// Fight Fly
	{
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -210.f, 5.f, -0.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -165.f, 5.f, -3.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -128.f, 5.f, -38.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -78.f, 5.f, -37.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -190.f, 5.f, 28.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -92.f, 5.f, 47.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -177.f, 5.f, -34.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -212.f, 5.f, -42.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -166.f, 5.f, -74.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -198.f, 5.f, -82.f);
	}
	// Plant Dionaea
	{
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 11.f, 2.f, 92.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 44.f, 2.f, 105.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 91.f, 2.f, 108.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 68.f, 2.f, 186.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 133.f, 2.f, 169.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 147.f, 2.f, 110.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 186.f, 2.f, 150.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 185.f, 2.f, -9.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 94.f, 2.f, 24.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 136.f, 2.f, 61.f);
	}
	// Plant Venus
	{
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 3.f, 2.f, -97.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 53.f, 2.f, -119.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 95.f, 2.f, -138.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 91.f, 2.f, -73.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 105.f, 2.f, -27.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 27.f, 2.f, -49.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, -39.f, 2.f, 5.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 0.f, 2.f, 40.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, -56.f, 2.f, 71.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, -44.f, 2.f, 135.f);
	}
	// Flower Fairy
	{
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 143.f, 5.f, -158.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 153.f, 5.f, -209.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 187.f, 5.f, -212.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 218.f, 5.f, -197.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 220.f, 5.f, -153.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 154.f, 5.f, -85.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 179.f, 5.f, -57.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 159.f, 5.f, -135.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 186.f, 5.f, -182.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 189.f, 5.f, -149.f);
	}

	// �Ϲ� ���� ������
	m_CMonsterNum = monsterID;
}

void Room::InitZenithMonsters()
{
	int monsterID = 0;

	// Mushroom
	{
		m_Zmonsters[monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, 344.f, 44.f, -52.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, 260.f, 44.f, -152.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, 330.f, 44.f, 0.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, 290.f, 44.f, 100.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, 290.f, 44.f, 0.f);
	}
	// Fight Fly
	{
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, 0.0f, 44.f, -200.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, 160.f, 44.f, -94.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -150.f, 44.f, -170.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -183.f, 44.f, -80.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, 100.f, 44.f, 60.f);
	}
	// Plant Dionaea
	{
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, -2.f, 44.f, -165.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, -150.f, 44.f, 18.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, -208.f, 44.f, 203.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 167.f, 44.f, 57.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 39.f, 44.f, -137.f);
	}
	// Plant Venus
	{
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 202.f, 44.f, -143.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 93.0f, 44.f, -290.0f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, -158.0f, 44.f, -263.0f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, -347.0f, 44.f, -87.0f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, -200.0f, 44.f, 100.0f);
	}
	// Flower Fairy 
	{
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 469.f, 44.f,	-2.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 165.f, 44.f, 167.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 117.0f, 44.f, 97.0f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, -49.0f, 44.f, -95.0f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, -86.0f, 44.f, 126.0f);
	}

	// �Ϲ� ���� ������
	m_ZMonsterNum = monsterID + 1;

	// Boss Monster
	m_Zmonsters[m_ZMonsterNum].SetMonster(m_ZMonsterNum, NormalMonsterType::BossMonster, 0.f, 45.f, 0.f);
}

void Room::InitZMonsterFirstLastCoord()
{
	int monsterID = 0;

	// Mushroom
	{
		m_Zmonsters[monsterID].SetFristLastCoord(344.f, -52.f, 300.f, -52.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(260.f, -152.f, 300.f, -112.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(330.0f, 0.f, 380.0f, 40.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(290.0f, 100.f, 250.0f, 60.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(290.0f, 0.f, 290.0f, 40.f);
	}
	// Fight Fly
	{
		m_Zmonsters[++monsterID].SetFristLastCoord(0.f, -200.f, 40.0f, -261.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(160.f, -94.f, 180.f, -18.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(-150.0f, -170.f, -107.0f, -215.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(-183.f, -80.1f, -187.f, -18.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(100.f, 60.f, 172.0f, 14.f);
	}
	// Plant Dionaea
	{
		m_Zmonsters[++monsterID].SetFristLastCoord(-2.0f, -165.f, -143.0f, -103.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(-150.f, 18.f, -75.0f, 93.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(-208.0f, 203.f, 43.0f, 237.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(167.0f, 57.f, 114.0f, 181.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(39.0f, -137.f, 125.0f, -17.f);
	}
	// Plant Venus
	{
		m_Zmonsters[++monsterID].SetFristLastCoord(202.0f, -143.0f, 161.0f, -220.0f);
		m_Zmonsters[++monsterID].SetFristLastCoord(93.0f, -290.0f, -46.0f, -315.0f);
		m_Zmonsters[++monsterID].SetFristLastCoord(-158.0f, -263.0f, -280.0f, -133.0f);
		m_Zmonsters[++monsterID].SetFristLastCoord(-347.0f, -87.0f, -310.0f, 24.0f);
		m_Zmonsters[++monsterID].SetFristLastCoord(-200.0f, 100.0f, -332.0f, 74.0f);
	}
	// Flower Fairy
	{
		m_Zmonsters[++monsterID].SetFristLastCoord(	469.f, -2.f, 396.f, .0f);
		m_Zmonsters[++monsterID].SetFristLastCoord(	165.f, 167.f, 264.f, 173.f);
		m_Zmonsters[++monsterID].SetFristLastCoord( 117.0f, 97.0f, -4.0f, -152.0f);
		m_Zmonsters[++monsterID].SetFristLastCoord( -49.0f, -95.0f, -132.0f, -42.0f);
		m_Zmonsters[++monsterID].SetFristLastCoord(-86.0f, 126.0f, -227.f, 153.f);
	}
}

void Room::BroadcastMonsterPosition(int idx)
{
	{
		SC_Packet_ZMonsterMove pkt;
		pkt.type = SC_PACKET_ZMONSTERMOVE;
		pkt.size = sizeof(pkt);
		pkt.monsterID = idx;
		pkt.x = m_Zmonsters[idx].GetX();
		pkt.y = m_Zmonsters[idx].GetY();
		pkt.z = m_Zmonsters[idx].GetZ();

		for (int id : m_clients)
			if (g_network.clients[id].m_used)
				g_network.clients[id].do_send(pkt);

	}
}