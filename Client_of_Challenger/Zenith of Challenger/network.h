#pragma once
#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include "GameFramework.h"
#include "protocol.h"

#pragma comment (lib, "ws2_32.lib")

constexpr int PORT_NUM = 4000;
constexpr const char* SERVER_IP = "127.0.0.1";

class ClientNetwork
{
public:
	ClientNetwork();
	~ClientNetwork();

	void		Connect();
	void		Disconnect();
	bool		SendPacket(const char* data, int length);

	void		ProcessLogin(char* buffer);
	void		ProcessRoomjoin(char* buffer);
	void		ProcessGamestart(char* buffer);
	void		ProcessInitialstate(char* buffer);
	void		ProcessInitMonster(char* buffer);
	void		ProcessUpdateCoord2Player(char* buffer);

public:
	SOCKET			m_clientsocket = INVALID_SOCKET;
	std::thread		m_recvThread;
	bool			m_running = false;

	int				m_clientID = 0;


private:
	void		Receive();
};