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
			case SC_PACKET_ZMONSTERTARGET:
				ProcessZMonsterTarget(currentBuffer);
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
		gGameFramework->monstersCoord[pkt->monsters[i].monsterid] = pos;
	}
}

void ClientNetwork::ProcessZenithMonster(char* buffer)
{
	SC_Packet_ZenithMonster* pkt = reinterpret_cast<SC_Packet_ZenithMonster*>(buffer);

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

void ClientNetwork::ProcessInventory2Equip(char* buffer)
{
	SC_Packet_SelectItem* pkt = reinterpret_cast<SC_Packet_SelectItem*>(buffer);
	pkt->item;
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
			if(currentScene->m_Otherplayer[0])currentScene->m_Otherplayer[0]->m_CurrentAnim = pkt->animation;
		}
		else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			if(currentScene->m_Otherplayer[1])currentScene->m_Otherplayer[1]->m_CurrentAnim = pkt->animation;
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
	}
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

void ClientNetwork::ProcessCMonsterTarget(char* buffer)
{
	SC_Packet_CMonsterTarget* pkt = reinterpret_cast<SC_Packet_CMonsterTarget*>(buffer);
	pkt->monsterID;
	pkt->targetID;
}

void ClientNetwork::ProcessZMonsterTarget(char* buffer)
{
	SC_Packet_ZMonsterTarget* pkt = reinterpret_cast<SC_Packet_ZMonsterTarget*>(buffer);
	pkt->monsterID;
	pkt->targetID;
}