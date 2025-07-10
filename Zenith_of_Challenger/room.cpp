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
		std::cout << "[ERROR] 클라이언트 [" << RoomMasterID << "] 는 방장이 아니여서 시작할 수 없습니다.\n";
		// TODO : 클라한테 너 방장 아니여서 시작 못한다는 거 말해줘야 할듯.(나중에)
		return;
	}

	InitChallengeMonsters();									// 도전 스테이지 몬스터 초기화

	g_network.SendWhoIsMyTeam(GetClients());					// 아이디 값 보내주기
	g_network.SendInitialState(GetClients());					// 게임방 안에 본인 포함 모두한테 초기 좌표 패킷 보내기
	g_network.SendInitMonster(GetClients(),m_Cmonsters);		// 게임방 안의 모든 몬스터 초기화
	g_network.SendGameStart(GetClients());						// 게임방 안에 본인 포함 모두한테 게임시작 패킷 보내기
	
	AllPlayerNum(m_clients.size());								// 게임에 입장한 플레이어가 몇명이야?

	m_IsGaming = true;
	std::cout << "[INFO] 방[" << m_room_id << "] 게임을 시작하였습니다!\n";
}

void Room::PushStartZenithButton(int RoomMasterID)
{
	if (RoomMasterID != GetRoomMasterID() || m_RoomState != Stage::REPAIR_TIME) {
		std::cout << "[ERROR] 클라이언트 [" << RoomMasterID << "] 는 방장이 아니여서 시작할 수 없습니다.\n";
		// TODO : 클라한테 너 방장 아니여서 시작 못한다는 거 말해줘야 할듯.(나중에)
		return;
	}

	for (int i = 0; i < GetClientsNum(); ++i) {		// 정점 스테이지 스폰 위치가 어딘데
		g_client[m_clients[i]].SetZenithCoord(i);
	}

	//InitZenithMonsters();									// 정점 스테이지 몬스터 초기화 (보스 몬스터 포함)

	//g_network.SendZenithMonster(GetClients(),m_Zmonsters);// 정점 스테이지 모든 몬스터 초기화
	g_network.SendZenithState(GetClients());				// 정점 스테이지 클라이언트 초기 좌표 패킷 보내기
	g_network.SendStartZenithStage(GetClients());			// 게임방 안에 본인 포함 모두한테 게임시작 패킷 보내기

	std::cout << "[INFO] 방[" << m_room_id << "] 정점 스테이지에 진입하였습니다!\n";
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
}

void Room::ZenithTimerThread()
{
	// 정점 스테이지 5분
	for(int i = 0; i < ZENITH_TIME; ++i){
		std::this_thread::sleep_for(std::chrono::seconds(ZENITH_TIME));
		if (m_stopTimer) return;
		UpdateMonsterTargetList();
	}
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

void Room::UpdateMonsterTargetList()
{
	static auto lastUpdate = std::chrono::steady_clock::now();
	auto now = std::chrono::steady_clock::now();
	float elapsed = std::chrono::duration<float>(now - lastUpdate).count();
	if (elapsed < AGGRO_N_TARGET_UPDATE_TIME) return;  // 3초마다 실행
	lastUpdate = now;
	
	if (m_RoomState == Stage::CHALLENGE_STAGE) {		// 도전스테이지 몬스터
		for (int i = 0; i < m_CMonsterNum; ++i)
		{
			if (m_Cmonsters[i].GetLived())
			{
				int targetID = m_Cmonsters[i].UpdateTargetList();

				// 클라이언트에게 바라보는 대상 패킷 전송
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
	else if (m_RoomState == Stage::ZENITH_STAGE) {		// 정점스테이지 몬스터
		for (int i = 0; i < m_ZMonsterNum; ++i)
		{
			if (m_Zmonsters[i].GetLived())
			{
				int targetID = m_Zmonsters[i].UpdateTargetList();

				// 클라이언트에게 바라보는 대상 패킷 전송
				SC_Packet_ZMonsterTarget pkt;
				pkt.type = SC_PACKET_ZMONSTERTARGET;
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

}

void Room::UpdateMonsterAggroList()
{
	auto now = std::chrono::steady_clock::now();
	float elapsed = std::chrono::duration<float>(now - m_UpdatelastAggro).count();
	if (elapsed < AGGRO_N_TARGET_UPDATE_TIME) return;  // 0.3초마다 갱신

	// 플레이어 좌표 초기화 및 갱신 --> 플레이어가 3명이 고정이 아니여서 그냥 밀어버리고 새로 받음.
	m_PlayerCoord.clear();
	for (int id : m_clients)
	{
		float x = g_client[id].GetX();
		float z = g_client[id].GetZ();
		m_PlayerCoord.push_back({ id, x, z });
	}

	// 몬스터에 플레이어 좌표 업데이트 리스트 전달
	for (int i = 0; i < m_Zmonsters.size(); ++i) {
		if (m_Zmonsters[i].GetLived() == false)
			continue;
		m_Zmonsters[i].UpdateAggroList(m_PlayerCoord);
	}

	for (int i = 0; i < m_ZMonsterNum; ++i)
	{
		if (m_Cmonsters[i].GetLived())  // 살아있을때
		{
			m_Zmonsters[i].AIMove();
		}
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
		m_Cmonsters[monsterID].SetMonster( monsterID, NormalMonsterType::Mushroom,-197.f, 2.f, -134.f);
		m_Cmonsters[++monsterID].SetMonster( monsterID, NormalMonsterType::Mushroom,-193.f, 2.f, -175.f);
		m_Cmonsters[++monsterID].SetMonster( monsterID, NormalMonsterType::Mushroom,-144.f, 2.f, -175.f);
		m_Cmonsters[++monsterID].SetMonster( monsterID, NormalMonsterType::Mushroom, 51.f, 2.f, -170.f);
		m_Cmonsters[++monsterID].SetMonster( monsterID, NormalMonsterType::Mushroom,-123.f, 2.f, -107.f);
		m_Cmonsters[++monsterID].SetMonster( monsterID, NormalMonsterType::Mushroom,-89.f, 2.f, -199.f);
		m_Cmonsters[++monsterID].SetMonster( monsterID, NormalMonsterType::Mushroom,-60.f, 2.f, -166.f);
		m_Cmonsters[++monsterID].SetMonster( monsterID, NormalMonsterType::Mushroom,-75.f, 2.f, -115.f);
		m_Cmonsters[++monsterID].SetMonster( monsterID, NormalMonsterType::Mushroom,-4.f, 2.f, -161.f);
		m_Cmonsters[++monsterID].SetMonster( monsterID, NormalMonsterType::Mushroom,-71.f, 2.f, -76.f);
	}
	// Fight Fly
	{
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly,-210.f, 5.f, -0.f );
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly,-165.f, 5.f, -3.f );
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly,-128.f, 5.f, -38.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly,-78.f, 5.f, -37.f );
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly,-190.f, 5.f, 28.f );
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly,-92.f, 5.f, 47.f );
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly,-177.f, 5.f, -34.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly,-212.f, 5.f, -42.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly,-166.f, 5.f, -74.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly,-198.f, 5.f, -82.f);
	}
	// Plant Dionaea
	{
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea,11.f, 2.f, 92.f );
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea,44.f, 2.f, 105.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea,91.f, 2.f, 108.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea,68.f, 2.f, 186.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea,133.f, 2.f, 169.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea,147.f, 2.f, 110.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea,186.f, 2.f, 150.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea,185.f, 2.f, -9.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea,94.f, 2.f, 24.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea,136.f, 2.f, 61.f);
	}
	// Plant Venus
	{
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus,3.f, 2.f, -97.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus,53.f, 2.f, -119.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus,95.f, 2.f, -138.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus,91.f, 2.f, -73.f );
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus,105.f, 2.f, -27.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus,27.f, 2.f, -49.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus,-39.f, 2.f, 5.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus,0.f, 2.f, 40.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus,-56.f, 2.f, 71.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, -44.f, 2.f, 135.f);
	}
	// Flower Fairy
	{
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy,143.f, 5.f, -158.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy,153.f, 5.f, -209.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy,187.f, 5.f, -212.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy,218.f, 5.f, -197.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy,220.f, 5.f, -153.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy,154.f, 5.f, -85.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy,179.f, 5.f, -57.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy,159.f, 5.f, -135.f); 
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy,186.f, 5.f, -182.f);
		m_Cmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy,189.f, 5.f, -149.f);
	}
	
	// 일반 몬스터 마리수
	m_CMonsterNum = monsterID;
}

void Room::InitZenithMonsters()
{
	int monsterID = 0;

	// Mushroom
	{
		m_Zmonsters[monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, -197.f, 2.f, -134.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, -193.f, 2.f, -175.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, -144.f, 2.f, -175.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, 51.f, 2.f, -170.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::Mushroom, -123.f, 2.f, -107.f);
	}
	// Fight Fly
	{
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -210.f, 5.f, -0.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -165.f, 5.f, -3.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -128.f, 5.f, -38.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -78.f, 5.f, -37.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FightFly, -190.f, 5.f, 28.f);
	}
	// Plant Dionaea
	{
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 11.f, 2.f, 92.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 44.f, 2.f, 105.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 91.f, 2.f, 108.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 68.f, 2.f, 186.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantDionaea, 133.f, 2.f, 169.f);
	}
	// Plant Venus
	{
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 3.f, 2.f, -97.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 53.f, 2.f, -119.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 95.f, 2.f, -138.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 91.f, 2.f, -73.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::PlantVenus, 105.f, 2.f, -27.f);
	} 
	// Flower Fairy
	{
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 143.f, 5.f, -158.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 153.f, 5.f, -209.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 187.f, 5.f, -212.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 218.f, 5.f, -197.f);
		m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::FlowerFairy, 220.f, 5.f, -153.f);
	}

	// 일반 몬스터 마리수
	m_ZMonsterNum = monsterID;

	// Boss Monster
	m_Zmonsters[++monsterID].SetMonster(monsterID, NormalMonsterType::BossMonster, 0.f, 65.f, 0.f);
}