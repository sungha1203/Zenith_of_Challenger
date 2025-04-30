#include "network.h"

ClientNetwork::ClientNetwork()
{
}

ClientNetwork::~ClientNetwork()
{
}

void ClientNetwork::Connect()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)	return;

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
	if (sent == SOCKET_ERROR) return false;
	return true;
}

void ClientNetwork::Receive()
{
	char buffer[1024] = { 0 };
	while (m_running) {
		int received = recv(m_clientsocket, buffer, sizeof(buffer), 0);
		if (received > 0) {
			switch (buffer[0]) {
			case SC_PACKET_LOGIN_RESPONSE:
				ProcessLogin(buffer);
				break;
			case SC_PACKET_ROOM_RESPONSE:
				ProcessRoomjoin(buffer);
				break;
			case SC_PACKET_WHOISMYTEAM:
				ProcessWhoismyteam(buffer);
				break;
			case SC_PACKET_GAMESTART:
				ProcessGamestart(buffer);
				break;
			case SC_PACKET_INITIALSTATE:
				ProcessInitialstate(buffer);
				break;
			case SC_PACKET_INITMONSTER:
				ProcessInitMonster(buffer);
				break;
			case SC_PACKET_UPDATE2PLAYER:
				ProcessUpdateCoord2Player(buffer);
				break;
			case SC_PACKET_REPAIRTIME:
				ProcessStartRepairTime(buffer);
				break;
			case SC_PACKET_MONSTERHP:
				ProcessMonsterHP(buffer);
				break;
			case SC_PACKET_DROPITEM:
				break;
			case SC_PACKET_INVENTORY:
				ProcessInventory(buffer);
				break;
			default:
				break;
			}
		}
		else if (received == 0) {

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
		if (pkt->teamID[i] != m_clientID)		// 내 클라면 저장안함
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
		//	player->SetPosition(pos);
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

void ClientNetwork::ProcessInitMonster(char* buffer)
{
	SC_Packet_InitMonster* pkt = reinterpret_cast<SC_Packet_InitMonster*>(buffer);
	XMFLOAT3 pos(pkt->x, pkt->y, pkt->z);
	gGameFramework->monstersCoord[pkt->monsterid] = pos;
}

void ClientNetwork::ProcessUpdateCoord2Player(char* buffer)
{
	SC_Packet_Update2Player* pkt = reinterpret_cast<SC_Packet_Update2Player*>(buffer);
	XMFLOAT3 pos(pkt->x, pkt->y, pkt->z);
	// @@@ = pkt->client_id;		// 받은 좌표의 아이디 
	// @@@ = pos;					// 그 아이디의 좌표 	
	if (pkt->client_id == m_clientID) {
		gGameFramework->g_pos = pos;

		//auto player = gGameFramework->GetPlayer();
		//if (player)
		//	player->SetPosition(pos);
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

void ClientNetwork::ProcessChat(char* buffer)
{
	SC_Packet_Chat* pkt = reinterpret_cast<SC_Packet_Chat*>(buffer);

}

void ClientNetwork::ProcessStartRepairTime(char* buffer)
{
	SC_Packet_RepairTime* pkt = reinterpret_cast<SC_Packet_RepairTime*>(buffer);
	XMFLOAT3 pos(pkt->x, pkt->y, pkt->z);
	if (pkt->startRT == true) {
		if (pkt->client_id == m_clientID) {
			gGameFramework->g_pos = pos;
			gGameFramework->GetSceneManager()->GetCurrentScene()->m_player->SetPosition(pos);

			//auto player = gGameFramework->GetPlayer();
			//if (player)
			//	player->SetPosition(pos);
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
		type = "Frightfly"; index = monsterID - 10;
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

}

void ClientNetwork::ProcessInventory(char* buffer)
{
	SC_Packet_Inventory* pkt = reinterpret_cast<SC_Packet_Inventory*>(buffer);
	pkt->gold;

	// 현재 씬 가져오기 (GameScene으로 캐스팅 필요)
	shared_ptr<Scene> currentScene = gGameFramework->GetSceneManager()->GetCurrentScene();
	GameScene* gameScene = dynamic_cast<GameScene*>(currentScene.get());

	if (gameScene)
	{
		gameScene->SetGoldScore(pkt->gold);
	}

}
