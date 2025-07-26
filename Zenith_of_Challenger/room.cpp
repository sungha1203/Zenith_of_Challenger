#include "room.h"

void Room::Init(int room_id)
{
	m_room_id = room_id;
	m_clients.clear();
}

void Room::ResetRoom()
{
	if (m_timer.joinable()) m_timer.detach();
	if (m_ZmonsterPosTimer.joinable()) m_ZmonsterPosTimer.detach();

	m_IsGaming = false;
	m_RoomState = Stage::LOBBY;
	m_clients.clear();
	m_inventory = InventoryItem();

	SetStopTimer(false);
	SetSkipTimer(false);
	SetSkipButton(false);
	m_playerNum = 0;
	m_enterClientNum = 0;
	m_enterZenithNum = 0;
	m_bossDie = false;
	m_stopMonsterPosThread = false;

	m_UpdatelastAggro = std::chrono::steady_clock::now();

	for (auto& monster : m_Cmonsters) monster.Reset();
	for (auto& monster : m_Zmonsters) monster.Reset();

	m_CMonsterNum = 0;
	m_ZMonsterNum = 0;
	m_clearTime = 0;
	m_PlayerCoord.clear();

	InitZMonsterFirstLastCoord();  // ���� ���� �̵� ��ǥ �ʱ�ȭ
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

	for (int i = 0; i < GetClientsNum(); ++i) {				// ���� �������� ���� ��ġ�� ���
		g_client[m_clients[i]].SetZenithCoord(i);
	}
	for (int i = 0; i < GetClientsNum(); ++i) {				// ���� �������� ���� ��ġ�� ���
		g_client[m_clients[i]].SetAttack();
	}
	
	g_network.SendZenithState(GetClients());				// ���� �������� Ŭ���̾�Ʈ �ʱ� ��ǥ ��Ŷ ������
	g_network.SendStartZenithStage(GetClients());			// ���ӹ� �ȿ� ���� ���� ������� ���ӽ��� ��Ŷ ������
	g_network.SendPlayerAttack(GetClients());				// �÷��̾� ���ݷ� ��Ŷ ������

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
		if (m_bossDie == true) {
			m_clearTime = i;
			break;
		}
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

		for (int i = 0; i <= m_ZMonsterNum; ++i)
		{
			// ��������� && ���� ����
			if (m_Zmonsters[i].GetLived() && i != 25)
			{
				m_Zmonsters[i].Move();

				if (m_Zmonsters[i].AttackAnimation()) {			// ���Ͱ� ������ �����ϸ�
					g_network.SendZMonsterAttack(GetClients(), i, 0);
				}
				if (m_Zmonsters[i].AttackMotion()) {
					g_network.SendBossAttackMotion(GetClients(), m_Zmonsters[i].GetBossSkillType());
				}
			}
			// ��������� && ���� ���� ����
			else if (m_Zmonsters[i].GetLived() && i == 25) {
				m_Zmonsters[i].BossMove();

				if (m_Zmonsters[i].AttackAnimation()) {			// ���� ���Ͱ� ������ �����ϸ�
					g_network.SendZMonsterAttack(GetClients(), i, m_Zmonsters[i].GetBossSkillType());
				}
				if (m_Zmonsters[i].GetDamageThisFrame()) {		// ���� ���Ͱ� ��ų ������ �� ������ ��������
					m_Zmonsters[i].BossSkillDamage(m_clients);
					m_Zmonsters[i].SetDamageThisFrame(false);
				}
			}

			// ���� ��ǥ Ŭ���̾�Ʈ�� ����
			BroadcastMonsterPosition(i);
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
	m_IsGaming = false;
	m_stopMonsterPosThread = true;

	g_network.SendEndGame(GetClients(), m_clearTime);
	//ResetRoom();

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
		if (i != 25)			// ���� �Ϲ� ����
			m_Zmonsters[i].UpdateAggroList(m_PlayerCoord);
		else					// ���� ���� ����
			m_Zmonsters[i].UpdateBossAggroList(m_PlayerCoord);
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

void Room::SetClearBoss()
{
	m_bossDie = true;
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
		m_Cmonsters[monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, -197.f, 0.f, -134.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, -193.f, 0.f, -175.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, -144.f, 0.f, -175.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, 51.f, 0.f, -170.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, -123.f, 0.f, -107.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, -89.f, 0.f, -199.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, -60.f, 0.f, -166.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, -75.f, 0.f, -115.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, -4.f, 0.f, -161.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, -71.f, 0.f, -76.f, 0);
	}
	// Fight Fly
	{
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -210.f, 0.f, -0.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -165.f, 0.f, -3.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -128.f, 0.f, -38.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -78.f, 0.f, -37.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -190.f, 0.f, 28.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -92.f, 0.f, 47.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -177.f, 0.f, -34.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -212.f, 0.f, -42.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -166.f, 0.f, -74.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -198.f, 0.f, -82.f, 0);
	}
	// Plant Dionaea
	{
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 11.f, 0.f, 92.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 44.f, 0.f, 105.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 91.f, 0.f, 108.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 68.f, 0.f, 186.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 133.f, 0.f, 169.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 147.f, 0.f, 110.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 186.f, 0.f, 150.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 185.f, 0.f, -9.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 94.f, 0.f, 24.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 136.f, 0.f, 61.f, 0);
	}
	// Plant Venus
	{
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 3.f, 0.f, -97.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 53.f, 0.f, -119.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 95.f, 0.f, -138.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 91.f, 0.f, -73.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 105.f, 0.f, -27.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 27.f, 0.f, -49.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, -39.f, 0.f, 5.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 0.f, 0.f, 40.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, -56.f, 0.f, 71.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, -44.f, 0.f, 135.f, 0);
	}
	// Flower Fairy
	{
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 143.f, 0.f, -158.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 153.f, 0.f, -209.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 187.f, 0.f, -212.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 218.f, 0.f, -197.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 220.f, 0.f, -153.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 154.f, 0.f, -85.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 179.f, 0.f, -57.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 159.f, 0.f, -135.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 186.f, 0.f, -182.f, 0);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 189.f, 0.f, -149.f, 0);
	}

	// �Ϲ� ���� ������
	m_CMonsterNum = monsterID;
}

void Room::InitZenithMonsters()
{
	int monsterID = 0;

	// Mushroom
	{
		m_Zmonsters[monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, 344.f, 44.f, -52.f, 1);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, 260.f, 44.f, -152.f, 1);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, 330.f, 44.f, 0.f, 1);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, 290.f, 44.f, 100.f, 1);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, 290.f, 44.f, 0.f, 1);
	}
	// Fight Fly
	{
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, 31.0f, 44.f, -234.0f, 1);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, 219.0f, 44.f, -147.0f, 1);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, 160.0f, 44.f, -247.0f, 1);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, 21.0f, 44.f, -340.0f, 1);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -120.0f, 44.f, -338.0f, 1);
	}
	// Plant Dionaea
	{
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, -215.0f, 44.f, -247.0f, 1);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, -331.0f, 44.f, -174.0f, 1);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, -170.0f, 44.f, -261.0f, 1);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, -96.0f, 44.f, -279.0f, 1);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, -346.0f, 44.f, 64.0f, 1);
	}
	// Plant Venus
	{
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 182.0f, 44.f, 106.0f, 1);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 196.0f, 44.f, -131.0f, 1);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 210.0f, 44.f, -38.0f, 1);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 279.0f, 44.f, 166.0f, 1);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, -77.0f, 44.f, 230.0f, 1);
	}
	// Flower Fairy 
	{
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, -109.0f, 41.f, 88.0f, 1);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, -266.0f, 41.f, -111.0f, 1);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, -46.0f, 41.f, -232.0f, 1);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, -182.0f, 41.f, -138.0f, 1);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 395.0f, 41.f, -1.0f, 1);
	}

	// �Ϲ� ���� ������
	m_ZMonsterNum = monsterID + 1;

	// Boss Monster
	m_Zmonsters[m_ZMonsterNum].SetMonster(m_ZMonsterNum, NormalMonsterType::BossMonster, 0.f, 45.f, -23.f, 1);
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
		m_Zmonsters[++monsterID].SetFristLastCoord(31.f, -234.f, 69.f, -289.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(219.f, -147.f, 198.f, -223.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(160.f, -247.f, 38.f, -323.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(21.f, -340.f, -86.f, -335.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(-120.f, -338.f, -194.f, -281.f);
	}
	// Plant Dionaea
	{
		m_Zmonsters[++monsterID].SetFristLastCoord(-215.f, -247.f, -305.f, -203.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(-331.f, -174.f, -359.f, -62.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(-170.f, -261.f, -226.f, -170.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(-96.f, -279.f, -158.f, -272.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(-346.f, 64.f, -274.f, 118.f);
	}
	// Plant Venus
	{
		m_Zmonsters[++monsterID].SetFristLastCoord(182.f, 106.f, 76.f, 139.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(196.f, -131.f, 210.f, -59.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(210.f, -38.f, 203.f, 67.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(279.f, 166.f, 124.f, 191.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(-77.f, 230.f, 56.f, 241.f);
	}
	// Flower Fairy
	{
		m_Zmonsters[++monsterID].SetFristLastCoord(-109.f, 88.f, -236.f, 114.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(-266.f, -111.f, -251.f, 107.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(-46.f, -232.f, -148.f, -183.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(-182.f, -138.f, -200.f, 73.f);
		m_Zmonsters[++monsterID].SetFristLastCoord(395.f, -1.f, 480.f, -1.f);
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


		// �ٶ󺸴� ����
		if (idx != 25) {// ���� �Ϲ� ����
			switch (m_Zmonsters[idx].GetState()) {
			case 0:		// ���� �պ��
				if (m_Zmonsters[idx].GetDirection()) {		// ������
					pkt.targetX = m_Zmonsters[idx].GetFirstLastCoord(1, 0);
					pkt.targetZ = m_Zmonsters[idx].GetFirstLastCoord(1, 1);
				}
				else {										// ������
					pkt.targetX = m_Zmonsters[idx].GetFirstLastCoord(0, 0);
					pkt.targetZ = m_Zmonsters[idx].GetFirstLastCoord(0, 1);
				}
				break;
			case 1:		// �÷��̾� ��׷�
				pkt.targetX = g_client[m_Zmonsters[idx].GetAggroPlayer()].GetX();
				pkt.targetZ = g_client[m_Zmonsters[idx].GetAggroPlayer()].GetZ();
				break;
			case 2:		// ���� �������� ����
				pkt.targetX = m_Zmonsters[idx].GetFirstLastCoord(0, 0);
				pkt.targetZ = m_Zmonsters[idx].GetFirstLastCoord(0, 1);
				break;
			case 3:		// ����
				pkt.targetX = g_client[m_Zmonsters[idx].GetAggroPlayer()].GetX();
				pkt.targetZ = g_client[m_Zmonsters[idx].GetAggroPlayer()].GetZ();
				break;
			}
		}
		else {			// ���� ���� ����
			switch (m_Zmonsters[idx].GetState()) {
			case 0:		// ���� ���� ó�� ��ġ
				pkt.targetX = -172.79f;
				pkt.targetZ = 77.81f;
				break;
			case 1:		// �÷��̾� ��׷�
				pkt.targetX = g_client[m_Zmonsters[idx].GetAggroPlayer()].GetX();
				pkt.targetZ = g_client[m_Zmonsters[idx].GetAggroPlayer()].GetZ();
				break;
			case 2:		// ���� �������� ����
				pkt.targetX = 0.0f;
				pkt.targetZ = 0.0f;
				break;
			case 3:		// ����
				if (m_Zmonsters[idx].GetBossSkillAnimation() == true) {
					pkt.targetX = m_Zmonsters[idx].GetSkillTargetX();
					pkt.targetZ = m_Zmonsters[idx].GetSkillTargetZ();
				}
				else {
					pkt.targetX = g_client[m_Zmonsters[idx].GetAggroPlayer()].GetX();
					pkt.targetZ = g_client[m_Zmonsters[idx].GetAggroPlayer()].GetZ();
				}
				break;
			}
		}

		pkt.targetY = m_Zmonsters[idx].GetY();

		for (int id : m_clients)
			if (g_network.clients[id].m_used)
				g_network.clients[id].do_send(pkt);
	}
}