#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "network.h"
#include "session.h"
#include "RoomManager.h"

Network								g_network;
SOCKET								g_ListenSocket;
OVER_EXP							g_a_over;
RoomManager							g_room_manager;
std::unordered_map<int, ClientInfo> g_client;			// 인게임 정보

Network::Network() {
	m_client = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	h_iocp = INVALID_HANDLE_VALUE;
}

Network::~Network() {

}

//-------------------------------------------------[ 임   시 ]-------------------------------------------------
std::unordered_map<std::string, std::string> login_data;		// 로그인 가능 데이터 관리
std::unordered_map<std::string, int> logged_in_users;			// 로그인된 유저 관리

// [임시] 로그인 파일, 나중에 DB로 연동할 예정
void LoadLoginData() {
	std::ifstream file("login.txt");
	if (!file.is_open()) {
		std::cout << "[ERROR] 로그인 파일을 열 수 없습니다!" << std::endl;
		return;
	}

	std::string line, id, pw;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		if (!(iss >> id >> pw)) continue;
		login_data[id] = pw;
	}

	file.close();
	std::cout << "[INFO] 로그인 데이터 로딩 완료. 총 " << login_data.size() << " 개의 계정 존재함." << std::endl;
}

// 로그인 검증
bool CheckLogin(const std::string& id, const std::string& pw) {
	auto it = login_data.find(id);
	if (it != login_data.end() && it->second == pw) {
		return true;
	}
	return false;
}

// 클라 로그인 성공 여부
void Network::ProcessLogin(int client_id, char* buffer, int length) {
	std::istringstream iss(std::string(buffer + 5, length - 5));
	std::string id, pw;

	if (!(iss >> id >> pw)) {
		std::cout << "[ERROR] 클라이언트 [" << client_id << "] 로그인 패킷 오류!" << std::endl;
		SendLoginResponse(client_id, false);
		return;
	}

	//  이미 로그인된 아이디인지 확인
	if (logged_in_users.find(id) != logged_in_users.end()) {
		std::cout << "[ERROR] 클라이언트 [" << client_id << "] 로그인 실패 - 이미 로그인된 ID: " << id << std::endl;
		SendLoginResponse(client_id, false);
		return;
	}

	if (CheckLogin(id, pw)) {
		logged_in_users[id] = client_id;		// 로그인된 유저 목록에 추가
		g_network.clients[client_id].m_login_id = id;		// 세션에 로그인된 ID 저장

		std::cout << "[INFO] 클라이언트 [" << client_id << "] 로그인 성공 - ID: " << id << std::endl;
		SendLoginResponse(client_id, true);
	}
	else {
		std::cout << "[ERROR] 클라이언트 [" << client_id << "] 로그인 실패 - ID: " << id << std::endl;
		SendLoginResponse(client_id, false);
	}
}

// 클라에 로그인 성공 여부 응답
void Network::SendLoginResponse(int client_id, bool success) {
	SC_Packet_LoginResponse packet;
	packet.type = SC_PACKET_LOGIN_RESPONSE;
	packet.size = sizeof(packet);
	packet.clientID = client_id;
	packet.success = success;
	g_network.clients[client_id].do_send(packet);
}
//-------------------------------------------------[ 임   시 ]-------------------------------------------------



void Network::init() {
	// 윈속 초기화
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	// 임시 로그인 데이터 로드
	LoadLoginData();

	// 서버 소켓 생성
	g_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	// 서버 주소 설정
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;

	// 소켓 바인딩
	if (bind(g_ListenSocket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) == SOCKET_ERROR) {
		std::cout << "[ERROR] bind 실패 : " << WSAGetLastError() << std::endl;
		closesocket(g_ListenSocket);
		WSACleanup();
	};

	// 연결 대기
	if (listen(g_ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cout << "[ERROR] listen 실패 : " << WSAGetLastError() << std::endl;
		closesocket(g_ListenSocket);
		WSACleanup();
	};

	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);

	//CompletionPort객체 생성 요청을 한다.
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	if (h_iocp == nullptr) {
		std::cout << "[ERROR] CreateIoCompletionPort 실패 : " << GetLastError() << std::endl;
	}
	auto h_iocpHandle = CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_ListenSocket), h_iocp, 9999, 0);
	if (h_iocpHandle == nullptr) {
		std::cout << "[ERROR] ListenSocket IOCP bind 실패 : " << WSAGetLastError() << std::endl;
	}

	m_client = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	g_a_over._comp_type = OP_ACCEPT;
	AcceptEx(g_ListenSocket, m_client, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);

	auto num_core = std::thread::hardware_concurrency();

	for (int i = 0; i < num_core; ++i) {
		workers.emplace_back(&Network::WorkerThread, this);
	}

	for (std::thread& a : workers) {
		a.join();
	}

	closesocket(g_ListenSocket);
	WSACleanup();
}

int Network::CreateClient() {
	for (int i = 0; i < MAX_USER; ++i) {
		if (clients[i].m_used == false) {
			clients[i].m_id = i;
			g_client[i] = ClientInfo(i);
			return i;
		}
	}
	return -1;
}

void Network::CloseClient(int client_id) {
	if (client_id < 0 || client_id >= MAX_USER) return;

	std::string logout_id = g_network.clients[client_id].m_login_id;
	if (!logout_id.empty() && logged_in_users.find(logout_id) != logged_in_users.end()) {
		std::cout << "[INFO] 클라이언트 [" << client_id << "] 로그아웃 - ID: " << logout_id << std::endl;
		logged_in_users.erase(logout_id);				// 로그인 리스트에서 제거
		g_network.clients[client_id].m_login_id.clear();			// 세션의 ID 정보 삭제
		g_room_manager.LeaveRoom(client_id);
	}

	closesocket(g_network.clients[client_id].m_socket);			// 클라이언트 소켓 닫기
	g_network.clients[client_id].m_socket = INVALID_SOCKET;
	g_network.clients[client_id].m_used = false;					// 비활성화
	g_network.clients[client_id].m_id = -1;						// ID 초기화
}

void Network::WorkerThread() {
	DWORD num_bytes;
	ULONG_PTR key;
	WSAOVERLAPPED* over = nullptr;

	while (true) {
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);

		if (FALSE == ret) {
			int err = GetLastError();

			if (err == ERROR_NETNAME_DELETED || err == ERROR_CONNECTION_ABORTED || err == ERROR_BROKEN_PIPE) {
				std::cout << "[INFO] 클라이언트 [" << key << "] 비정상 종료 감지. 연결 해제." << std::endl;
				CloseClient(static_cast<int>(key));  // 강제 종료
			}
			else if (ex_over && ex_over->_comp_type == OP_ACCEPT) {
				std::cout << "[ERROR] Accept 실패" << std::endl;
			}
			else {
				std::cout << "[WARNING] GetQueuedCompletionStatus 오류 발생: " << err << std::endl;
			}

		}

		switch (ex_over->_comp_type) {
		case OP_ACCEPT:
		{
			int client_id = CreateClient();
			if (client_id != -1) {
				clients[client_id].m_used = true;
				clients[client_id].m_id = client_id;
				clients[client_id].m_socket = m_client;

				std::cout << "[INFO] 클라이언트 [" << client_id << "] 연결!!!" << std::endl;

				CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_client), h_iocp, client_id, 0);
				clients[client_id].do_recv();
				m_client = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else {
				std::cout << "[ERROR] 인원 초과!!" << std::endl;
				closesocket(m_client);
				m_client = INVALID_SOCKET;

				ZeroMemory(&g_a_over._over, sizeof(g_a_over._over));
				int addr_size = sizeof(SOCKADDR_IN);

				if (m_client == INVALID_SOCKET) {
					m_client = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				}
				AcceptEx(g_ListenSocket, m_client, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);
			}
			ZeroMemory(&g_a_over._over, sizeof(g_a_over._over));
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(g_ListenSocket, m_client, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);
			break;
		}
		case OP_RECV:
		{
			if (num_bytes == 0) {									// 클라이언트가 정상적으로 종료된 경우
				CloseClient(static_cast<int>(key));
			}
			else {
				clients[key].RecvComplete(num_bytes);
				//g_network.HandlePacket(key, clients[key].m_recv_over._send_buf, num_bytes);
			}
			break;
		}
		case OP_SEND:
		{
			delete ex_over;
			break;
		}
		}
	}
}

void Network::HandlePacket(int client_id, char* buffer, int length) {
	if (client_id < 0 || client_id >= MAX_USER) {
		std::cout << "[ERROR] 잘못된 클라이언트 ID : " << client_id << std::endl;
		return;
	}

	int packet_type = buffer[0]; // 첫 바이트가 패킷 타입

	switch (packet_type) {
	case CS_PACKET_LOGIN:
		ProcessLogin(client_id, buffer, length);
		break;
	case CS_PACKET_ROOM:
		ProcessRoomJoin(client_id, buffer, length);
		break;
	case CS_PACKET_CUSTOMIZE:
		ProcessCustomize(client_id, buffer, length);
		break;
	case CS_PACKET_GAMESTART:
		ProcessGameStartButton(client_id);
		break;
	case CS_PACKET_UPDATECOORD:
		ProcessUpdatePlayer(client_id, buffer, length);
		break;
	case CS_PACKET_INGAMEREADY:
		ProcessIngameReady(client_id, buffer, length);
		break;
	case CS_PACKET_STARTZENITH:
		ProcessZenithStartButton(client_id);
		break;
	case CS_PACKET_ZENITHREADY:
		ProcessZenithReady(client_id, buffer, length);
		break;
	case CS_PACKET_SKIPCHALLENGE:
		ProcessSkipChallenge(client_id, buffer, length);
		break;
	case CS_PACKET_MONSTERHP:
		ProcessMonsterHP(client_id, buffer, length);
		break;
	case CS_PACKET_INVENTORY:
		ProcessInventorySelcet(client_id, buffer, length);
		break;
	case CS_PACKET_LOGOUT:
		CloseClient(client_id);
		break;
	case CS_PACKET_DEBUGGOLD:
		ProcessDebugGold(client_id, buffer, length);
		break;
	case CS_PACKET_DEBUGITEM:
		ProcessDebugItem(client_id, buffer, length);
		break;
	case CS_PACKET_ITEMSTATE:
		ProcessItemState(client_id, buffer, length);
		break;
	case CS_PACKET_ANIMATION:
		ProcessAnimation(client_id, buffer, length);
		break;
	case CS_PACKET_DAMAGED:
		ProcessDamaged(client_id, buffer, length);
		break;
	case CS_PACKET_ATTACKEFFECT:
		ProcessAttackEffect(client_id, buffer, length);
		break;
	default:
		std::cout << "[ERROR] 알 수 없는 패킷 수신함. 클라이언트 [" << client_id << "]" << std::endl;
		break;
	}
}

//------------------------------------[Recv Packet]------------------------------------

// 방 입장 성공 여부
void Network::ProcessRoomJoin(int client_id, char* buffer, int length) {
	int room_id = buffer[2];  // 클라이언트가 입력한 방 번호

	if (g_room_manager.JoinRoom(client_id, room_id)) {
		SendRoomJoinResponse(client_id, true, room_id);
	}
	else {
		SendRoomJoinResponse(client_id, false, room_id);
	}
}

// 커스터 마이징
void Network::ProcessCustomize(int client_id, char* buffer, int length) {
	int room_id = g_room_manager.GetRoomID(client_id);
	Room& room = g_room_manager.GetRoom(room_id);
	const auto& client = room.GetClients();

	CS_Packet_Customize* pkt = reinterpret_cast<CS_Packet_Customize*>(buffer);
	g_client[client_id].SetClothes(pkt->clothes);

	SC_Packet_Customize pkt2;
	pkt2.type = SC_PACKET_CUSTOMIZE;
	pkt2.clientID = client_id;
	pkt2.clothes = pkt->clothes;

	for (int other_id : client) {
		g_network.clients[other_id].do_send(pkt2);
	}
}

// 인게임 내 플레이어들한테 업데이트
void Network::ProcessUpdatePlayer(int client_id, char* buffer, int length) {
	CS_Packet_UPDATECOORD* pkt = reinterpret_cast<CS_Packet_UPDATECOORD*>(buffer);
	g_client[client_id].SetCoord(pkt->x, pkt->y, pkt->z);
	g_client[client_id].SetAngle(pkt->angle);

	int room_id = g_room_manager.GetRoomID(client_id);
	if (room_id == -1) return;

	Room& room = g_room_manager.GetRoom(room_id);
	const auto& client = room.GetClients();

	// 다른 클라들에게 좌표 전파
	SC_Packet_Update2Player pkt2;
	pkt2.type = SC_PACKET_UPDATE2PLAYER;
	pkt2.client_id = client_id;
	pkt2.x = pkt->x;
	pkt2.y = pkt->y;
	pkt2.z = pkt->z;
	pkt2.angle = pkt->angle;
	pkt2.size = sizeof(SC_Packet_Update2Player);
	for (int other_id : client) {
		if (other_id == client_id) continue;
		g_network.clients[other_id].do_send(pkt2);
	}
}

// 게임 시작 버튼 누른 직후
void Network::ProcessGameStartButton(int client_id) {
	int room_id = g_room_manager.GetRoomID(client_id);
	if (room_id == -1) return;

	Room& room = g_room_manager.GetRoom(room_id);
	room.PushStartGameButton(client_id);
}

// 도전 스테이지 입장 성공 여부
void Network::ProcessIngameReady(int client_id, char* buffer, int length) {
	CS_Packet_GameReady* pkt = reinterpret_cast<CS_Packet_GameReady*>(buffer);
	if (pkt->ReadySuccess == true) {
		int room_id = g_room_manager.GetRoomID(client_id);
		if (room_id == -1) return;

		Room& room = g_room_manager.GetRoom(room_id);
		room.PlusPlayerReadyNum();
		if (room.GetPlayerNum() == room.GetEnterClientNum()) {
			room.StartGame();
		}
	}
}

// 도전 스테이지 스킵(정비 시간 입장)
void Network::ProcessSkipChallenge(int client_id, char* buffer, int length) {
	CS_Packet_SkipChallenge* pkt = reinterpret_cast<CS_Packet_SkipChallenge*>(buffer);
	if (pkt->skip == true) {
		int room_id = g_room_manager.GetRoomID(client_id);
		if (room_id == -1) return;

		Room& room = g_room_manager.GetRoom(room_id);
		if (room.GetSkipButton() == false) {
			room.SetSkipTimer(true);
			room.SetSkipButton(true);
		}
	}
}

// 정점 스테이지 입장 준비 완료 버튼 누른 직후
void Network::ProcessZenithStartButton(int client_id) {
	int room_id = g_room_manager.GetRoomID(client_id);
	if (room_id == -1) return;

	Room& room = g_room_manager.GetRoom(room_id);
	room.PushStartZenithButton(client_id);
}

// 정점 스테이지 입장 성공 여부
void Network::ProcessZenithReady(int client_id, char* buffer, int length) {
	CS_Packet_ZenithReady* pkt = reinterpret_cast<CS_Packet_ZenithReady*>(buffer);
	if (pkt->ReadySuccess == true) {
		int room_id = g_room_manager.GetRoomID(client_id);
		if (room_id == -1) return;

		Room& room = g_room_manager.GetRoom(room_id);
		room.PlusPlayerZenithReadyNum();
		if (room.GetPlayerNum() == room.GetEnterZenithNum()) {
			room.StartZenithStage();
		}
	}
}

// 인게임 속 채팅
void Network::ProcessChat(int client_id, char* buffer, int length) {
	CS_Packet_Chat* pkt = reinterpret_cast<CS_Packet_Chat*>(buffer);

	int room_id = g_room_manager.GetRoomID(client_id);
	Room& room = g_room_manager.GetRoom(room_id);
	const auto& client = room.GetClients();

	SC_Packet_Chat pkt2;
	pkt2.type = SC_PACKET_CHAT;
	strcpy(pkt2.msg, pkt->msg);
	pkt2.size = sizeof(pkt2);

	for (int other_id : client) {
		g_network.clients[other_id].do_send(pkt2);
	}
}

// 도전 몬스터 HP 업데이트
void Network::ProcessMonsterHP(int client_id, char* buffer, int length) {
	CS_Packet_MonsterHP* pkt = reinterpret_cast<CS_Packet_MonsterHP*>(buffer);

	int room_id = g_room_manager.GetRoomID(client_id);
	Room& room = g_room_manager.GetRoom(room_id);
	const auto& client = room.GetClients();

	room.GetCMonster(pkt->monsterID).TakeDamage(pkt->damage);

	SC_Packet_MonsterHP pkt2;
	pkt2.type = SC_PACKET_MONSTERHP;
	pkt2.monsterID = pkt->monsterID;
	pkt2.monsterHP = room.GetCMonsters(pkt->monsterID).GetHP();
	pkt2.size = sizeof(SC_Packet_MonsterHP);
	for (int other_id : client) {
		g_network.clients[other_id].do_send(pkt2);
	}

	// 도전 몬스터 잡으면 나오는 드랍 아이템
	if (room.GetCMonster(pkt->monsterID).GetHP() == 0) {
		int dropItem = static_cast<int>(room.GetCMonster(pkt->monsterID).DropWHAT());

		std::random_device rd;
		std::default_random_engine dre{ rd() };
		std::uniform_int_distribution<int> uid1{ 1, 10 };


		SC_Packet_DropItem pkt3;
		pkt3.type = SC_PACKET_DROPITEM;
		pkt3.item = dropItem - 1;
		pkt3.x = room.GetCMonster(pkt->monsterID).GetX();
		pkt3.y = room.GetCMonster(pkt->monsterID).GetY();
		pkt3.z = room.GetCMonster(pkt->monsterID).GetZ();
		pkt3.size = sizeof(pkt3);

		if (dropItem > 0 && dropItem <= 3) {		// 무기
			room.ADDJobWeapon(dropItem);
			pkt3.itemNum = room.GetWeaponTypeNum(dropItem);
		}
		else if (dropItem > 3 && dropItem <= 6) {	// 전직서
			room.AddJobDocument(dropItem);
			pkt3.itemNum = room.GetJobTypeNum(dropItem);
		}

		for (int other_id : client) {
			g_network.clients[other_id].do_send(pkt3);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		room.AddGold(uid1(dre));
	}

}

// 정점 몬스터 HP 업데이트
void Network::ProcessZMonsterHP(int client_id, char* buffer, int length)
{
	CS_Packet_ZMonsterHP* pkt = reinterpret_cast<CS_Packet_ZMonsterHP*>(buffer);

	int room_id = g_room_manager.GetRoomID(client_id);
	Room& room = g_room_manager.GetRoom(room_id);
	const auto& client = room.GetClients();

	room.GetZMonster(pkt->monsterID).TakeDamage(pkt->damage);

	SC_Packet_ZMonsterHP pkt2;
	pkt2.type = SC_PACKET_ZMONSTERHP;
	pkt2.monsterID = pkt->monsterID;
	pkt2.monsterHP = room.GetCMonsters(pkt->monsterID).GetHP();
	pkt2.size = sizeof(SC_Packet_ZMonsterHP);
	for (int other_id : client) {
		g_network.clients[other_id].do_send(pkt2);
	}

	if (pkt->monsterID = 25 && room.GetCMonsters(pkt->monsterID).GetHP() == 0) {
		room.SetClearBoss();
	}
}

// 인벤토리 무기 및 전직서 선택
void Network::ProcessInventorySelcet(int client_id, char* buffer, int length) {
	int room_id = g_room_manager.GetRoomID(client_id);
	Room& room = g_room_manager.GetRoom(room_id);
	const auto& client = room.GetClients();

	CS_Packet_Inventory* pkt = reinterpret_cast<CS_Packet_Inventory*>(buffer);
	SC_Packet_Inventory pkt2;		// 모든 클라의 인벤토리에서 해당 아이템을 하나 삭제하기 위한 패킷
	pkt2.type = SC_PACKET_INVENTORY;
	pkt2.size = sizeof(SC_Packet_Inventory);
	SC_Packet_SelectItem pkt3;		// 해당 클라의 장비창에 해당 아이템을 장착하기 위한 패킷
	pkt3.type = SC_PACKET_SELECTITEM;
	pkt3.item = pkt->item;
	pkt3.clientID = client_id;
	pkt3.size = sizeof(SC_Packet_SelectItem);
	// 무기 선택했고, 선택한 무기가 없을 때(주먹), 선택한 무기가 1개이상일 때
	if (pkt->item > 0 && pkt->item <= 3
		&& static_cast<int>(g_client[client_id].GetWeaponType()) == 0
		&& room.GetWeaponTypeNum(pkt->item) > 0) {
		g_client[client_id].SetWeapon(pkt->item);
		room.DecideJobWeapon(pkt->item);				// 방(인벤토리)에 있는 해당 무기 하나 지움
		pkt2.item = pkt->item;
		pkt2.num = room.GetWeaponTypeNum(pkt->item);
		for (int other_id : client) {
			g_network.clients[other_id].do_send(pkt2);
			g_network.clients[other_id].do_send(pkt3);
		}

	}
	// 전직서 선택했고, 선택한 직업이 없을 때(도전자), 선택한 전직서가 1개 이상일때
	else if (pkt->item > 3 && pkt->item <= 6
		&& static_cast<int>(g_client[client_id].GetJobType()) == 0
		&& room.GetJobTypeNum(pkt->item) > 0)			// 방(인벤토리)에 있는 해당 전직서 하나 지움
	{
		g_client[client_id].SetJobType(pkt->item);
		g_client[client_id].LeverUpPlayer((int)g_client[client_id].GetJobType());
		room.DecideJobDocument(pkt->item);
		pkt2.item = pkt->item;
		pkt2.num = room.GetJobTypeNum(pkt->item);
		for (int other_id : client) {
			g_network.clients[other_id].do_send(pkt2);
			g_network.clients[other_id].do_send(pkt3);
		}
	}
	else
		return;
}

// 무기 강화(장비창)
void Network::ProcessItemState(int client_id, char* buffer, int length) {
	CS_Packet_ItemState* pkt = reinterpret_cast<CS_Packet_ItemState*>(buffer);

	int room_id = g_room_manager.GetRoomID(client_id);
	Room& room = g_room_manager.GetRoom(room_id);
	//const auto& client = room.GetClients();

	// 1. 강화 시도 버튼을 눌렀을 때 2. 게임 골드가 0원 이상일 때  3. 강화 무기 등급이 9등급 아래일 때
	if (pkt->enhanceTry == true && room.GetGold() > 0 && g_client[client_id].GetWeaponGrade() < 9) {
		room.SpendGold(g_client[client_id].GetWeaponGrade() + 1);		// 무기 강화 비용 지불
		g_client[client_id].SetWeaponGrade();							// 무기 강화 시도
	}

	SC_Packet_ItemState pkt2;
	pkt2.type = SC_PACKET_ITEMSTATE;
	pkt2.result = g_client[client_id].GetWeaponGrade();
	pkt2.size = sizeof(pkt2);

	g_network.clients[client_id].do_send(pkt2);


}

// 디버깅용 골드 추가
void Network::ProcessDebugGold(int client_id, char* buffer, int length) {
	CS_Packet_DebugGold* pkt = reinterpret_cast<CS_Packet_DebugGold*>(buffer);

	int room_id = g_room_manager.GetRoomID(client_id);
	Room& room = g_room_manager.GetRoom(room_id);

	if (pkt->plusGold = true) {
		room.AddGold(10);
	}

}

// 디버깅용 무기 및 전직서 추가
void Network::ProcessDebugItem(int client_id, char* buffer, int length) {
	CS_Packet_DebugItem* pkt = reinterpret_cast<CS_Packet_DebugItem*>(buffer);

	int room_id = g_room_manager.GetRoomID(client_id);
	Room& room = g_room_manager.GetRoom(room_id);
	const auto& client = room.GetClients();

	int itemNum = pkt->item;

	SC_Packet_DebugItem pkt2;
	pkt2.type = SC_PACKET_DEBUGITEM;
	pkt2.item = pkt->item - 1;
	pkt2.size = sizeof(pkt2);

	if (itemNum > 0 && itemNum <= 3) {		// 무기
		room.ADDJobWeapon(itemNum);
		pkt2.itemNum = room.GetWeaponTypeNum(itemNum);
	}
	else if (itemNum > 3 && itemNum <= 6) {	// 전직서
		room.AddJobDocument(itemNum);
		pkt2.itemNum = room.GetJobTypeNum(itemNum);
	}

	for (int other_id : client) {
		g_network.clients[other_id].do_send(pkt2);
	}

}

// 애니메이션
void Network::ProcessAnimation(int client_id, char* buffer, int length) {
	CS_Packet_Animaition* pkt = reinterpret_cast<CS_Packet_Animaition*>(buffer);

	int room_id = g_room_manager.GetRoomID(client_id);
	Room& room = g_room_manager.GetRoom(room_id);
	const auto& client = room.GetClients();

	int job = (int)g_client[client_id].GetJobType();  // 0 : 도전자,  1 : 전사, 2 : 마법사, 3 : 힐탱커

	SC_Packet_Animaition pkt2;
	pkt2.type = SC_PACKET_ANIMATION;
	pkt2.client_id = client_id;

	// 0 : 기본      1 : 걷기      2 : 달리기      3 : 도전자 기본 공격      4 : 직업스킬      5 : 직업 기본 공격
	if (pkt->animation == 4) {
		if (!g_client[client_id].CanUseSkill()) return;		// 스킬 쿨타임이면 무시

		switch (job) {
		case 0: pkt2.animation = 3; break;		// 도전자 스킬(기본공격)
		case 1: pkt2.animation = 4; break;		// 전  사 스킬
		case 2: pkt2.animation = 5; break;		// 마법사 스킬
		case 3: pkt2.animation = 6; break;		// 힐탱커 스킬
		}
		g_client[client_id].StartCoolTime();	// 마지막 스킬 사용 시점
	}
	else if (pkt->animation == 5) {
		switch (job) {
		case 0: pkt2.animation = 3; break;		// 도전자 기본공격
		case 1: pkt2.animation = 7; break;		// 전  사 기본공격
		case 2: pkt2.animation = 8; break;		// 마법사 기본공격
		case 3: return;							// 힐탱커 기본공격 X
		}
	}
	else {
		pkt2.animation = pkt->animation;
	}
	pkt2.size = sizeof(pkt2);

	for (int other_id : client) {
		g_network.clients[other_id].do_send(pkt2);
	}
}

// 스킬, 기본 공격 이펙트(전사, 마법사)
void Network::ProcessAttackEffect(int client_id, char* buffer, int length)
{
	CS_Packet_AttackEffect* pkt = reinterpret_cast<CS_Packet_AttackEffect*>(buffer);

	int room_id = g_room_manager.GetRoomID(client_id);
	Room& room = g_room_manager.GetRoom(room_id);
	const auto& client = room.GetClients();

	// 자신을 제외한 타 클라한테 투사체 혹은 스킬 이펙트 보여줌
	SC_Packet_AttackEffect pkt2;
	pkt2.type = SC_PACKET_ATTACKEFFECT;
	pkt2.size = sizeof(pkt2);
	pkt2.targetID = client_id;
	pkt2.skill = pkt->skill;
	pkt2.angle = g_client[client_id].GetAngle();

	for (int other_id : client) {
		g_network.clients[other_id].do_send(pkt2);
	}
}

// 힐팩 먹기
void Network::ProcessEatHealPack(int client_id, char* buffer, int length)
{
	CS_Packet_HealPack* pkt = reinterpret_cast<CS_Packet_HealPack*>(buffer);
	if (pkt->eat == true) {
		g_client[client_id].AddHP(10);
	}
}

// 플레이어 데미지
void Network::ProcessDamaged(int client_id, char* buffer, int length)
{
	CS_Packet_Damaged* pkt = reinterpret_cast<CS_Packet_Damaged*>(buffer);

	int room_id = g_room_manager.GetRoomID(client_id);
	Room& room = g_room_manager.GetRoom(room_id);
	const auto& client = room.GetClients();

	if (room.GetMode() == 1) {		// 도전 스테이지
		int result = g_client[client_id].MinusHP(room.GetCMonsters(pkt->monsterID).GetAttack(), 1);
		if (result == 1) {
			SC_Packet_Respone pkt2;
			pkt2.type = SC_PACKET_RESPONE;
			pkt2.size = sizeof(pkt2);
			pkt2.clientID = client_id;
			pkt2.x = g_client[client_id].GetX();
			pkt2.y = g_client[client_id].GetY();
			pkt2.z = g_client[client_id].GetZ();

			for (int other_id : client) {				
				g_network.clients[other_id].do_send(pkt2);
			}
		}

	}
	else if (room.GetMode() == 3) {	// 정점 스테이지
		int result = g_client[client_id].MinusHP(room.GetZMonsters(pkt->monsterID).GetAttack(), 3);
		if (result == 1) {
			SC_Packet_Respone pkt3;
			pkt3.type = SC_PACKET_RESPONE;
			pkt3.size = sizeof(pkt3);
			pkt3.clientID = client_id;
			pkt3.x = g_client[client_id].GetX();
			pkt3.y = g_client[client_id].GetY();
			pkt3.z = g_client[client_id].GetZ();

			for (int other_id : client) {
				if (other_id == client_id) continue;
				g_network.clients[other_id].do_send(pkt3);
			}
		}
	}
}

//------------------------------------[Send Packet]------------------------------------

// 방 입장 성공 여부 응답
void Network::SendRoomJoinResponse(int client_id, bool success, int room_id) {
	SC_Packet_RoomResponse packet;
	packet.type = SC_PACKET_ROOM_RESPONSE;
	packet.size = sizeof(packet);
	packet.success = success;
	packet.room_id = room_id;
	g_network.clients[client_id].do_send(packet);
}

// 변경된 게임방 인원수 상황 보내주기 (수정할예정)
void Network::SendRoomInfo(int room_id, int client_num) {
	SC_Packet_RoomList packet;
	packet.type = SC_PACKET_ROOMLIST;
	packet.size = sizeof(packet);
	packet.room_id = room_id;
	packet.current_players = client_num;

	for (int i = 0; i < MAX_USER; ++i) {
		if (g_network.clients[i].m_used) {
			g_network.clients[i].do_send(packet);
		}
	}
}

// 게임 시작 버튼 누른 직후
void Network::SendGameStart(const std::vector<int>& client_id) {
	SC_Packet_GameStart packet;
	packet.type = SC_PACKET_GAMESTART;
	packet.size = sizeof(packet);
	packet.startCS = true;

	for (int id : client_id) {
		if (g_network.clients[id].m_used) {
			g_network.clients[id].do_send(packet);
			std::cout << "[INFO] 클라이언트 [" << id << "] 게임 시작 패킷 전송" << std::endl;
		}
	}
}

// 아이디 값 보내주기
void Network::SendWhoIsMyTeam(const std::vector<int>& client_id) {
	SC_Packet_MyTeam packet;
	packet.type = SC_PACKET_WHOISMYTEAM;
	packet.size = sizeof(packet);
	packet.teamSize = client_id.size();
	int teamcnt = static_cast<int>(client_id.size());
	for (int i = 0; i < teamcnt; ++i)
		packet.teamID[i] = client_id[i];

	for (int id : client_id) {
		if (g_network.clients[id].m_used)
			g_network.clients[id].do_send(packet);
	}
}

// 도전 스테이지 스폰 시 위치 설정
void Network::SendInitialState(const std::vector<int>& client_id) {
	for (int target_id : client_id) {
		if (!g_network.clients[target_id].m_used) continue;

		for (int other_id : client_id) {
			if (!g_network.clients[other_id].m_used) continue;
			SC_Packet_initialstate packet;
			packet.type = SC_PACKET_INITIALSTATE;
			packet.client_id = target_id;
			packet.x = g_client[target_id].GetX();
			packet.y = g_client[target_id].GetY();
			packet.z = g_client[target_id].GetZ();
			packet.size = sizeof(packet);

			g_network.clients[other_id].do_send(packet);
		}
	}
}

// 정점 스테이지 스폰 시 위치 설정
void Network::SendZenithState(const std::vector<int>& client_id)
{
	for (int target_id : client_id) {
		if (!g_network.clients[target_id].m_used) continue;

		for (int other_id : client_id) {
			if (!g_network.clients[other_id].m_used) continue;
			SC_Packet_Zenithstate packet;
			packet.type = SC_PACKET_ZENITHSTATE;
			packet.client_id = target_id;
			packet.x = g_client[target_id].GetX();
			packet.y = g_client[target_id].GetY();
			packet.z = g_client[target_id].GetZ();
			packet.size = sizeof(packet);

			g_network.clients[other_id].do_send(packet);
		}
	}
}

// 정비 시간 시작
void Network::SendStartRepairTime(const std::vector<int>& client_id) {
	for (int target_id : client_id) {
		if (!g_network.clients[target_id].m_used) continue;

		for (int other_id : client_id) {
			if (!g_network.clients[other_id].m_used) continue;
			SC_Packet_RepairTime packet;
			packet.type = SC_PACKET_REPAIRTIME;
			packet.startRT = true;
			packet.client_id = target_id;
			packet.x = g_client[target_id].GetX();
			packet.y = g_client[target_id].GetY();
			packet.z = g_client[target_id].GetZ();
			packet.size = sizeof(packet);

			g_network.clients[other_id].do_send(packet);

		}
	}
}

// 정점 스테이지 이동 대기
void Network::SendStartZenithStage(const std::vector<int>& client_id) {
	SC_Packet_ZenithStage packet;
	packet.type = SC_PACKET_ZENITHSTAGE;
	packet.size = sizeof(packet);
	packet.startZS = true;

	for (int id : client_id) {
		if (g_network.clients[id].m_used)
			g_network.clients[id].do_send(packet);
	}
}

// 골드 업데이트
void Network::SendUpdateGold(const std::vector<int>& client_id) {
	int room_id = g_room_manager.GetRoomID(client_id[0]);
	Room& room = g_room_manager.GetRoom(room_id);

	SC_Packet_Gold pkt;
	pkt.type = SC_PACKET_GOLD;
	pkt.gold = room.GetGold();
	pkt.size = sizeof(pkt);

	for (int id : client_id) {
		if (g_network.clients[id].m_used)
			g_network.clients[id].do_send(pkt);
	}
}

// 도전 몬스터 좌표 초기 설정
void Network::SendInitMonster(const std::vector<int>& client_id, const std::array<Monster, 50>& monsters) {
	SC_Packet_InitMonster packet;
	packet.type = SC_PACKET_INITMONSTER;
	packet.size = sizeof(packet);
	for (int monster_id = 0; monster_id < monsters.size(); ++monster_id) {
		const Monster& monster = monsters[monster_id];
		packet.monsters[monster_id].monsterid = monster_id;
		packet.monsters[monster_id].monstertype = static_cast<int>(monster.GetType());
		packet.monsters[monster_id].x = monster.GetX();
		packet.monsters[monster_id].y = monster.GetY();
		packet.monsters[monster_id].z = monster.GetZ();

	}
	for (int id : client_id) {
		if (g_network.clients[id].m_used) {
			g_network.clients[id].do_send(packet);
		}
	}
}

// 정점 몬스터 좌표 초기 설정
void Network::SendZenithMonster(const std::vector<int>& client_id, const std::array<Monster, 26>& monsters)
{
	SC_Packet_ZenithMonster packet;
	packet.type = SC_PACKET_ZENITHMONSTER;
	packet.size = sizeof(packet);
	for (int monster_id = 0; monster_id < monsters.size(); ++monster_id) {
		const Monster& monster = monsters[monster_id];
		packet.monsters[monster_id].monsterid = monster_id;
		packet.monsters[monster_id].monstertype = static_cast<int>(monster.GetType());
		packet.monsters[monster_id].x = monster.GetX();
		packet.monsters[monster_id].y = monster.GetY();
		packet.monsters[monster_id].z = monster.GetZ();

	}
	for (int id : client_id) {
		if (g_network.clients[id].m_used) {
			g_network.clients[id].do_send(packet);
		}
	}
}

// 플레이어 체력 업데이트
void Network::SendPlayerHP(int client_id)
{
	SC_Packet_PlayerHP pkt;
	pkt.type = SC_PACKET_PLAYERHP;
	pkt.hp = g_client[client_id].GetHP();
	pkt.size = sizeof(pkt);

	g_network.clients[client_id].do_send(pkt);
}

// 몬스터 공격
void Network::SendZMonsterAttack(const std::vector<int>& client_id, int MonsterID, int SkillType)
{
	SC_Packet_ZMonsterAttack pkt;
	pkt.type = SC_PACKET_ZMONSTERATTACK;
	pkt.monsterID = MonsterID;
	pkt.bossmonsterSkill = SkillType;
	pkt.size = sizeof(pkt);

	for (int id : client_id)
		if (g_network.clients[id].m_used)
			g_network.clients[id].do_send(pkt);
}

// 결과 보고 및 게임 종료
void Network::SendEndGame(const std::vector<int>& client_id, int time)
{
	SC_Packet_EndGame pkt;
	pkt.size = sizeof(pkt);
	pkt.time = time;

	for (int id : client_id) {
		if (g_network.clients[id].m_used) {
			g_network.clients[id].do_send(pkt);
		}
	}
}