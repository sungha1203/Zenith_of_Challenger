#pragma once
#include "stdafx.h"
extern std::unordered_map<int, ClientInfo> g_client;

enum class JobWeapon
{
	SWORD,
	WAND,
	SHIELD
};

enum class JobDocument
{
	WARRIOR,
	MAGICIAN,
	TANKER
};

enum class Stage
{
	LOBBY,
	CHALLENGE_STAGE,
	REPAIR_TIME,
	ZENITH_STAGE
};

struct InventoryItem								// ���� �κ��丮
{
	int gold = 0;									// ��ȭ
	std::map<JobWeapon, int> JobWeapons;			// ����
	std::map<JobDocument, int> JobDocuments;		// ������
};

// ���� ����, �������� ����, Ŭ���̾�Ʈ ����
class Room
{
private:
	bool				m_IsGaming;					// ���������� Ȯ��
	int					m_room_id;					// ���° ������
	Stage				m_RoomState;				// ���� ���ӹ� ����(�κ�, ����, ����ð�, ����)
	std::vector<int>	m_clients;					// ������ Ŭ���̾�Ʈ ID��
	InventoryItem		m_inventory;				// ������ �κ��丮 â

	std::atomic<bool>	m_stopTimer = false;		// Ŭ���̾�Ʈ ��� ������ ������ ���� ���
	std::thread			m_timer;
	int					m_playerNum = 0;			// ���� �����ϴ� �� �÷��̾� �ο���
	int					m_enterClientNum = 0;		// ���� ���������� ������ Ŭ���̾�Ʈ ��� (�� ���;��� Ÿ�̸� ����)
	int					m_enterZenithNum = 0;		// ������������ ���� ��� ��� (�� ���;��� Ÿ�̸� ����)
	std::mutex			m_PlayerMx;
	std::mutex			m_inventoryMx;

	static constexpr int CHALLENGE_TIME = 480;		// 8��
	//static constexpr int REPAIR_TIME	= 120;		// 2��
	static constexpr int ZENITH_TIME	= 300;		// 5��
	 

public:
	Room() : m_room_id(-1), m_IsGaming(false), m_RoomState(Stage::LOBBY) {}

	void    Init(int room_id);							// �� �ʱ�ȭ

	void	AddClient(int client_id);					// Ŭ�� ����
	void    RemoveClient(int client_id);				// Ŭ�� ���� ����

	void	PushStartGameButton(int RoomMasterID);		// ���� ���� �õ�
	void	PushStartZenithButton(int RoomMasterID);	// ������ ��� �÷��̾� �������� �̵� �õ�

	void    AllPlayerNum(int playerNum);				// �� �÷��̾� �ο���
	void    PlusPlayerReadyNum();						// �÷��̾� ���� ++1
	void	PlusPlayerZenithReadyNum();					// ���� �������� ���� ��� ��� ++1
	void    ChallengeTimerThread();						// ���� �������� Ÿ�̸� ������
	void	ZenithTimerThread();						// ���� �������� Ÿ�̸� ������
	void	StartGame();								// �κ�			-> ������������
	void	RepairTime();								// ���� �������� �� ������ �ð�
	void	StartZenithStage();							// ���� �������� -> ������������
	void	EndGame();									// ���� �������� -> �κ�
	
	int		GetClientsNum() const { return m_clients.size(); }								// �� �濡 ����־�?
	int		GetRoomMasterID() const { return m_clients.empty() ? -1 : m_clients.front(); }	// ���� ������?
	int		GetPlayerNum() const { return m_playerNum; }									// ���� �����Ҷ� ����� �����߾�?
	int		GetEnterClientNum() const { return m_enterClientNum; }							// ���� ����� ���� ������̾�?
	int		GetEnterZenithNum() const { return m_enterZenithNum;  }							// ���� ����� ���� �������� ������̾�?
	const std::vector<int>& GetClients() const { return m_clients; }						// ���� ���� ��� Ŭ���̾�Ʈ

	// �κ��丮
	void	AddGold(int plusgold);				// +���
	void    SpendGold(int minusgold);			// -���
	void	ADDJobWeapon(JobWeapon weapon);		// +����
	void	DecideJobWeapon(JobWeapon weapon);	// -����
	void	AddJobDocument(JobDocument job);	// +������
	void	DecideJobDocument(JobDocument job);	// -������
};