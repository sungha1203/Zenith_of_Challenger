#pragma once
#include "stdafx.h"
#include "session.h"

class Monster;

class Network{
public:
	Network();
	~Network();

	void			init();
	int				CreateClient();														// Ŭ���̾�Ʈ idx ����
	static void		CloseClient(int client_id);											// Ŭ���̾�Ʈ idx ����

	void			WorkerThread();														// Ŭ���̾�Ʈ ��Ʈ��ũ �̺�Ʈ ó��
	static void		HandlePacket(int client_id, char * buffer, int length);				// ��Ŷ �м�

	static void		ProcessLogin(int client_id, char * buffer, int length);				// �α��� ���� ����
	static void		ProcessRoomJoin(int client_id, char * buffer, int length);			// �� ���� ���� ����
	static void		ProcessCustomize(int client_id, char * buffer, int length);			// Ŀ���͸���¡
	static void		ProcessUpdatePlayer(int client_id, char * buffer, int length);		// �ΰ��� �� �÷��̾������ ������Ʈ
	static void		ProcessGameStartButton(int client_id);								// ���� ���� ��ư ���� ����
	static void		ProcessIngameReady(int client_id, char * buffer, int length);		// ���� �������� ���� ���� ����
	static void     ProcessSkipChallenge(int client_id, char * buffer, int length);		// ���� �������� ��ŵ(���� �ð� ����)
	static void		ProcessZenithStartButton(int client_id);							// ���� �������� ���� �غ� �Ϸ� ��ư ���� ����
	static void		ProcessZenithReady(int client_id, char * buffer, int length);		// ���� �������� ���� ���� ����
	static void		ProcessChat(int client_id, char * buffer, int length);				// �ΰ��� �� ä��
	static void		ProcessMonsterHP(int client_id, char * buffer, int length);			// ���� ���� HP ������Ʈ
	static void		ProcessZMonsterHP(int client_id, char* buffer, int length);			// ���� ���� HP ������Ʈ
	static void		ProcessInventorySelcet(int client_id, char * buffer, int length);	// �κ��丮 ���� �� ������ ����
	static void		ProcessItemState(int client_id, char * buffer, int length);			// ��ȭ �� ���� ����(���â)
	static void		ProcessDebugGold(int client_id, char * buffer, int length);			// ������ ��� �߰�
	static void		ProcessDebugItem(int client_id, char * buffer, int length);			// ������ ���� �� ������ �߰�
	static void		ProcessAnimation(int client_id, char * buffer, int length);			// �ִϸ��̼�
	static void		ProcessAttackEffect(int client_id, char* buffer, int length);		// ��ų, �⺻ ���� ����Ʈ(����, ������)
	static void		ProcessEatHealPack(int client_id, char* buffer, int length);		// ���ѸԱ�
	static void		ProcessDamaged(int client_id, char* buffer, int length);			// �������� ���� �Ծ��� ��

	// ---------��Ŷ �ѷ��ֱ�---------
	static void		SendLoginResponse(int client_id, bool success);
	static void		SendRoomJoinResponse(int client_id, bool success, int room_id);		// Ŭ������ �� ���� �������� �ѷ��ֱ�
	static void		SendRoomInfo(int room_id, int client_num);							// �� ��� ����
	static void		SendGameStart(const std::vector<int> & client_id);					// ���� ���� ��ư ���� ����
	static void		SendWhoIsMyTeam(const std::vector<int> & client_id);				// ���̵� �� �����ֱ�
	static void		SendInitialState(const std::vector<int> & client_id);				// ���� �������� �� �ʱ� ����
	static void		SendZenithState(const std::vector<int>& client_id);					// ���� �������� �� �ʱ� ����
	static void		SendStartRepairTime(const std::vector<int> & client_id);			// ���� �ð�(8�� ������ ������ ������ �̵�)
	static void		SendStartZenithStage(const std::vector<int> & client_id);			// ���� -> ���� ��������
	static void     SendPlayerAttack(const std::vector<int>& client_id);				// ���� �������� �̵� �� ���ݷ� ����
	static void		SendPlayerRespone(int client_id);									// ����, �������� �ǰ� 0�� �Ǹ� �ٽ� �¾�� �� ����
	static void		SendUpdateGold(const std::vector<int> & client_id);					// ��� ������Ʈ
	static void     SendPlayerHP(int client_id);										// �÷��̾� ü�� ������Ʈ				
	static void		SendInitMonster(const std::vector<int> & client_id, const std::array<Monster, 50> & monsters);		// ���� ���� �ʱ� ��ǥ ����
	static void		SendZenithMonster(const std::vector<int> & client_id, const std::array<Monster, 26> & monsters);	// ���� ���� �ʱ� ��ǥ ����
	static void		SendZMonsterAttack(const std::vector<int>& client_id, int MonsterID, int SkillType);				// ���� ���� ����
	static void		SendEndGame(const std::vector<int>& client_id, int time);											// ���� ����

public:
	SOCKET										m_client;								// Ŭ���̾�Ʈ ����
	HANDLE										h_iocp;									// CompletionPort ��ü �ڵ�

	std::vector<std::thread>					workers;								// ��� ������
	std::array<SESSION, MAX_USER>				clients;								// Ŭ���̾�Ʈ (���� ����)
};