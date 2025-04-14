#include "stdafx.h"
#include "network.h"
#include "session.h"
#include "RoomManager.h"

Network								g_network;
SOCKET								g_ListenSocket;
OVER_EXP							g_a_over;
RoomManager							g_room_manager;
std::unordered_map<int, ClientInfo> g_client;			// 인게임 정보


Network::Network()
{
	m_client = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	h_iocp = INVALID_HANDLE_VALUE;
}

Network::~Network()
{

}

//-------------------------------------------------[ 임   시 ]-------------------------------------------------
std::unordered_map<std::string, std::string> login_data;		// 로그인 가능 데이터 관리
std::unordered_map<std::string, int> logged_in_users;			// 로그인된 유저 관리

// [임시] 로그인 파일, 나중에 DB로 연동할 예정
void LoadLoginData()
{
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
bool CheckLogin(const std::string& id, const std::string& pw)
{
	auto it = login_data.find(id);
	if (it != login_data.end() && it->second == pw) {
		return true;
	}
	return false;
}

// 클라 로그인 성공 여부
void Network::ProcessLogin(int client_id, char* buffer, int length)
{
	std::istringstream iss(std::string(buffer + 1, length - 1));
	std::string id, pw;

	if (!(iss >> id >> pw)) {
		std::cout << "[ERROR] 클라이언트 [" << client_id << "] 로그인 패킷 오류!" << std::endl;
		SendLoginResponse(client_id, false);
		clients[client_id].do_recv();
		return;
	}

	//  이미 로그인된 아이디인지 확인
	if (logged_in_users.find(id) != logged_in_users.end()) {
		std::cout << "[ERROR] 클라이언트 [" << client_id << "] 로그인 실패 - 이미 로그인된 ID: " << id << std::endl;
		SendLoginResponse(client_id, false);
		clients[client_id].do_recv();			// 로그인 실패 후에도 다시 do_recv 실행
		return;
	}

	if (CheckLogin(id, pw)) {
		logged_in_users[id] = client_id;		// 로그인된 유저 목록에 추가
		clients[client_id].m_login_id = id;		// 세션에 로그인된 ID 저장

		std::cout << "[INFO] 클라이언트 [" << client_id << "] 로그인 성공 - ID: " << id << std::endl;
		SendLoginResponse(client_id, true);
		clients[client_id].do_recv();			// 로그인 성공 후에도 do_recv 실행
	}
	else {
		std::cout << "[ERROR] 클라이언트 [" << client_id << "] 로그인 실패 - ID: " << id << std::endl;
		SendLoginResponse(client_id, false);
		clients[client_id].do_recv();			// 로그인 실패 후에도 다시 do_recv() 실행
	}
}

// 클라에 로그인 성공 여부 응답
void Network::SendLoginResponse(int client_id, bool success)
{
	SC_Packet_LoginResponse packet;
	packet.type = SC_PACKET_LOGIN_RESPONSE;
	packet.size = sizeof(packet);
	packet.success = success;
	clients[client_id].do_send(packet);
}
//-------------------------------------------------[ 임   시 ]-------------------------------------------------



void Network::init()
{
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

	for (int i = 0; i < WORKER_THREAD_COUNT; ++i) {
		workers.emplace_back(&Network::WorkerThread, this);
	}

	for (std::thread& a : workers) {
		a.join();
	}

	closesocket(g_ListenSocket);
	WSACleanup();
}

int Network::CreateClient()
{
	for (int i = 0; i < MAX_USER; ++i) {
		if (clients[i].m_used == false) {
			clients[i].m_id = i;
			g_client[i] = ClientInfo(i, -1);
			return i;
		}
	}
	return -1;
}

void Network::CloseClient(int client_id)
{
	if (client_id < 0 || client_id >= MAX_USER) return;

	std::string logout_id = clients[client_id].m_login_id;
	if (!logout_id.empty() && logged_in_users.find(logout_id) != logged_in_users.end()) {
		std::cout << "[INFO] 클라이언트 [" << client_id << "] 로그아웃 - ID: " << logout_id << std::endl;
		logged_in_users.erase(logout_id);				// 로그인 리스트에서 제거
		clients[client_id].m_login_id.clear();			// 세션의 ID 정보 삭제
		g_room_manager.LeaveRoom(client_id);
	}

	closesocket(clients[client_id].m_socket);			// 클라이언트 소켓 닫기
	clients[client_id].m_socket = INVALID_SOCKET;
	clients[client_id].m_used = false;					// 비활성화
	clients[client_id].m_id = -1;						// ID 초기화
}

void Network::WorkerThread()
{
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
		case OP_RECV: {
			if (num_bytes == 0) {									// 클라이언트가 정상적으로 종료된 경우
				CloseClient(static_cast<int>(key));
			}
			else {
				g_network.HandlePacket(key, clients[key].m_recv_over._send_buf, num_bytes);
			}
			break;
		}
		case OP_SEND: {
			delete ex_over;
			break;
		}
		}
	}
}

void Network::HandlePacket(int client_id, char* buffer, int length)
{
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
	case CS_PACKET_UPDATEPLAYER:
		ProcessUpdatePlayer(client_id, buffer, length);
		break;
	case CS_PACKET_INGAMEREADY:
		ProcessIngameReady(client_id, buffer, length);
		break;
	case CS_PACKET_STARTZENITH:
		ProcessZenithReady(client_id, buffer, length);
		break;
	case CS_PACKET_LOGOUT:
		CloseClient(client_id);
		break;
	default:
		std::cout << "[ERROR] 알 수 없는 패킷 수신함. 클라이언트 [" << client_id << "]" << std::endl;
		break;
	}
}

// 방 입장 성공 여부
void Network::ProcessRoomJoin(int client_id, char* buffer, int length)
{
	int room_id = buffer[2];  // 클라이언트가 입력한 방 번호

	if (g_room_manager.JoinRoom(client_id, room_id)) {
		SendRoomJoinResponse(client_id, true, room_id);
		SendClientInformation(client_id);
		clients[client_id].do_recv();
	}
	else {
		SendRoomJoinResponse(client_id, false, room_id);
		clients[client_id].do_recv();
	}
}

// 커스터 마이징
void Network::ProcessCustomize(int client_id, char* buffer, int length)
{
	CS_Packet_Customize* pkt = reinterpret_cast<CS_Packet_Customize*>(buffer);
	g_client[client_id].SetClothes(pkt->clothes);
}

// 인게임 내 플레이어들한테 업데이트
void Network::ProcessUpdatePlayer(int client_id, char* buffer, int length)
{
	CS_Packet_UpdatePlayer* pkt = reinterpret_cast<CS_Packet_UpdatePlayer*>(buffer);
	g_client[client_id].SetCoord(pkt->x, pkt->y, pkt->z);

	int room_id = g_room_manager.GetRoomID(client_id);
	if (room_id == -1) return;

	Room& room = g_room_manager.GetRoom(room_id);
	const auto& client = room.GetClients();

	// 다른 클라들에게 좌표 전파
	SC_Packet_Update2Player pkt2;
	pkt2.client_id = client_id;
	pkt2.x = pkt->x;
	pkt2.y = pkt->y;
	pkt2.z = pkt->z;
	pkt2.dir = pkt->dir;
	pkt2.animation = pkt->animation;

	for (int other_id : client) {
		if (other_id == client_id) continue;
		clients[other_id].do_send(pkt2);
	}
}

// 게임 시작 버튼 누른 직후
void Network::ProcessGameStartButton(int client_id)
{
	int room_id = g_room_manager.GetRoomID(client_id);
	if (room_id == -1) return;

	Room& room = g_room_manager.GetRoom(room_id);
	room.PushStartGameButton(client_id);
}

// 도전 스테이지 입장 성공 여부
void Network::ProcessIngameReady(int client_id, char* buffer, int length)
{
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

// 정점 스테이지 입장 준비 완료 버튼 누른 직후
void Network::ProcessZenithStartButton(int client_id)
{
	int room_id = g_room_manager.GetRoomID(client_id);
	if (room_id == -1) return;

	Room& room = g_room_manager.GetRoom(room_id);
	room.PushStartZenithButton(client_id);
}

// 정점 스테이지 입장 성공 여부
void Network::ProcessZenithReady(int client_id, char* buffer, int length)
{
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

//------------------------------------[Send Packet]------------------------------------
 
// 방 입장 성공 여부 응답
void Network::SendRoomJoinResponse(int client_id, int success, int room_id)
{
	SC_Packet_RoomResponse packet;
	packet.type = SC_PACKET_ROOM_RESPONSE;
	packet.size = sizeof(packet);
	packet.success = success;
	packet.room_id = room_id;
	clients[client_id].do_send(packet);
}

// 변경된 게임방 인원수 상황 보내주기 (수정할예정)
void Network::SendRoomInfo(int room_id, int client_num)
{
	SC_Packet_RoomList packet;
	packet.type = SC_PACKET_ROOMLIST;
	packet.size = sizeof(packet);
	packet.room_id = room_id;
	packet.current_players = client_num;

	for (int i = 0; i < MAX_USER; ++i)
	{
		if (clients[i].m_used) {
			clients[i].do_send(packet);
		}
	}
}

// 게임 시작 버튼 누른 직후
void Network::SendGameStart(const std::vector<int>& client_id)
{
	SC_Packet_GameStart packet;
	packet.type = SC_PACKET_GAMESTART;
	packet.size = sizeof(packet);
	packet.startCS = true;

	for (int id : client_id) {
		if(clients[id].m_used)
			clients[id].do_send(packet);
	}
}

// 정비 시간 시작
void Network::SendStartRepairTime(const std::vector<int>& client_id)
{
	SC_Packet_RepairTime packet;
	packet.type = SC_PACKET_REPAIRTIME;
	packet.size = sizeof(packet);
	packet.startRT = true;

	for (int id : client_id) {
		if (clients[id].m_used)
			clients[id].do_send(packet);
	}
}

// 정점 스테이지 이동 대기
void Network::SendStartZenithStage(const std::vector<int>& client_id)
{
	SC_Packet_ZenithStage packet;
	packet.type = SC_PACKET_ZENITHSTAGE;
	packet.size = sizeof(packet);
	packet.startZS = true;

	for (int id : client_id) {
		if (clients[id].m_used)
			clients[id].do_send(packet);
	}
}

// 인벤토리 업데이트 (여기서부터 개발 이어서..)
void Network::SendUpdateInventory(const std::vector<int>& client_id)
{
	//
}

// [임시] 캐릭터 정보 전달
void Network::SendClientInformation(int client_id)
{
	SC_Packet_ClientInformation packet;
	packet.type = SC_PACKET_CLIENTINFORMATION;
	packet.size = sizeof(packet);
	packet.hp = g_client[client_id].GetHP();
	packet.attack = g_client[client_id].GetAttack();
	packet.speed = g_client[client_id].GetSpeed();
	packet.attackspeed = g_client[client_id].GetAttackSpeed();
	clients[client_id].do_send(packet);
}
