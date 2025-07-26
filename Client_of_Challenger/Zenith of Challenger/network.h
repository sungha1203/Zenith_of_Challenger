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
constexpr const char* SERVER_IP = "127.0.0.1";//"172.30.1.53"; //121.139.87.88 / 127.0.0.1 / 10.40.13.101

class ClientNetwork
{
public:
	ClientNetwork();
	~ClientNetwork();

	void		Connect();
	void		Disconnect();
	__declspec(noinline)
	bool		SendPacket(const char* data, int length);

	void		ProcessLogin(char* buffer);
	void		ProcessRoomjoin(char* buffer);
	void		ProcessWhoismyteam(char* buffer);
	void		ProcessCustomize(char* buffer);
	void		ProcessGamestart(char* buffer);
	void		ProcessInitialstate(char* buffer);
	void		ProcessInitMonster(char* buffer);
	void		ProcessZenithMonster(char* buffer);
	void		ProcessUpdateCoord2Player(char* buffer);
	void		ProcessChat(char* buffer);
	void		ProcessStartRepairTime(char* buffer);
	void		ProcessMonsterHP(char* buffer);
	void		ProcessZMonsterHP(char* buffer);
	void		ProcessItemDrop(char* buffer);
	void		ProcessGold(char* buffer);
	void		ProcessInventory(char* buffer);
	void		ProcessInventory2Equip(char* buffer);
	void		ProcessDebugItem(char* buffer);
	void		ProcessItemState(char* buffer);
	void		ProcessAnimation(char* buffer);
	void		ProcessPlayerAttack(char* buffer);
	void		ProcessAttackEffect(char* buffer);
	void		ProcessZenithState(char* buffer);
	void		ProcessZenithStage(char* buffer);
	void		ProcessCMonsterTarget(char* buffer);
	void		ProcessRespone(char* buffer);
	void		ProcessZMonsterMove(char* buffer);
	void		ProcessPlayerHP(char* buffer);
	void		ProcessBossAttackMotion(char* buffer);
	void		ProcessEndGame(char* buffer);
	void		ProcessZMonsterAttack(char* buffer);
	std::chrono::steady_clock::time_point m_lastAttackTime; 
public:
	SOCKET			m_clientsocket = INVALID_SOCKET;
	std::thread		m_recvThread;
	bool			m_running = false;

	int				m_clientID = 0;
	unsigned int	m_prevRemain;
	float m_challengerRatio = 50.0f;
	float m_swordRatio = 150.0f;
	float m_wizardRatio = 100.0f;
	float m_healTankerRatio = 500.0f;
private:
	void		Receive();
};
