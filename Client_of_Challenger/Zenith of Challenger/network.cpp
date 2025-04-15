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
	char buffer[1024];
	while (m_running) {
		int received = recv(m_clientsocket, buffer, sizeof(buffer), 0);
		if (buffer > 0) {
			switch (buffer[0]) {
			case SC_PACKET_LOGIN_RESPONSE:
				ProcessLogin(buffer);
				break;
			case SC_PACKET_ROOM_RESPONSE:
				ProcessRoomjoin(buffer);
				break;
			case SC_PACKET_GAMESTART:
				ProcessGamestart(buffer);
				break;
			case SC_PACKET_INITIALSTATE:
				ProcessInitialstate(buffer);
			default:
				break;
			}
		}
		else if (buffer == 0) {

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
	if(pkt->success)
		gGameFramework->GetClientState()->SetClientRoomNum(pkt->room_id);
}

void ClientNetwork::ProcessGamestart(char* buffer)
{
	SC_Packet_GameStart* pkt = reinterpret_cast<SC_Packet_GameStart*>(buffer);
	if (pkt->startCS = true)
		gGameFramework->IsSuccess = true;
}

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@지환이한테 물어보자
void ClientNetwork::ProcessInitialstate(char* buffer)
{
	SC_Packet_initialstate* pkt = reinterpret_cast<SC_Packet_initialstate*>(buffer);
	// 일단 자기 좌표만 받을거임.
	if (pkt->client_id == m_clientID) {
		XMFLOAT3 pos(pkt->x, pkt->y, pkt->z);
		auto player = gGameFramework->GetPlayer();
		if (player)
			player->SetPosition(pos);
	}
}