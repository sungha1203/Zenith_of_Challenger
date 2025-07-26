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
		MessageBoxA(nullptr, "������ ������ �� �����ϴ�.", "���� ����", MB_OK | MB_ICONERROR);
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
			case SC_PACKET_CUSTOMIZE:
				ProcessCustomize(currentBuffer);
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
			case SC_PACKET_PLAYERATTACK:
				ProcessPlayerAttack(currentBuffer);
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
			case SC_PACKET_ZMONSTERATTACK:
				ProcessZMonsterAttack(currentBuffer);
				break;
			case SC_PACKET_PLAYERHP:
				ProcessPlayerHP(currentBuffer);
				break;
			case SC_PACKET_ATTACKEFFECT:
				ProcessAttackEffect(currentBuffer);
				break;
			case SC_PACKET_ENDGAME:
				ProcessEndGame(currentBuffer);
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
		if (pkt->teamID[i] != m_clientID)      // �� Ŭ��� �������
		{
			//@@@ = pkt->teamID[i];
			if (gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0] == -2)
				gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0] = pkt->teamID[i];
			else if (gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1] == -2)
				gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1] = pkt->teamID[i];
		}
	}
}

// [������] Ŀ���͸���¡
void ClientNetwork::ProcessCustomize(char* buffer)
{
	SC_Packet_Customize* pkt = reinterpret_cast<SC_Packet_Customize*>(buffer);
	pkt->clientID;				// �� ���̾�? �� ���̾�?
	pkt->clothes;				// ���� ������
}

void ClientNetwork::ProcessGamestart(char* buffer)
{
	SC_Packet_GameStart* pkt = reinterpret_cast<SC_Packet_GameStart*>(buffer);

	//auto gameScene = dynamic_cast<GameScene*>(gGameFramework->GetSceneManager()->GetCurrentScene().get());

	if (pkt->startCS = true) {
		gGameFramework->IsSuccess = true;
		//gameScene->SetZenithStart(true);
		//gameScene->SetCameraToggle();
	}
}

void ClientNetwork::ProcessInitialstate(char* buffer)
{
	SC_Packet_initialstate* pkt = reinterpret_cast<SC_Packet_initialstate*>(buffer);
	// �ϴ� �ڱ� ��ǥ�� ��������.
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
		// ���� ��ǥ�� ��������� ó��
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

// [������] ä��
void ClientNetwork::ProcessChat(char* buffer)
{
	SC_Packet_Chat* pkt = reinterpret_cast<SC_Packet_Chat*>(buffer);

}

void ClientNetwork::ProcessStartRepairTime(char* buffer)
{
	SC_Packet_RepairTime* pkt = reinterpret_cast<SC_Packet_RepairTime*>(buffer);
	XMFLOAT3 pos(pkt->x, pkt->y, pkt->z);

	// ���� �� �������� (GameScene���� ĳ���� �ʿ�)
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

	// 1. ID �� Ÿ�� + �ε��� �ؼ�
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

	// 2. ���� ���� GameScene���� Ȯ�� �� ĳ����
	shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	GameScene* gameScene = dynamic_cast<GameScene*>(currentScene.get());
	if (!gameScene)
	{
		OutputDebugStringA("[ERROR] Current scene is not GameScene!\n");
		return;
	}

	// 3. ���� �׷쿡�� �ش� ���� ã��
	auto& monsterGroups = gameScene->GetMonsterGroups();
	if (!monsterGroups.contains(type) || index >= monsterGroups[type].size())
	{
		OutputDebugStringA("[ERROR] Monster not found in group!\n");
		return;
	}

	// 4. HP ����
	auto& monster = monsterGroups[type][index];
	if (monster)
	{
		monster->SetHP(hp);

		// HP�� 0 ���� && ���� ��ƼŬ �� �ѷȴٸ�
		if (hp <= 0 && !monster->IsParticleSpawned())
		{
			monster->PlayAnimationWithBlend("Die", 0.2f);
			monster->MarkParticleSpawned(); // �� ���� ����ǰ� ����

			// ��ƼŬ ����
			for (int i = 0; i < 100; ++i)
			{
				gameScene->GetParticleManager()->SpawnParticle(monster->GetPosition());
			}
		}

	}
}

// [������] ���� ���� HP ����
void ClientNetwork::ProcessZMonsterHP(char* buffer)
{
	SC_Packet_ZMonsterHP* pkt = reinterpret_cast<SC_Packet_ZMonsterHP*>(buffer);
	int monsterID = pkt->monsterID;
	int hp = pkt->monsterHP;

	// 1. ID �� Ÿ�� + �ε��� �ؼ�
	std::string type;
	int index = 0;

	if (monsterID >= 0 && monsterID < 5) {
		type = "Mushroom_Dark"; index = monsterID;
	}
	else if (monsterID < 10) {
		type = "FrightFly"; index = monsterID - 5;
	}
	else if (monsterID < 15) {
		type = "Plant_Dionaea"; index = monsterID - 10;
	}
	else if (monsterID < 20) {
		type = "Venus_Blue"; index = monsterID - 15;
	}
	else if (monsterID < 25) {
		type = "Flower_Fairy"; index = monsterID - 20;
	}
	else if (monsterID == 25) {
		type = "Metalon"; index = 0;
	}
	else {
		return;
	}

	// 2. ���� ���� GameScene���� Ȯ��
	shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	GameScene* gameScene = dynamic_cast<GameScene*>(currentScene.get());
	if (!gameScene)
	{
		return;
	}

	// 3. ���� �������� ���� �׷� ����
	auto& monsterGroups = gameScene->GetMonsterGroups();
	if (!monsterGroups.contains(type) || index >= monsterGroups[type].size())
	{
		return;
	}

	// 4. ü�� ����ȭ �� ó��
	auto& monster = monsterGroups[type][index];
	if (monster)
	{
		monster->SetHP(hp);
	}
}

void ClientNetwork::ProcessItemDrop(char* buffer)
{
	SC_Packet_DropItem* pkt = reinterpret_cast<SC_Packet_DropItem*>(buffer);

	// ���� �� �������� (GameScene���� ĳ���� �ʿ�)
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

	// ���� �� �������� (GameScene���� ĳ���� �ʿ�)
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

// [������] �ش� Ŭ���� ���â�� �ش� �������� �����ϱ� / ���� ���� �� ���� ���� ��Ŷ
void ClientNetwork::ProcessInventory2Equip(char* buffer)
{
	SC_Packet_SelectItem* pkt = reinterpret_cast<SC_Packet_SelectItem*>(buffer);
	pkt->clientID;			// ������ �ٸ� �������
	pkt->item;				// ���������� ��������

	shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	GameScene* gameScene = dynamic_cast<GameScene*>(currentScene.get());
	if (gameScene && pkt->item >= 1 && pkt->item < 4)
	{
		// otherid[0] �Ǵ� otherid[1] �� ��Ī�Ǵ��� Ȯ��
		if (pkt->clientID == m_clientID)
		{
			gameScene->m_WhatGrab = pkt->item;
		}
		else if (pkt->clientID == gameScene->otherid[0])
		{
			gameScene->m_WhatOtherGrab[0] = pkt->item;
		}
		else if (pkt->clientID == gameScene->otherid[1])
		{
			gameScene->m_WhatOtherGrab[1] = pkt->item;
		}
	}
	if (gameScene && pkt->item == 4) // ����
	{
		// otherid[0] �Ǵ� otherid[1] �� ��Ī�Ǵ��� Ȯ��
		if (pkt->clientID == m_clientID)
		{
			gameScene->m_player->SetPosition(gGameFramework->g_pos);
		}
		else if (pkt->clientID == gameScene->otherid[0])
		{
			gameScene->SetOtherJob1(0);
			gameScene->m_Otherplayer[0]->SetPosition(gameScene->otherpos[0]);
		}
		else if (pkt->clientID == gameScene->otherid[1])
		{
			gameScene->SetOtherJob2(1);
			gameScene->m_Otherplayer[1]->SetPosition(gameScene->otherpos[1]);
		}
	}
	if (gameScene && pkt->item == 5) // ������
	{
		// otherid[0] �Ǵ� otherid[1] �� ��Ī�Ǵ��� Ȯ��
		if (pkt->clientID == m_clientID)
		{
			gameScene->m_player->SetPosition(gGameFramework->g_pos);
		}
		else if (pkt->clientID == gameScene->otherid[0])
		{
			gameScene->SetOtherJob1(2);
		}
		else if (pkt->clientID == gameScene->otherid[1])
		{
			gameScene->SetOtherJob2(3);
		}
	}
	if (gameScene && pkt->item == 6) // ����Ŀ
	{
		// otherid[0] �Ǵ� otherid[1] �� ��Ī�Ǵ��� Ȯ��
		if (pkt->clientID == m_clientID)
		{
			gameScene->m_player->SetPosition(gGameFramework->g_pos);
		}
		else if (pkt->clientID == gameScene->otherid[0])
		{
			gameScene->SetOtherJob1(4);
		}
		else if (pkt->clientID == gameScene->otherid[1])
		{
			gameScene->SetOtherJob2(5);
		}
	}
}

void ClientNetwork::ProcessDebugItem(char* buffer)
{
	SC_Packet_DebugItem* pkt = reinterpret_cast<SC_Packet_DebugItem*>(buffer);

	// ���� �� �������� (GameScene���� ĳ���� �ʿ�)
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

	// ���� �� �������� (GameScene���� ĳ���� �ʿ�)
	shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	GameScene* gameScene = dynamic_cast<GameScene*>(currentScene.get());

	if (gameScene)
	{
		gameScene->SetupgradeScore(pkt->result);
		gameScene->UpdateEnhanceDigits(); // ��ȭ ��ġ UI ����
	}
}

void ClientNetwork::ProcessAnimation(char* buffer)
{
	SC_Packet_Animaition* pkt = reinterpret_cast<SC_Packet_Animaition*>(buffer);

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
		if (pkt->client_id == m_clientID)
		{
			auto gameScene = dynamic_cast<GameScene*>(gGameFramework->GetSceneManager()->GetCurrentScene().get());
			gameScene->m_player->SetCurrentAnimation("Kick");
			//gameScene->m_player->isPunching = true;
		}
		else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			//currentScene->m_Otherplayer[0]->m_CurrentAnim = pkt->animation;
			currentScene->m_Otherplayer[0]->SetCurrentAnimation("Kick");
			currentScene->m_Otherplayer[0]->isAttacking = true;;
		}
		else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			//currentScene->m_Otherplayer[1]->m_CurrentAnim = pkt->animation;
			currentScene->m_Otherplayer[1]->SetCurrentAnimation("Kick");
			currentScene->m_Otherplayer[1]->isAttacking = true;;
		}
		break;
	case 4: // ����
		if (pkt->client_id == m_clientID)
		{
			auto gameScene = dynamic_cast<GameScene*>(gGameFramework->GetSceneManager()->GetCurrentScene().get());
			gameScene->ActivateSwordAuraSkill(0);
			gameScene->m_player->SetCurrentAnimation("Goong");
			g_Sound.PlaySoundEffect("Sounds/Sword Magic Sound Effect.wav");

		}
		else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			currentScene->m_Otherplayer[0]->m_CurrentAnim = pkt->animation;
			auto gameScene = dynamic_cast<GameScene*>(gGameFramework->GetSceneManager()->GetCurrentScene().get());
			gameScene->ActivateSwordAuraSkill(1);
			gameScene->m_Otherplayer[0]->SetCurrentAnimation("Goong");
			g_Sound.PlaySoundEffect("Sounds/Sword Magic Sound Effect.wav");
			g_Sound.SetSFXVolume(0.2f);
		}
		else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			currentScene->m_Otherplayer[1]->m_CurrentAnim = pkt->animation;
			auto gameScene = dynamic_cast<GameScene*>(gGameFramework->GetSceneManager()->GetCurrentScene().get());
			gameScene->ActivateSwordAuraSkill(2);
			gameScene->m_Otherplayer[1]->SetCurrentAnimation("Goong");
			g_Sound.PlaySoundEffect("Sounds/Sword Magic Sound Effect.wav");
			g_Sound.SetSFXVolume(0.2f);
		}
		break;
	case 5: // ����
	{
		auto currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
		auto gameScene = std::dynamic_pointer_cast<GameScene>(currentScene);

		if (!gameScene) break;

		if (pkt->client_id == m_clientID)
		{
			gameScene->SetWizardSkillAttack(1); //�÷��̾� ���� �߻�
			gameScene->m_player->SetCurrentAnimation("Goong");
			g_Sound.PlaySoundEffect("Sounds/Wizzad_Skill.wav");
		}
		// �÷��̾� 0
		if (pkt->client_id == currentScene->otherid[0])
		{
			currentScene->m_Otherplayer[0]->m_CurrentAnim = pkt->animation;
			gameScene->SetWizardSkillAttack(2); //Ÿ Ŭ�� 1�� �÷��̾� ���� �߻�
			gameScene->m_Otherplayer[0]->SetCurrentAnimation("Goong");
			gameScene->m_Otherplayer[0]->isAttacking = true;
			g_Sound.PlaySoundEffect("Sounds/Wizzad_Skill.wav");
			g_Sound.SetSFXVolume(0.2f);
		}
		// �÷��̾� 1
		else if (pkt->client_id == currentScene->otherid[1])
		{
			currentScene->m_Otherplayer[1]->m_CurrentAnim = pkt->animation;
			gameScene->SetWizardSkillAttack(3); //Ÿ Ŭ�� 1�� �÷��̾� ���� �߻�
			gameScene->m_Otherplayer[1]->SetCurrentAnimation("Goong");
			gameScene->m_Otherplayer[1]->isAttacking = true;
			g_Sound.PlaySoundEffect("Sounds/Wizzad_Skill.wav");
			g_Sound.SetSFXVolume(0.2f);
		}
	}
	break;
	case 6: // ����Ŀ
		if (pkt->client_id == m_clientID)
		{
			auto currentScene2 = gGameFramework->GetSceneManager()->GetCurrentScene();
			auto gameScene = std::dynamic_pointer_cast<GameScene>(currentScene2);
			if (gameScene) {
				gameScene->SpawnHealingObject(2);
				gameScene->m_player->SetCurrentAnimation("Goong");
			}
		}

		if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			currentScene->m_Otherplayer[0]->m_CurrentAnim = pkt->animation;

			auto currentScene2 = gGameFramework->GetSceneManager()->GetCurrentScene();
			auto gameScene = std::dynamic_pointer_cast<GameScene>(currentScene2);
			if (gameScene) {
				gameScene->SpawnHealingObject(0);
				gameScene->m_Otherplayer[0]->SetCurrentAnimation("Goong");
			}
		}

		if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			currentScene->m_Otherplayer[1]->m_CurrentAnim = pkt->animation;

			auto currentScene2 = gGameFramework->GetSceneManager()->GetCurrentScene();
			auto gameScene = std::dynamic_pointer_cast<GameScene>(currentScene2);
			if (gameScene) {
				gameScene->SpawnHealingObject(1);
				gameScene->m_Otherplayer[1]->SetCurrentAnimation("Goong");
			}
		}
		break;
	case 7: // ���� �⺻ ����
		if (pkt->client_id == m_clientID)
		{
			auto currentScene2 = gGameFramework->GetSceneManager()->GetCurrentScene();
			auto gameScene = std::dynamic_pointer_cast<GameScene>(currentScene2);
			if (gameScene) {
				gameScene->m_player->SetCurrentAnimation("Slash");
			}
		}
		// �÷��̾� 0
		if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			//currentScene->m_Otherplayer[0]->m_CurrentAnim = pkt->animation;
			currentScene->m_Otherplayer[0]->SetCurrentAnimation("Slash");
			currentScene->m_Otherplayer[0]->isAttacking = true;
		}
		// �÷��̾� 1
		else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			//currentScene->m_Otherplayer[1]->m_CurrentAnim = pkt->animation;
			currentScene->m_Otherplayer[1]->SetCurrentAnimation("Slash");
			currentScene->m_Otherplayer[1]->isAttacking = true;
		}

		break;
	case 8: // ������ �⺻ ����
	{
		auto currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
		auto gameScene = std::dynamic_pointer_cast<GameScene>(currentScene);

		if (!gameScene) break;
		if (pkt->client_id == m_clientID)
		{
			gameScene->SetWizardNormalAttack(1); //�÷��̾� ���� �߻�
			gameScene->m_player->SetCurrentAnimation("Slash");

		}
		// �÷��̾� 0
		if (pkt->client_id == currentScene->otherid[0])
		{
			currentScene->m_Otherplayer[0]->m_CurrentAnim = pkt->animation;
			gameScene->SetWizardNormalAttack(2);  //Ÿ Ŭ�� 1�� �÷��̾� ���� �߻�
			gameScene->m_Otherplayer[0]->SetCurrentAnimation("Slash");
		}
		// �÷��̾� 1
		else if (pkt->client_id == currentScene->otherid[1])
		{
			currentScene->m_Otherplayer[1]->m_CurrentAnim = pkt->animation;
			gameScene->SetWizardNormalAttack(3);  //Ÿ Ŭ�� 2�� �÷��̾� ���� �߻�
			gameScene->m_Otherplayer[1]->SetCurrentAnimation("Slash");
		}
	}
	break;
	case 9:
	{
		//if (pkt->client_id == m_clientID)
		//{
		//	auto currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
		//	auto gameScene = std::dynamic_pointer_cast<GameScene>(currentScene);
		//	if (gameScene) {
		//		gameScene->m_player->SetCurrentAnimation("Slash");
		//	}
		//}
		// �÷��̾� 0
		if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			//currentScene->m_Otherplayer[0]->m_CurrentAnim = pkt->animation;
			currentScene->m_Otherplayer[0]->SetCurrentAnimation("Die");
			currentScene->m_Otherplayer[0]->isAttacking = true;
		}
		// �÷��̾� 1
		else if (pkt->client_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1])
		{
			shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
			//currentScene->m_Otherplayer[1]->m_CurrentAnim = pkt->animation;
			currentScene->m_Otherplayer[1]->SetCurrentAnimation("Die");
			currentScene->m_Otherplayer[1]->isAttacking = true;
		}
	}
		break;
	}
}

void ClientNetwork::ProcessPlayerAttack(char* buffer)
{
	SC_Packet_PlayerAttack* pkt = reinterpret_cast<SC_Packet_PlayerAttack*>(buffer);
	pkt->normalAttack;
	pkt->skillAttack;

	shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	GameScene* gameScene = dynamic_cast<GameScene*>(currentScene.get());

	gameScene->SetskillAttack(pkt->skillAttack);
	gameScene->SetnormalAttack(pkt->normalAttack);

}

// [������] ����, ������ �⺻ ���� �� ��ų ���� ����Ʈ �κ�
void ClientNetwork::ProcessAttackEffect(char* buffer)
{
	SC_Packet_AttackEffect* pkt = reinterpret_cast<SC_Packet_AttackEffect*>(buffer);
	pkt->targetID;				// � �÷��̾ ���°���?  ��ų ����Ʈ�� ������ ��ġ�� targetID�� �˼��־�
	pkt->skill;					// ���� ��ų??
	pkt->angle;
	// 0. ���� �⺻ ���� ����Ʈ,    1. ���� ��ų ���� ����Ʈ,    2. ������ �⺻ ���� ����Ʈ,    3. ������ ��ų ���� ����Ʈ

	auto currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	auto gameScene = std::dynamic_pointer_cast<GameScene>(currentScene);
	//switch (pkt->skill) {
	//case 0:
	//	break;
	//case 1:
	//	break;
	//case 2:
	//	//if (pkt->targetID == m_clientID)
	//	//{
	//	//	gameScene->FireMagicBall(0, 0); //�÷��̾� ���� �߻�
	//	//}
	//	//else if (pkt->targetID == currentScene->otherid[0])
	//	//{
	//	//	gameScene->FireMagicBall(1, pkt->angle); //Ÿ Ŭ�� 1�� �÷��̾� ���� �߻�
	//	//}
	//	//else if (pkt->targetID == currentScene->otherid[1])
	//	//{
	//	//	gameScene->FireMagicBall(2, pkt->angle); //Ÿ Ŭ�� 2�� �÷��̾� ���� �߻�
	//	//}
	//	break;
	//case 3:
	//	gameScene->FireUltimateBulletRain(pkt->targetID, pkt->angle);
	//	break;
	//}
}

void ClientNetwork::ProcessZenithState(char* buffer)
{
	SC_Packet_Zenithstate* pkt = reinterpret_cast<SC_Packet_Zenithstate*>(buffer);
	XMFLOAT3 pos(pkt->x, pkt->y, pkt->z);

	// ���� �� �������� (GameScene���� ĳ���� �ʿ�)
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

// [������] ���������������� ���� �Ĵٺ����־�?
void ClientNetwork::ProcessCMonsterTarget(char* buffer)
{
	SC_Packet_CMonsterTarget* pkt = reinterpret_cast<SC_Packet_CMonsterTarget*>(buffer);
	pkt->monsterID;
	pkt->targetID;
}

// [������] ����, �������� �ǰ� 0�� �Ǹ� �ٽ� �¾�� �� �������ֱ�
void ClientNetwork::ProcessRespone(char* buffer)
{

	SC_Packet_Respone* pkt = reinterpret_cast<SC_Packet_Respone*>(buffer);
	// ���� �� �������� (GameScene���� ĳ���� �ʿ�)
	shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	GameScene* gameScene = dynamic_cast<GameScene*>(currentScene.get());
	
	CS_Packet_Animaition Diepkt;
	Diepkt.type = CS_PACKET_ANIMATION;
	Diepkt.animation = 9;
	Diepkt.size = sizeof(Diepkt);
	gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&Diepkt), Diepkt.size);

	pkt->clientID;
	pkt->x;
	pkt->y;
	pkt->z;
	XMFLOAT3 pos = { pkt->x, pkt->y, pkt->z };
	if (pkt->clientID == m_clientID)
	{
		gameScene->m_player->SetCurrentAnimation("Die");
		gameScene->m_player->m_isDying = true;
		gameScene->m_player->responePos = pos;
	}
	//else if (pkt->clientID == gameScene->otherid[0])
	//{
	//	gameScene->m_Otherplayer[0]->m_position = pos; 
	//}
	//else if (pkt->clientID == gameScene->otherid[1])
	//{
	//	gameScene->m_Otherplayer[1]->m_position = pos; 
	//}	
}

// [������] ���� ���� �ٶ󺸴� ������ ��ǥ
void ClientNetwork::ProcessZMonsterMove(char* buffer)
{
	SC_Packet_ZMonsterMove* pkt = reinterpret_cast<SC_Packet_ZMonsterMove*>(buffer);
	if (pkt->monsterID == 25)
	{
		gGameFramework->BossCoord.x = pkt->x;
		gGameFramework->BossCoord.y = pkt->y;
		gGameFramework->BossCoord.z = pkt->z;
		XMFLOAT3 toWard = {
			pkt->targetX - pkt->x,
			0.f, // Y�� ȸ���̹Ƿ� ���� ����
			pkt->targetZ - pkt->z
		};
		toWard = Vector3::Normalize(toWard);
		// [3] ȸ�� ���� ��� (Z ����)
		float angle = atan2f(toWard.x, toWard.z); // x/z
		float degrees = XMConvertToDegrees(angle);
		gGameFramework->BossToward = degrees + 180;
	}
	else
	{
		gGameFramework->ZmonstersCoord[pkt->monsterID].x = pkt->x;
		gGameFramework->ZmonstersCoord[pkt->monsterID].y = pkt->y;
		gGameFramework->ZmonstersCoord[pkt->monsterID].z = pkt->z;
		XMFLOAT3 toWard = {
			pkt->targetX - pkt->x,
			0.f, // Y�� ȸ���̹Ƿ� ���� ����
			pkt->targetZ - pkt->z
		};
		toWard = Vector3::Normalize(toWard);
		// [3] ȸ�� ���� ��� (Z ����)
		float angle = atan2f(toWard.x, toWard.z); // x/z
		float degrees = XMConvertToDegrees(angle);
		gGameFramework->ZmonstersToward[pkt->monsterID] = degrees + 180;
	}

}

// [������] HP�ٿ� ���� ü�� ����
void ClientNetwork::ProcessPlayerHP(char* buffer)
{
	// ���� �� �������� (GameScene���� ĳ���� �ʿ�)
	shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	GameScene* gameScene = dynamic_cast<GameScene*>(currentScene.get());

	SC_Packet_PlayerHP* pkt = reinterpret_cast<SC_Packet_PlayerHP*>(buffer);
	switch (gameScene->m_job)
	{
	case 0:
		gameScene->m_uiObjects[1]->m_fillAmount = (pkt->hp / 100.f);
		break;
	case 1:
		gameScene->m_uiObjects[1]->m_fillAmount = (pkt->hp / 300.f);
		break;
	case 2:
		gameScene->m_uiObjects[1]->m_fillAmount = (pkt->hp / 200.f);
		break;
	case 3:
		gameScene->m_uiObjects[1]->m_fillAmount = (pkt->hp / 1000.f);
		break;
	default:
		break;
	}
	// 0.0~2.0
}

// [������] ���� ��� �� �� ���� ����
void ClientNetwork::ProcessEndGame(char* buffer)
{
	SC_Packet_EndGame* pkt = reinterpret_cast<SC_Packet_EndGame*>(buffer);
	pkt->time;
	// Ŭ���� �ð� ��������, ���� 300�ʷ� ������ ����, 300�� ������ �������� ����
	// �� ��ȯ�������� �׳� �츮 ���â ó�� �������� �ϰ� Ŭ���� �ʸ� �־�θ� �ɰŰ���.
	// �׸��� 10�� �ڿ� �ڵ����� �� ���� â���� ���ư��� �� ��ȯ

	shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	GameScene* gameScene = dynamic_cast<GameScene*>(currentScene.get());

	gameScene->m_bossMonsters[0]->SetCurrentAnimation("Die");
	gameScene->SetEnding();
}

// [������] ���� ���� ����  -  ��Ŷ ���ڸ��� �ش� ���� ���� �ִϸ��̼� ����(������ �׳� �ٶ󺸴� ��)
void ClientNetwork::ProcessZMonsterAttack(char* buffer)
{
	SC_Packet_ZMonsterAttack* pkt = reinterpret_cast<SC_Packet_ZMonsterAttack*>(buffer);

	shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	GameScene* gameScene = dynamic_cast<GameScene*>(currentScene.get());

	if (pkt->monsterID != 25) {				// ���� �Ϲ� ���� (1�ʵ��� ��ų �ִϸ��̼�)

		gGameFramework->ZmonstersPlayAttack[pkt->monsterID] = true;
		string type;
		switch (pkt->monsterID / 5)
		{
		case 0:
			type = "Mushroom_Dark";
			break;
		case 1:
			type = "FrightFly";
			break;
		case 2:
			type = "Plant_Dionaea";
			break;
		case 3:
			type = "Venus_Blue";
			break;
		case 4:
			type = "Flower_Fairy";
			break;
		default:
			break;
		}
		auto monster = gameScene->m_BossStageMonsters[type][pkt->monsterID % 5];
		monster->PlayAnimationWithBlend("Attack", 0.2f);
		monster->m_AnimOnce = true;
	}
	else {									// ���� ���� ���� (2�ʵ���  �����ְ� 1�ʵ��� ���� ������ų �ִϸ��̼�)
		if (pkt->bossmonsterSkill == 2)									//����
		{
			gameScene->SpawnShockwaveWarning(gameScene->m_bossMonsters[0]->GetPosition());
			//gameScene->m_bossMonsters[0]->PlayAnimationWithBlend("Jump", 2.0f);
		}
		else if (pkt->bossmonsterSkill == 1)								//����
		{
			gameScene->SpawnDashWarning(gameScene->m_bossMonsters[0]->GetPosition(), gGameFramework->BossToward);
			//gameScene->m_bossMonsters[0]->PlayAnimationWithBlend("Dash", 2.0f);
		}
	}
}



// [���� ���� �Ϸ� -> Ŭ���ʿ��� ������ ���]
// �ؿ� ��Ŷ�̶�� �Ǿ� �ִ°� Ŭ�󿡼� ������ ��Ŷ�� ������ �Ǵ� �� ��������ž�.
// "��Ŷ : X " �� �̹� �� ó���Ǿ� �����ϱ� �������� ���� ��Ŷ������ ���� ���� ��°ž�. 
// ������ ��Ŷ �����ϴ� ����� ���̶� �����ϰ� ��������ȭ �� �س����ϱ� pkt.ġ�� �� �����ߴ��� �� ����.
// "��Ŷ : ~ " �� Ŭ�󿡼� ������ ��������ϴ°� �����ϱ� �ذ��ؾ��� �̰ž�~
// 
// 
// 1.  ���� ���� ������ ����				[O]	/    ��Ŷ : X					   /    ���� ��� : ��� �÷��̾ ������ ��ġ�� ���� ���Ͱ� ����
// ->  ProcessZenithMonster�Լ����� ���� ���� �ʱ� ��ǥ �޾ƿ��� ���̾�.
// 2.  ProcessZMonsterHP�Լ� ��Ŷ ó��		[X]	/	 ��Ŷ : X					   /	���� ��� : ���� ���� ü�� ����ȭ, �� ���̸� ��� �÷��̾ ���⿡ �� ���̴°� ����
// 3.  ProcessPlayerHP�Լ� ��Ŷ ó��		[X]	/	 ��Ŷ : X					   /    ���� ��� : HP�ٿ� ���� ü�� ����
// 4.  ProcessRespone�Լ� ��Ŷ ó��			[X]	/	 ��Ŷ : X					   /    ���� ��� : HP�� 0�� �Ǹ� ���� �������� �̵�
// 5.  �������ִ� ���� �Ծ�����				[X]	/    ��Ŷ : CS_Packet_HealPack     /    ���� ��� : HP ����
// 6.  �÷��̾� ��ų ����Ʈ					[X]	/    ��Ŷ : CS_Packet_AttackEffect /    ���� ��� : ������ � �÷��̾ �̷� ��ų �����ϱ� �� �ڸ��� ����Ʈ�� �־����� �ֹ����� 
// ->  skill(0 ~ 3)       // 0. ���� �⺻ ���� ����Ʈ 1. ���� ��ų ���� ����Ʈ 2. ������ �⺻ ���� ����Ʈ 3. ������ ��ų ���� ����Ʈ
// 7.  ProcessAttackEffect�Լ� ��Ŷ ó��	[X]	/	 ��Ŷ : X					   /	���� ��� : Ÿ �÷��̾��� ��ų �� �⺻ ���� ����Ʈ�� ����
// 8.  ProcessCMonsterTarrget�Լ� ��Ŷ ó��	[X]	/    ��Ŷ : X					   /	���� ��� : ���� ���Ͱ� ���� ����� �÷��̾ �Ĵٺ�
// 9.  ProcessEndGame�Լ� ��Ŷ ó��			[X]	/    ��Ŷ : X					   /	���� ��� : ���� ���͸� ����� �� Ȥ�� 300�ʰ� ������ ���� Ŭ���� ��Ȳ ������ 10�� �ڿ� �ڵ����� �� ����â���� ���ư�.
// 10. ������ �� �������� ��Ŷ �����ֱ�		[X]	/    ��Ŷ : CS_Packet_Customize	   /	���� ��� : ���� ���� ���� �������� Ŭ���ϸ� ������ ������.
// 99. ProcessCustomize�Լ� ��Ŷ ó��		[X] /    ��Ŷ : X					   /	���� ��� : ���� Ŀ���͸���¡�� �� ����.
//
// 
// �����ϴٰ� �������°� ���� �� �����س�����
// [������]�̶�� �Ǿ��ִ°� �����ٵ� �װ� �������� ���� ��Ŷ ó���� �ϸ� �Ǵ°ž�
// ���� ������ ȭ����!