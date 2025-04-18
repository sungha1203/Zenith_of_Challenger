#pragma once
#include "stdafx.h"
#include "session.h"

class Monster;

class Network
{
public:
	Network();
	~Network();

	void		init();
	int			CreateClient();													// Ŭ���̾�Ʈ idx ����
	void		CloseClient(int client_id);										// Ŭ���̾�Ʈ idx ����

	void		WorkerThread();													// Ŭ���̾�Ʈ ��Ʈ��ũ �̺�Ʈ ó��
	void		HandlePacket(int client_id, char* buffer, int length);			// ��Ŷ �м�

	void		ProcessLogin(int client_id, char* buffer, int length);			// �α��� ���� ����
	void		ProcessRoomJoin(int client_id, char* buffer, int length);		// �� ���� ���� ����
	void		ProcessCustomize(int client_id, char* buffer, int length);		// Ŀ���͸���¡
	void		ProcessUpdatePlayer(int client_id, char* buffer, int length);	// �ΰ��� �� �÷��̾������ ������Ʈ
	void		ProcessGameStartButton(int client_id);							// ���� ���� ��ư ���� ����
	void		ProcessIngameReady(int client_id, char* buffer, int length);	// ���� �������� ���� ���� ����
	void		ProcessZenithStartButton(int client_id);						// ���� �������� ���� �غ� �Ϸ� ��ư ���� ����
	void		ProcessZenithReady(int client_id, char* buffer, int length);	// ���� �������� ���� ���� ����

	// ---------��Ŷ �ѷ��ֱ�---------
	void		SendLoginResponse(int client_id, bool success);
	void		SendRoomJoinResponse(int client_id, bool success, int room_id); // Ŭ������ �� ���� �������� �ѷ��ֱ�
	void		SendRoomInfo(int room_id, int client_num);						// �� ��� ����
	void		SendGameStart(const std::vector<int>& client_id);				// ���� ���� ��ư ���� ����
	void		SendInitialState(const std::vector<int>& client_id);			// �ٸ� �÷��̾������ �� �ʱ� ����
	void		SendStartRepairTime(const std::vector<int>& client_id);			// ���� �ð�(8�� ������ ������ ������ �̵�)
	void		SendStartZenithStage(const std::vector<int>& client_id);		// ���� -> ���� ��������
	void		SendUpdateInventory(const std::vector<int>& client_id);			// �κ��丮 ������Ʈ
	void		SendInitMonster(const std::vector<int>& client_id, const std::unordered_map<int, Monster>& monsters);	// ���� �ʱ� ��ǥ ����
	// ---------��Ŷ �ѷ��ֱ�---------

public:
	SOCKET										m_client;						// Ŭ���̾�Ʈ ����
	HANDLE										h_iocp;							// CompletionPort ��ü �ڵ�

	std::vector<std::thread>					workers;						// ��� ������
	std::array<SESSION, MAX_USER>				clients;						// Ŭ���̾�Ʈ (���� ����)
};