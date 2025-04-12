#pragma once
#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include "protocol.h"

#pragma comment (lib, "ws2_32.lib")

constexpr int PORT_NUM = 4000;
constexpr const char* SERVER_IP = "172.30.1.53";

struct ClientState
{
	int clientID = -1;
	int RoomNum = -1;
};

class ClientNetwork
{
public:
	ClientNetwork();
	~ClientNetwork();

	void		Connect();
	void		Disconnect();
	bool		SendPacket(const char* data, int length);

public:
	SOCKET			m_clientsocket = INVALID_SOCKET;
	std::thread		m_recvThread;
	bool			m_running = false;

private:
	void		Receive();
};