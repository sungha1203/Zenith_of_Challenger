#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "network.h"
#include "session.h"
#include "RoomManager.h"

Network								g_network;
SOCKET								g_ListenSocket;
OVER_EXP							g_a_over;
RoomManager							g_room_manager;
std::unordered_map<int, ClientInfo> g_client;			// �ΰ��� ����

Network::Network() {
	m_client = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	h_iocp = INVALID_HANDLE_VALUE;
}

Network::~Network() {

}

//-------------------------------------------------[ ��   �� ]-------------------------------------------------
std::unordered_map<std::string, std::string> login_data;		// �α��� ���� ������ ����
std::unordered_map<std::string, int> logged_in_users;			// �α��ε� ���� ����

// [�ӽ�] �α��� ����, ���߿� DB�� ������ ����
void LoadLoginData() {
	std::ifstream file("login.txt");
	if (!file.is_open()) {
		std::cout << "[ERROR] �α��� ������ �� �� �����ϴ�!" << std::endl;
		return;
	}

	std::string line, id, pw;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		if (!(iss >> id >> pw)) continue;
		login_data[id] = pw;
	}

	file.close();
	std::cout << "[INFO] �α��� ������ �ε� �Ϸ�. �� " << login_data.size() << " ���� ���� ������." << std::endl;
}

// �α��� ����
bool CheckLogin(const std::string& id, const std::string& pw) {
	auto it = login_data.find(id);
	if (it != login_data.end() && it->second == pw) {
		return true;
	}
	return false;
}

// Ŭ�� �α��� ���� ����
void Network::ProcessLogin(int client_id, char* buffer, int length) {
	std::istringstream iss(std::string(buffer + 5, length - 5));
	std::string id, pw;

	if (!(iss >> id >> pw)) {
		std::cout << "[ERROR] Ŭ���̾�Ʈ [" << client_id << "] �α��� ��Ŷ ����!" << std::endl;
		SendLoginResponse(client_id, false);
		return;
	}

	//  �̹� �α��ε� ���̵����� Ȯ��
	if (logged_in_users.find(id) != logged_in_users.end()) {
		std::cout << "[ERROR] Ŭ���̾�Ʈ [" << client_id << "] �α��� ���� - �̹� �α��ε� ID: " << id << std::endl;
		SendLoginResponse(client_id, false);
		return;
	}

	if (CheckLogin(id, pw)) {
		logged_in_users[id] = client_id;		// �α��ε� ���� ��Ͽ� �߰�
		g_network.clients[client_id].m_login_id = id;		// ���ǿ� �α��ε� ID ����

		std::cout << "[INFO] Ŭ���̾�Ʈ [" << client_id << "] �α��� ���� - ID: " << id << std::endl;
		SendLoginResponse(client_id, true);
	}
	else {
		std::cout << "[ERROR] Ŭ���̾�Ʈ [" << client_id << "] �α��� ���� - ID: " << id << std::endl;
		SendLoginResponse(client_id, false);
	}
}

// Ŭ�� �α��� ���� ���� ����
void Network::SendLoginResponse(int client_id, bool success) {
	SC_Packet_LoginResponse packet;
	packet.type = SC_PACKET_LOGIN_RESPONSE;
	packet.size = sizeof(packet);
	packet.clientID = client_id;
	packet.success = success;
	g_network.clients[client_id].do_send(packet);
}
//-------------------------------------------------[ ��   �� ]-------------------------------------------------



void Network::init() {
	// ���� �ʱ�ȭ
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	// �ӽ� �α��� ������ �ε�
	LoadLoginData();

	// ���� ���� ����
	g_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	// ���� �ּ� ����
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;

	// ���� ���ε�
	if (bind(g_ListenSocket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) == SOCKET_ERROR) {
		std::cout << "[ERROR] bind ���� : " << WSAGetLastError() << std::endl;
		closesocket(g_ListenSocket);
		WSACleanup();
	};

	// ���� ���
	if (listen(g_ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cout << "[ERROR] listen ���� : " << WSAGetLastError() << std::endl;
		closesocket(g_ListenSocket);
		WSACleanup();
	};

	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);

	//CompletionPort��ü ���� ��û�� �Ѵ�.
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	if (h_iocp == nullptr) {
		std::cout << "[ERROR] CreateIoCompletionPort ���� : " << GetLastError() << std::endl;
	}
	auto h_iocpHandle = CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_ListenSocket), h_iocp, 9999, 0);
	if (h_iocpHandle == nullptr) {
		std::cout << "[ERROR] ListenSocket IOCP bind ���� : " << WSAGetLastError() << std::endl;
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
		std::cout << "[INFO] Ŭ���̾�Ʈ [" << client_id << "] �α׾ƿ� - ID: " << logout_id << std::endl;
		logged_in_users.erase(logout_id);				// �α��� ����Ʈ���� ����
		g_network.clients[client_id].m_login_id.clear();			// ������ ID ���� ����
		g_room_manager.LeaveRoom(client_id);
	}

	closesocket(g_network.clients[client_id].m_socket);			// Ŭ���̾�Ʈ ���� �ݱ�
	g_network.clients[client_id].m_socket = INVALID_SOCKET;
	g_network.clients[client_id].m_used = false;					// ��Ȱ��ȭ
	g_network.clients[client_id].m_id = -1;						// ID �ʱ�ȭ
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
				std::cout << "[INFO] Ŭ���̾�Ʈ [" << key << "] ������ ���� ����. ���� ����." << std::endl;
				CloseClient(static_cast<int>(key));  // ���� ����
			}
			else if (ex_over && ex_over->_comp_type == OP_ACCEPT) {
				std::cout << "[ERROR] Accept ����" << std::endl;
			}
			else {
				std::cout << "[WARNING] GetQueuedCompletionStatus ���� �߻�: " << err << std::endl;
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

				std::cout << "[INFO] Ŭ���̾�Ʈ [" << client_id << "] ����!!!" << std::endl;

				CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_client), h_iocp, client_id, 0);
				clients[client_id].do_recv();
				m_client = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else {
				std::cout << "[ERROR] �ο� �ʰ�!!" << std::endl;
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
			if (num_bytes == 0) {									// Ŭ���̾�Ʈ�� ���������� ����� ���
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
		std::cout << "[ERROR] �߸��� Ŭ���̾�Ʈ ID : " << client_id << std::endl;
		return;
	}

	int packet_type = buffer[0]; // ù ����Ʈ�� ��Ŷ Ÿ��

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
		std::cout << "[ERROR] �� �� ���� ��Ŷ ������. Ŭ���̾�Ʈ [" << client_id << "]" << std::endl;
		break;
	}
}

//------------------------------------[Recv Packet]------------------------------------

// �� ���� ���� ����
void Network::ProcessRoomJoin(int client_id, char* buffer, int length) {
	int room_id = buffer[2];  // Ŭ���̾�Ʈ�� �Է��� �� ��ȣ

	if (g_room_manager.JoinRoom(client_id, room_id)) {
		SendRoomJoinResponse(client_id, true, room_id);
	}
	else {
		SendRoomJoinResponse(client_id, false, room_id);
	}
}

// Ŀ���� ����¡
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

// �ΰ��� �� �÷��̾������ ������Ʈ
void Network::ProcessUpdatePlayer(int client_id, char* buffer, int length) {
	CS_Packet_UPDATECOORD* pkt = reinterpret_cast<CS_Packet_UPDATECOORD*>(buffer);
	g_client[client_id].SetCoord(pkt->x, pkt->y, pkt->z);
	g_client[client_id].SetAngle(pkt->angle);

	int room_id = g_room_manager.GetRoomID(client_id);
	if (room_id == -1) return;

	Room& room = g_room_manager.GetRoom(room_id);
	const auto& client = room.GetClients();

	// �ٸ� Ŭ��鿡�� ��ǥ ����
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

// ���� ���� ��ư ���� ����
void Network::ProcessGameStartButton(int client_id) {
	int room_id = g_room_manager.GetRoomID(client_id);
	if (room_id == -1) return;

	Room& room = g_room_manager.GetRoom(room_id);
	room.PushStartGameButton(client_id);
}

// ���� �������� ���� ���� ����
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

// ���� �������� ��ŵ(���� �ð� ����)
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

// ���� �������� ���� �غ� �Ϸ� ��ư ���� ����
void Network::ProcessZenithStartButton(int client_id) {
	int room_id = g_room_manager.GetRoomID(client_id);
	if (room_id == -1) return;

	Room& room = g_room_manager.GetRoom(room_id);
	room.PushStartZenithButton(client_id);
}

// ���� �������� ���� ���� ����
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

// �ΰ��� �� ä��
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

// ���� ���� HP ������Ʈ
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

	// ���� ���� ������ ������ ��� ������
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

		if (dropItem > 0 && dropItem <= 3) {		// ����
			room.ADDJobWeapon(dropItem);
			pkt3.itemNum = room.GetWeaponTypeNum(dropItem);
		}
		else if (dropItem > 3 && dropItem <= 6) {	// ������
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

// ���� ���� HP ������Ʈ
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

// �κ��丮 ���� �� ������ ����
void Network::ProcessInventorySelcet(int client_id, char* buffer, int length) {
	int room_id = g_room_manager.GetRoomID(client_id);
	Room& room = g_room_manager.GetRoom(room_id);
	const auto& client = room.GetClients();

	CS_Packet_Inventory* pkt = reinterpret_cast<CS_Packet_Inventory*>(buffer);
	SC_Packet_Inventory pkt2;		// ��� Ŭ���� �κ��丮���� �ش� �������� �ϳ� �����ϱ� ���� ��Ŷ
	pkt2.type = SC_PACKET_INVENTORY;
	pkt2.size = sizeof(SC_Packet_Inventory);
	SC_Packet_SelectItem pkt3;		// �ش� Ŭ���� ���â�� �ش� �������� �����ϱ� ���� ��Ŷ
	pkt3.type = SC_PACKET_SELECTITEM;
	pkt3.item = pkt->item;
	pkt3.clientID = client_id;
	pkt3.size = sizeof(SC_Packet_SelectItem);
	// ���� �����߰�, ������ ���Ⱑ ���� ��(�ָ�), ������ ���Ⱑ 1���̻��� ��
	if (pkt->item > 0 && pkt->item <= 3
		&& static_cast<int>(g_client[client_id].GetWeaponType()) == 0
		&& room.GetWeaponTypeNum(pkt->item) > 0) {
		g_client[client_id].SetWeapon(pkt->item);
		room.DecideJobWeapon(pkt->item);				// ��(�κ��丮)�� �ִ� �ش� ���� �ϳ� ����
		pkt2.item = pkt->item;
		pkt2.num = room.GetWeaponTypeNum(pkt->item);
		for (int other_id : client) {
			g_network.clients[other_id].do_send(pkt2);
			g_network.clients[other_id].do_send(pkt3);
		}

	}
	// ������ �����߰�, ������ ������ ���� ��(������), ������ �������� 1�� �̻��϶�
	else if (pkt->item > 3 && pkt->item <= 6
		&& static_cast<int>(g_client[client_id].GetJobType()) == 0
		&& room.GetJobTypeNum(pkt->item) > 0)			// ��(�κ��丮)�� �ִ� �ش� ������ �ϳ� ����
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

// ���� ��ȭ(���â)
void Network::ProcessItemState(int client_id, char* buffer, int length) {
	CS_Packet_ItemState* pkt = reinterpret_cast<CS_Packet_ItemState*>(buffer);

	int room_id = g_room_manager.GetRoomID(client_id);
	Room& room = g_room_manager.GetRoom(room_id);
	//const auto& client = room.GetClients();

	// 1. ��ȭ �õ� ��ư�� ������ �� 2. ���� ��尡 0�� �̻��� ��  3. ��ȭ ���� ����� 9��� �Ʒ��� ��
	if (pkt->enhanceTry == true && room.GetGold() > 0 && g_client[client_id].GetWeaponGrade() < 9) {
		room.SpendGold(g_client[client_id].GetWeaponGrade() + 1);		// ���� ��ȭ ��� ����
		g_client[client_id].SetWeaponGrade();							// ���� ��ȭ �õ�
	}

	SC_Packet_ItemState pkt2;
	pkt2.type = SC_PACKET_ITEMSTATE;
	pkt2.result = g_client[client_id].GetWeaponGrade();
	pkt2.size = sizeof(pkt2);

	g_network.clients[client_id].do_send(pkt2);


}

// ������ ��� �߰�
void Network::ProcessDebugGold(int client_id, char* buffer, int length) {
	CS_Packet_DebugGold* pkt = reinterpret_cast<CS_Packet_DebugGold*>(buffer);

	int room_id = g_room_manager.GetRoomID(client_id);
	Room& room = g_room_manager.GetRoom(room_id);

	if (pkt->plusGold = true) {
		room.AddGold(10);
	}

}

// ������ ���� �� ������ �߰�
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

	if (itemNum > 0 && itemNum <= 3) {		// ����
		room.ADDJobWeapon(itemNum);
		pkt2.itemNum = room.GetWeaponTypeNum(itemNum);
	}
	else if (itemNum > 3 && itemNum <= 6) {	// ������
		room.AddJobDocument(itemNum);
		pkt2.itemNum = room.GetJobTypeNum(itemNum);
	}

	for (int other_id : client) {
		g_network.clients[other_id].do_send(pkt2);
	}

}

// �ִϸ��̼�
void Network::ProcessAnimation(int client_id, char* buffer, int length) {
	CS_Packet_Animaition* pkt = reinterpret_cast<CS_Packet_Animaition*>(buffer);

	int room_id = g_room_manager.GetRoomID(client_id);
	Room& room = g_room_manager.GetRoom(room_id);
	const auto& client = room.GetClients();

	int job = (int)g_client[client_id].GetJobType();  // 0 : ������,  1 : ����, 2 : ������, 3 : ����Ŀ

	SC_Packet_Animaition pkt2;
	pkt2.type = SC_PACKET_ANIMATION;
	pkt2.client_id = client_id;

	// 0 : �⺻      1 : �ȱ�      2 : �޸���      3 : ������ �⺻ ����      4 : ������ų      5 : ���� �⺻ ����
	if (pkt->animation == 4) {
		if (!g_client[client_id].CanUseSkill()) return;		// ��ų ��Ÿ���̸� ����

		switch (job) {
		case 0: pkt2.animation = 3; break;		// ������ ��ų(�⺻����)
		case 1: pkt2.animation = 4; break;		// ��  �� ��ų
		case 2: pkt2.animation = 5; break;		// ������ ��ų
		case 3: pkt2.animation = 6; break;		// ����Ŀ ��ų
		}
		g_client[client_id].StartCoolTime();	// ������ ��ų ��� ����
	}
	else if (pkt->animation == 5) {
		switch (job) {
		case 0: pkt2.animation = 3; break;		// ������ �⺻����
		case 1: pkt2.animation = 7; break;		// ��  �� �⺻����
		case 2: pkt2.animation = 8; break;		// ������ �⺻����
		case 3: return;							// ����Ŀ �⺻���� X
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

// ��ų, �⺻ ���� ����Ʈ(����, ������)
void Network::ProcessAttackEffect(int client_id, char* buffer, int length)
{
	CS_Packet_AttackEffect* pkt = reinterpret_cast<CS_Packet_AttackEffect*>(buffer);

	int room_id = g_room_manager.GetRoomID(client_id);
	Room& room = g_room_manager.GetRoom(room_id);
	const auto& client = room.GetClients();

	// �ڽ��� ������ Ÿ Ŭ������ ����ü Ȥ�� ��ų ����Ʈ ������
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

// ���� �Ա�
void Network::ProcessEatHealPack(int client_id, char* buffer, int length)
{
	CS_Packet_HealPack* pkt = reinterpret_cast<CS_Packet_HealPack*>(buffer);
	if (pkt->eat == true) {
		g_client[client_id].AddHP(10);
	}
}

// �÷��̾� ������
void Network::ProcessDamaged(int client_id, char* buffer, int length)
{
	CS_Packet_Damaged* pkt = reinterpret_cast<CS_Packet_Damaged*>(buffer);

	int room_id = g_room_manager.GetRoomID(client_id);
	Room& room = g_room_manager.GetRoom(room_id);
	const auto& client = room.GetClients();

	if (room.GetMode() == 1) {		// ���� ��������
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
	else if (room.GetMode() == 3) {	// ���� ��������
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

// �� ���� ���� ���� ����
void Network::SendRoomJoinResponse(int client_id, bool success, int room_id) {
	SC_Packet_RoomResponse packet;
	packet.type = SC_PACKET_ROOM_RESPONSE;
	packet.size = sizeof(packet);
	packet.success = success;
	packet.room_id = room_id;
	g_network.clients[client_id].do_send(packet);
}

// ����� ���ӹ� �ο��� ��Ȳ �����ֱ� (�����ҿ���)
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

// ���� ���� ��ư ���� ����
void Network::SendGameStart(const std::vector<int>& client_id) {
	SC_Packet_GameStart packet;
	packet.type = SC_PACKET_GAMESTART;
	packet.size = sizeof(packet);
	packet.startCS = true;

	for (int id : client_id) {
		if (g_network.clients[id].m_used) {
			g_network.clients[id].do_send(packet);
			std::cout << "[INFO] Ŭ���̾�Ʈ [" << id << "] ���� ���� ��Ŷ ����" << std::endl;
		}
	}
}

// ���̵� �� �����ֱ�
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

// ���� �������� ���� �� ��ġ ����
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

// ���� �������� ���� �� ��ġ ����
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

// ���� �ð� ����
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

// ���� �������� �̵� ���
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

// ��� ������Ʈ
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

// ���� ���� ��ǥ �ʱ� ����
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

// ���� ���� ��ǥ �ʱ� ����
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

// �÷��̾� ü�� ������Ʈ
void Network::SendPlayerHP(int client_id)
{
	SC_Packet_PlayerHP pkt;
	pkt.type = SC_PACKET_PLAYERHP;
	pkt.hp = g_client[client_id].GetHP();
	pkt.size = sizeof(pkt);

	g_network.clients[client_id].do_send(pkt);
}

// ���� ����
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

// ��� ���� �� ���� ����
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