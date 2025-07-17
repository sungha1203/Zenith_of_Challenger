#include "network.h"

ClientNetwork::ClientNetwork() :m_prevRemain(0) {}

ClientNetwork::~ClientNetwork()
{
}

void ClientNetwork::Connect()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)   return;

	m_clientsocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_clientsocket == INVALID_SOCKET) {
		WSACleanup();
		return;
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT_NUM);
	inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

	if (connect(m_clientsocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		closesocket(m_clientsocket);
		WSACleanup();
		MessageBoxA(nullptr, "서버에 연결할 수 없습니다.", "연결 실패", MB_OK | MB_ICONERROR);
		exit(1);
	}

	m_running = true;
	m_recvThread = std::thread(&ClientNetwork::Receive, this);
}

void ClientNetwork::Disconnect()
{
	m_running = false;

	if (m_recvThread.joinable())
		m_recvThread.join();

	if (m_clientsocket != INVALID_SOCKET) {
		closesocket(m_clientsocket);
		m_clientsocket = INVALID_SOCKET;
	}
}

bool ClientNetwork::SendPacket(const char* data, int length)
{
	int sent = send(m_clientsocket, data, length, 0);

	char debug[128];
	sprintf_s(debug, "[SendPacket] length=%d, sent=%d, error=%d\n", length, sent, WSAGetLastError());
	OutputDebugStringA(debug);

	if (sent == SOCKET_ERROR) return false;
	return true;
}

void ClientNetwork::Receive() {
	char buffer[2000] = { 0 };
	while (m_running) {
		int received = recv(m_clientsocket, buffer + m_prevRemain, sizeof(buffer) - m_prevRemain, 0);
		char* currentBuffer = buffer;
		int remainSize = received + m_prevRemain;
		if (received == 0) {
			continue;
		}
		while (remainSize >= 5) {
			unsigned int size = 0;
			memcpy_s(&size, 4, &currentBuffer[1], 4);
			if (remainSize < size) {
				break;
			}
			switch (currentBuffer[0]) {
			case SC_PACKET_LOGIN_RESPONSE:
				ProcessLogin(currentBuffer);
				break;
			case SC_PACKET_ROOM_RESPONSE:
				ProcessRoomjoin(currentBuffer);
				break;
			case SC_PACKET_WHOISMYTEAM:
				ProcessWhoismyteam(currentBuffer);
				break;
			case SC_PACKET_GAMESTART:
				ProcessGamestart(currentBuffer);
				break;
			case SC_PACKET_INITIALSTATE:
				ProcessInitialstate(currentBuffer);
				break;
			case SC_PACKET_INITMONSTER:
				ProcessInitMonster(currentBuffer);
				break;
			case SC_PACKET_ZENITHMONSTER:
				ProcessZenithMonster(currentBuffer);
				break;
			case SC_PACKET_UPDATE2PLAYER:
				ProcessUpdateCoord2Player(currentBuffer);
				break;
			case SC_PACKET_REPAIRTIME:
				ProcessStartRepairTime(currentBuffer);
				break;
			case SC_PACKET_MONSTERHP:
				ProcessMonsterHP(currentBuffer);
				break;
			case SC_PACKET_ZMONSTERHP:
				ProcessZMonsterHP(currentBuffer);
				break;
			case SC_PACKET_DROPITEM:
				ProcessItemDrop(currentBuffer);
				break;
			case SC_PACKET_GOLD:
				ProcessGold(currentBuffer);
				break;
			case SC_PACKET_INVENTORY:
				ProcessInventory(currentBuffer);
				break;
			case SC_PACKET_SELECTITEM:
				ProcessInventory2Equip(currentBuffer);
				break;
			case SC_PACKET_DEBUGITEM:
				ProcessDebugItem(currentBuffer);
				break;
			case SC_PACKET_ITEMSTATE:
				ProcessItemState(currentBuffer);
				break;
			case SC_PACKET_ANIMATION:
				ProcessAnimation(currentBuffer);
				break;
			case SC_PACKET_ZENITHSTAGE:
				ProcessZenithStage(currentBuffer);
				break;
			case SC_PACKET_ZENITHSTATE:
				ProcessZenithState(currentBuffer);
				break;
			case SC_PACKET_CMONSTERTARGET:
				ProcessCMonsterTarget(currentBuffer);
				break;
			case SC_PACKET_RESPONE:
				ProcessRespone(currentBuffer);
				break;
			case SC_PACKET_ZMONSTERMOVE:
				ProcessZMonsterMove(currentBuffer);
				break;
			default:
				break;
			}
			remainSize -= size;
			currentBuffer = currentBuffer + size;
		}
		m_prevRemain = static_cast<uint32_t>(remainSize);
		if (remainSize > 0) {
			std::memcpy(buffer, currentBuffer, remainSize);
		}
	}
	Disconnect();
}

void ClientNetwork::ProcessLogin(char* buffer)
{
	SC_Packet_LoginResponse* pkt = reinterpret_cast<SC_Packet_LoginResponse*>(buffer);
	if (pkt->success == true) {
		gGameFramework->GetClientState()->SetIsLogin(true);
		m_clientID = pkt->clientID;
	}
}

void ClientNetwork::ProcessRoomjoin(char* buffer)
{
	SC_Packet_RoomResponse* pkt = reinterpret_cast<SC_Packet_RoomResponse*>(buffer);
	if (pkt->success)
		gGameFramework->GetClientState()->SetClientRoomNum(pkt->room_id);
}

void ClientNetwork::ProcessWhoismyteam(char* buffer)
{
	SC_Packet_MyTeam* pkt = reinterpret_cast<SC_Packet_MyTeam*>(buffer);
	for (int i = 0; i < pkt->teamSize; ++i) {
		if (pkt->teamID[i] != m_clientID)      // 내 클라면 저장안함
		{
			//@@@ = pkt->teamID[i];
			if (gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0] == -2)
				gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0] = pkt->teamID[i];
			else if (gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1] == -2)
				gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1] = pkt->teamID[i];
		}
	}
}

void ClientNetwork::ProcessGamestart(char* buffer)
{
	SC_Packet_GameStart* pkt = reinterpret_cast<SC_Packet_GameStart*>(buffer);
	if (pkt->startCS = true)
		gGameFramework->IsSuccess = true;
}

void ClientNetwork::ProcessInitialstate(char* buffer)
{
	SC_Packet_initialstate* pkt = reinterpret_cast<SC_Packet_initialstate*>(buffer);
	// 일단 자기 좌표만 받을거임.
	XMFLOAT3 pos(pkt->x, pkt->y, pkt->z);
	if (pkt->client_id == m_clientID) {
		gGameFramework->g_pos = pos;

		//auto player = gGameFramework->GetPlayer();
		//if (player)
		//   player->SetPosition(pos);
	}
	else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0])
	{
		gGameFramework->GetSceneManager()->GetCurrentScene()->otherpos[0] = pos;
	}
	else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1])
	{
		gGameFramework->GetSceneManager()->GetCurrentScene()->otherpos[1] = pos;
	}
	else
	{
		return;
	}


}

void ClientNetwork::ProcessInitMonster(char* buffer) {
	SC_Packet_InitMonster* pkt = reinterpret_cast<SC_Packet_InitMonster*>(buffer);
	for (auto i = 0; i < 50; ++i) {
		XMFLOAT3 pos(pkt->monsters[i].x, pkt->monsters[i].y, pkt->monsters[i].z);
		gGameFramework->CmonstersCoord[pkt->monsters[i].monsterid] = pos;
	}
}

void ClientNetwork::ProcessZenithMonster(char* buffer)
{
	SC_Packet_ZenithMonster* pkt = reinterpret_cast<SC_Packet_ZenithMonster*>(buffer);
	for (auto i = 0; i < 26; ++i) {
		XMFLOAT3 pos(pkt->monsters[i].x, pkt->monsters[i].y, pkt->monsters[i].z);
		gGameFramework->ZmonstersCoord[pkt->monsters[i].monsterid] = pos;
	}
}

void ClientNetwork::ProcessUpdateCoord2Player(char* buffer)
{
	SC_Packet_Update2Player* pkt = reinterpret_cast<SC_Packet_Update2Player*>(buffer);
	XMFLOAT3 pos(pkt->x, pkt->y, pkt->z);
	float angle = pkt->angle;

	if (pkt->client_id == m_clientID) {
		gGameFramework->g_pos = pos;
		// 본인 좌표는 여기까지만 처리
	}
	else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0])
	{
		auto& scene = *gGameFramework->GetSceneManager()->GetCurrentScene();
		scene.otherpos[0] = pos;
		scene.otherangle[0] = angle;
	}
	else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1])
	{
		auto& scene = *gGameFramework->GetSceneManager()->GetCurrentScene();
		scene.otherpos[1] = pos;
		scene.otherangle[1] = angle;
	}
}

// [개발중] 채팅
void ClientNetwork::ProcessChat(char* buffer)
{
	SC_Packet_Chat* pkt = reinterpret_cast<SC_Packet_Chat*>(buffer);

}

void ClientNetwork::ProcessStartRepairTime(char* buffer)
{
	SC_Packet_RepairTime* pkt = reinterpret_cast<SC_Packet_RepairTime*>(buffer);
	XMFLOAT3 pos(pkt->x, pkt->y, pkt->z);

	// 현재 씬 가져오기 (GameScene으로 캐스팅 필요)
	shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	GameScene* gameScene = dynamic_cast<GameScene*>(currentScene.get());

	if (pkt->startRT == true) {
		if (pkt->client_id == m_clientID) {
			gGameFramework->g_pos = pos;
			gGameFramework->GetSceneManager()->GetCurrentScene()->m_player->SetPosition(pos);

			//auto player = gGameFramework->GetPlayer();
			//if (player)
			//   player->SetPosition(pos);
		}
		else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0])
		{
			gGameFramework->GetSceneManager()->GetCurrentScene()->otherpos[0] = pos;
			gGameFramework->GetSceneManager()->GetCurrentScene()->m_Otherplayer[0]->m_position = pos;

		}
		else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1])
		{
			gGameFramework->GetSceneManager()->GetCurrentScene()->otherpos[1] = pos;
			gGameFramework->GetSceneManager()->GetCurrentScene()->m_Otherplayer[1]->m_position = pos;
		}
		else
		{
			return;
		}
	}
	//gameScene->m_ZenithEnabled = !m_ZenithEnabled;
	gameScene->SetZenithEnabled();
	gameScene->ActivateZenithStageMonsters();
}

void ClientNetwork::ProcessMonsterHP(char* buffer)
{
	SC_Packet_MonsterHP* pkt = reinterpret_cast<SC_Packet_MonsterHP*>(buffer);
	int monsterID = pkt->monsterID;
	int hp = pkt->monsterHP;

	// 1. ID → 타입 + 인덱스 해석
	string type;
	int index = 0;

	if (monsterID >= 0 && monsterID < 10) {
		type = "Mushroom_Dark"; index = monsterID;
	}
	else if (monsterID < 20) {
		type = "FrightFly"; index = monsterID - 10;
	}
	else if (monsterID < 30) {
		type = "Plant_Dionaea"; index = monsterID - 20;
	}
	else if (monsterID < 40) {
		type = "Venus_Blue"; index = monsterID - 30;
	}
	else if (monsterID < 50) {
		type = "Flower_Fairy"; index = monsterID - 40;
	}
	else {
		OutputDebugStringA("[ERROR] Invalid Monster ID!\n");
		return;
	}

	// 2. 현재 씬이 GameScene인지 확인 및 캐스팅
	shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	GameScene* gameScene = dynamic_cast<GameScene*>(currentScene.get());
	if (!gameScene)
	{
		OutputDebugStringA("[ERROR] Current scene is not GameScene!\n");
		return;
	}

	// 3. 몬스터 그룹에서 해당 몬스터 찾기
	auto& monsterGroups = gameScene->GetMonsterGroups();
	if (!monsterGroups.contains(type) || index >= monsterGroups[type].size())
	{
		OutputDebugStringA("[ERROR] Monster not found in group!\n");
		return;
	}

	// 4. HP 갱신
	auto& monster = monsterGroups[type][index];
	if (monster)
	{
		monster->SetHP(hp);

		// HP가 0 이하 && 아직 파티클 안 뿌렸다면
		if (hp <= 0 && !monster->IsParticleSpawned())
		{
			monster->MarkParticleSpawned(); // 한 번만 실행되게 설정

			// 파티클 스폰
			for (int i = 0; i < 100; ++i)
			{
				gameScene->GetParticleManager()->SpawnParticle(monster->GetPosition());
			}
		}

	}
}

// [개발중] 정점 몬스터 HP 갱신
void ClientNetwork::ProcessZMonsterHP(char* buffer)
{
	SC_Packet_ZMonsterHP* pkt = reinterpret_cast<SC_Packet_ZMonsterHP*>(buffer);
	int monsterID = pkt->monsterID;
	int hp = pkt->monsterHP;

	// 1. ID → 타입 + 인덱스 해석
	string type;
	int index = 0;

	if (monsterID >= 0 && monsterID < 5) {
		type = "Mushroom_Dark"; index = monsterID;
	}
	else if (monsterID < 10) {

		type = "FrightFly"; index = monsterID;
	}
	else if (monsterID < 15) {
		type = "Plant_Dionaea"; index = monsterID;
	}
	else if (monsterID < 20) {
		type = "Venus_Blue"; index = monsterID;
	}
	else if (monsterID < 25) {
		type = "Flower_Fairy"; index = monsterID;
	}
	else if (monsterID == 25) {			// 보스 몬스터
		type = "Venus_Blue"; index = monsterID;
	}
	else {
		OutputDebugStringA("[ERROR] Invalid Monster ID!\n");
		return;
	}

	// 도전 몬스터랑 동일...
}

void ClientNetwork::ProcessItemDrop(char* buffer)
{
	SC_Packet_DropItem* pkt = reinterpret_cast<SC_Packet_DropItem*>(buffer);

	// 현재 씬 가져오기 (GameScene으로 캐스팅 필요)
	shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	GameScene* gameScene = dynamic_cast<GameScene*>(currentScene.get());

	if (gameScene)
	{
		gameScene->SetInventoryCount(pkt->item, pkt->itemNum);
	}
}

void ClientNetwork::ProcessGold(char* buffer)
{
	SC_Packet_Gold* pkt = reinterpret_cast<SC_Packet_Gold*>(buffer);

	// 현재 씬 가져오기 (GameScene으로 캐스팅 필요)
	shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	GameScene* gameScene = dynamic_cast<GameScene*>(currentScene.get());

	if (gameScene)
	{
		gameScene->SetGoldScore(pkt->gold);
	}

}

void ClientNetwork::ProcessInventory(char* buffer)
{
	shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	GameScene* gameScene = dynamic_cast<GameScene*>(currentScene.get());

	SC_Packet_Inventory* pkt = reinterpret_cast<SC_Packet_Inventory*>(buffer);
	int item = pkt->item - 1;
	int itemNum = pkt->num;

	if (gameScene)
	{
		gameScene->SetInventoryCount(item, itemNum);
	}
}

// [개발중] 해당 클라의 장비창에 해당 아이템을 장착하기 / 상대방 직업 및 장착 무기 패킷
void ClientNetwork::ProcessInventory2Equip(char* buffer)
{
	SC_Packet_SelectItem* pkt = reinterpret_cast<SC_Packet_SelectItem*>(buffer);
	pkt->clientID;			// 내껀지 다른 사람껀지
	pkt->item;				// 전직서인지 무기인지
}

void ClientNetwork::ProcessDebugItem(char* buffer)
{
	SC_Packet_DebugItem* pkt = reinterpret_cast<SC_Packet_DebugItem*>(buffer);

	// 현재 씬 가져오기 (GameScene으로 캐스팅 필요)
	shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	GameScene* gameScene = dynamic_cast<GameScene*>(currentScene.get());

	if (gameScene)
	{
		gameScene->SetInventoryCount(pkt->item, pkt->itemNum);
	}
}

void ClientNetwork::ProcessItemState(char* buffer)
{
	SC_Packet_ItemState* pkt = reinterpret_cast<SC_Packet_ItemState*>(buffer);

	// 현재 씬 가져오기 (GameScene으로 캐스팅 필요)
	shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	GameScene* gameScene = dynamic_cast<GameScene*>(currentScene.get());

	if (gameScene)
	{
		gameScene->SetupgradeScore(pkt->result);
		gameScene->UpdateEnhanceDigits(); // 강화 수치 UI 갱신
	}
}

void ClientNetwork::ProcessAnimation(char* buffer)
{
	SC_Packet_Animaition* pkt = reinterpret_cast<SC_Packet_Animaition*>(buffer);

	//if (pkt->animation == 3) {		// 0 == 펀치
	//	if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0])
	//	{
	//		shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	//		currentScene->m_Otherplayer[0]->SetCurrentAnimation("Punch.001");
	//	}
	//	else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1])
	//	{
	//		shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	//		currentScene->m_Otherplayer[1]->SetCurrentAnimation("Punch.001");
	//	}
	//}
	//else if(pkt->animation == 2)
	//{		// 1 == 달리기
	//	if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0])
	//	{
	//		shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	//		currentScene->m_Otherplayer[0]->m_currentAnim = "Running";
	//	}
	//	else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1])
	//	{
	//		shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	//		currentScene->m_Otherplayer[1]->m_currentAnim = "Running";
	//	}
	//}

	switch (pkt->animation) {
	case 0:  // idle
		if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			if (currentScene->m_Otherplayer[0])currentScene->m_Otherplayer[0]->m_CurrentAnim = pkt->animation;
		}
		else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			if (currentScene->m_Otherplayer[1])currentScene->m_Otherplayer[1]->m_CurrentAnim = pkt->animation;
		}
		break;
	case 1:  // walking
		if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			currentScene->m_Otherplayer[0]->m_CurrentAnim = pkt->animation;
		}
		else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			currentScene->m_Otherplayer[1]->m_CurrentAnim = pkt->animation;
		}
		break;
	case 2:  // running
		if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			currentScene->m_Otherplayer[0]->m_CurrentAnim = pkt->animation;
		}
		else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			currentScene->m_Otherplayer[1]->m_CurrentAnim = pkt->animation;
		}
		break;
	case 3:  // punching
		if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			currentScene->m_Otherplayer[0]->m_CurrentAnim = pkt->animation;
		}
		else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			currentScene->m_Otherplayer[1]->m_CurrentAnim = pkt->animation;
		}
		break;
	case 4: // 전사
		if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			currentScene->m_Otherplayer[0]->m_CurrentAnim = pkt->animation;
		}
		else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			currentScene->m_Otherplayer[1]->m_CurrentAnim = pkt->animation;
		}
		break;
	case 5: // 법사
		if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			currentScene->m_Otherplayer[0]->m_CurrentAnim = pkt->animation;
		}
		else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			currentScene->m_Otherplayer[1]->m_CurrentAnim = pkt->animation;
		}
		break;
	case 6: // 힐탱커
		if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			currentScene->m_Otherplayer[0]->m_CurrentAnim = pkt->animation;

			auto currentScene2 = gGameFramework->GetSceneManager()->GetCurrentScene();
			auto gameScene = std::dynamic_pointer_cast<GameScene>(currentScene2);
			if (gameScene) {
				gameScene->SpawnHealingObject(0);

			}
		}
		else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			currentScene->m_Otherplayer[1]->m_CurrentAnim = pkt->animation;
			
			auto currentScene2 = gGameFramework->GetSceneManager()->GetCurrentScene();
			auto gameScene = std::dynamic_pointer_cast<GameScene>(currentScene2);
			if (gameScene) {
				gameScene->SpawnHealingObject(1);

			}
		}
		break;
	case 7: // 전사 기본 공격

		break;
	case 8: // 마법사 기본 공격
		auto currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
		auto gameScene = std::dynamic_pointer_cast<GameScene>(currentScene);

		if (!gameScene) break;

		// 플레이어 0
		if (pkt->client_id == currentScene->otherid[0])
		{
			currentScene->m_Otherplayer[0]->m_CurrentAnim = pkt->animation;
			gameScene->FireMagicBall(0); //0번 플레이어 기준 발사
		}
		// 플레이어 1
		else if (pkt->client_id == currentScene->otherid[1])
		{
			currentScene->m_Otherplayer[1]->m_CurrentAnim = pkt->animation;
			gameScene->FireMagicBall(1); //1번 플레이어 기준 발사
		}
		break;
	}
}

// [개발중] 전사, 마법사 기본 공격 및 스킬 공격 이펙트 부분
void ClientNetwork::ProcessAttackEffect(char* buffer)
{
	SC_Packet_AttackEffect* pkt = reinterpret_cast<SC_Packet_AttackEffect*>(buffer);
	pkt->targetID;				// 어떤 플레이어가 쓰는거임?  스킬 이펙트가 나오는 위치도 targetID로 알수있어
	pkt->skill;					// 무슨 스킬??
	pkt->angle;
}

void ClientNetwork::ProcessZenithState(char* buffer)
{
	SC_Packet_Zenithstate* pkt = reinterpret_cast<SC_Packet_Zenithstate*>(buffer);
	XMFLOAT3 pos(pkt->x, pkt->y, pkt->z);

	// 현재 씬 가져오기 (GameScene으로 캐스팅 필요)
	shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	GameScene* gameScene = dynamic_cast<GameScene*>(currentScene.get());

	if (pkt->client_id == m_clientID) {
		gGameFramework->g_pos = pos;
		gGameFramework->GetSceneManager()->GetCurrentScene()->m_player->SetPosition(pos);

		//auto player = gGameFramework->GetPlayer();
		//if (player)
		//   player->SetPosition(pos);
	}
	else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0])
	{
		gGameFramework->GetSceneManager()->GetCurrentScene()->otherpos[0] = pos;
		gGameFramework->GetSceneManager()->GetCurrentScene()->m_Otherplayer[0]->m_position = pos;

	}
	else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1])
	{
		gGameFramework->GetSceneManager()->GetCurrentScene()->otherpos[1] = pos;
		gGameFramework->GetSceneManager()->GetCurrentScene()->m_Otherplayer[1]->m_position = pos;
	}
	else
	{
		return;
	}
}

void ClientNetwork::ProcessZenithStage(char* buffer)
{
	SC_Packet_ZenithStage* pkt = reinterpret_cast<SC_Packet_ZenithStage*>(buffer);
	if (pkt->startZS = true) {
		gGameFramework->IsSuccess2 = true;

		CS_Packet_ZenithReady pkt;
		pkt.type = CS_PACKET_ZENITHREADY;
		pkt.ReadySuccess = true;
		pkt.size = sizeof(pkt);

		gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);
	}
}

// [개발중] 도전스테이지에서 누구 쳐다보고있어?
void ClientNetwork::ProcessCMonsterTarget(char* buffer)
{
	SC_Packet_CMonsterTarget* pkt = reinterpret_cast<SC_Packet_CMonsterTarget*>(buffer);
	pkt->monsterID;
	pkt->targetID;
}

// [개발중] 도전, 정점에서 피가 0이 되면 다시 태어나는 곳 지정해주기
void ClientNetwork::ProcessRespone(char* buffer)
{
	SC_Packet_Respone* pkt = reinterpret_cast<SC_Packet_Respone*>(buffer);
	pkt->clientID;
	pkt->x;
	pkt->y;
	pkt->z;
	// ProcessWhoisMyteam()부분보고 하면 돼. id랑 xyz값 넘겨주니까 그걸로 상대방 위치 보내주면 돼.
}

void ClientNetwork::ProcessZMonsterMove(char* buffer)
{
	SC_Packet_ZMonsterMove* pkt = reinterpret_cast<SC_Packet_ZMonsterMove*>(buffer);

	gGameFramework->ZmonstersCoord[pkt->monsterID].x = pkt->x;
	gGameFramework->ZmonstersCoord[pkt->monsterID].y = pkt->y;
	gGameFramework->ZmonstersCoord[pkt->monsterID].z = pkt->z;
}

// [개발중] HP바와 실제 체력 연동
void ClientNetwork::ProcessPlayerHP(char* buffer)
{
	SC_Packet_PlayerHP* pkt = reinterpret_cast<SC_Packet_PlayerHP*>(buffer);
	pkt->hp;
}

// [개발중] 보스 잡고 난 후 게임 종료
void ClientNetwork::ProcessEndGame(char* buffer)
{
	SC_Packet_EndGame* pkt = reinterpret_cast<SC_Packet_EndGame*>(buffer);
	pkt->time;		
	// 클리어 시간 몇초인지, 만약 300초로 왔으면 실패, 300초 안으로 들어왔으면 성공
	// 씬 전환하지말고 그냥 우리 장비창 처럼 사진으로 하고 클리어 초만 넣어두면 될거같아.
	// 그리고 10초 뒤에 자동으로 방 선택 창으로 돌아가게 씬 전환
}



// [서버 개발 완료 -> 클라쪽에서 진행할 목록]
// 밑에 패킷이라고 되어 있는건 클라에서 서버에 패킷을 보내야 되는 거 적어놓은거야.
// "패킷 : X " 는 이미 다 처리되어 있으니까 서버에서 보낸 패킷가지고 정보 갖고 노는거야. 
// 서버에 패킷 전달하는 방식은 전이랑 동일하고 프로토콜화 다 해놨으니까 pkt.치면 뭐 보내야는지 다 나와.
// "패킷 : ~ " 는 클라에서 서버에 보내줘야하는게 있으니까 해결해야함 이거야~
// 
// 
// 1. 정점 몬스터 렌더링 수정 [O]				/    패킷 : X					   /    예상 결과 : 모든 플레이어가 동일한 위치의 정점 몬스터가 보임
// ->  ProcessZenithMonster함수에서 정점 몬스터 초기 좌표 받아오는 곳이야.
// 2. ProcessZMonsterHP함수 패킷 처리 []		/	 패킷 : X					   /	예상 결과 : 정점 몬스터 체력 동기화, 피 깎이면 모든 플레이어가 보기에 다 깎이는게 보임
// 3. ProcessPlayerHP함수 패킷 처리	 []			/	 패킷 : X					   /    예상 결과 : HP바와 실제 체력 연동
// 4. ProcessRespone함수 패킷 처리	[]			/	 패킷 : X					   /    예상 결과 : HP가 0이 되면 시작 지점으로 이동
// 5. 떨어져있는 힐팩 먹었을때	[]				/    패킷 : CS_Packet_HealPack     /    예상 결과 : HP 증가
// 6. 플레이어 스킬 이펙트	[]					/    패킷 : CS_Packet_AttackEffect /    예상 결과 : 서버에 어떤 플레이어가 이런 스킬 썼으니까 그 자리에 이펙트좀 넣어줘라고 주문넣음 
// -> skill(0 ~ 3)                // 0. 전사 기본 공격 이펙트 1. 전사 스킬 공격 이펙트 2. 마법사 기본 공격 이펙트 3. 마법사 스킬 공격 이펙트
// 7. ProcessAttackEffect함수 패킷 처리	[]		/	 패킷 : X					   /	예상 결과 : 타 플레이어의 스킬 및 기본 공격 이펙트가 보임
// 8. ProcessCMonsterTarrget함수 패킷 처리	[]	/    패킷 : X					   /	예상 결과 : 도전 몬스터가 가장 가까운 플레이어를 쳐다봄
// 9. ProcessEndGame함수 패킷 처리	[]			/    패킷 : X					   /	예상 결과 : 보스 몬스터를 잡았을 때 혹은 300초가 지나면 게임 클리어 현황 나오고 10초 뒤에 자동으로 방 선택창으로 돌아감.
// 
//
// 개발하다가 생각나는거 있음 더 정리해놓을게
// [개발중]이라고 되어있는게 있을텐데 그건 서버에서 보낸 패킷 처리만 하면 되는거야
// 졸작 마무리 화이팅!