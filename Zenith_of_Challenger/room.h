#pragma once
#include "stdafx.h"
#include "monster.h"
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

	InventoryItem()
	{
		for (int i = 0; i <= 2; ++i)
		{
			JobWeapons[static_cast<JobWeapon>(i)] = 0;
		}
		for (int i = 0; i <= 2; ++i)
		{
			JobDocuments[static_cast<JobDocument>(i)] = 0;
		}
	}
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
	std::atomic<bool>	m_skipTimer = false;		// ������������ ��ŵ�� ������ �� -> true
	std::atomic<bool>	m_skipButton = false;		// ��ŵ ��ư �ι� �̻� ������
	std::thread			m_timer;
	int					m_playerNum = 0;			// ���� �����ϴ� �� �÷��̾� �ο���
	int					m_enterClientNum = 0;		// ���� ���������� ������ Ŭ���̾�Ʈ ��� (�� ���;��� Ÿ�̸� ����)
	int					m_enterZenithNum = 0;		// ������������ ���� ��� ��� (�� ���;��� Ÿ�̸� ����)

	std::mutex			m_PlayerMx;
	std::mutex			m_inventoryMx;

	static constexpr int CHALLENGE_TIME = 480;		// 8��
	//static constexpr int REPAIR_TIME	= 120;		// 2��
	static constexpr int ZENITH_TIME	= 300;		// 5��
	
	std::array<Monster,50>	m_monsters;				// �濡�� �����ϴ� ����(����)
	int						m_MonsterNum;			// ���� ��(����)

public:
	Room() : m_room_id(-1), m_IsGaming(false), m_RoomState(Stage::LOBBY) {}

	void    Init(int room_id);							// �� �ʱ�ȭ
	void	ResetRoom();								// �� Ŭ����

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
	
	void	SetStopTimer();
	void	SetSkipTimer();
	void	SetSkipButton(bool check);

	int		GetClientsNum() const { return m_clients.size(); }								// �� �濡 ����־�?
	int		GetRoomMasterID() const { return m_clients.empty() ? -1 : m_clients.front(); }	// ���� ������?
	int		GetPlayerNum() const { return m_playerNum; }									// ���� �����Ҷ� ����� �����߾�?
	int		GetEnterClientNum() const { return m_enterClientNum; }							// ���� ����� ���� ������̾�?
	int		GetEnterZenithNum() const { return m_enterZenithNum;  }							// ���� ����� ���� �������� ������̾�?
	bool	GetSkipButton() const { return m_skipButton;  }									// ��ŵ ��ư ������ �ȴ�����?
	int		GetGold() const { return m_inventory.gold; }									// ��� ���־�?
	int		GetWeaponTypeNum(int num) const { return m_inventory.JobWeapons.at(static_cast<JobWeapon>(num-1));}		// �ش� ���� � �־�?
	int		GetJobTypeNum(int num) const { return m_inventory.JobDocuments.at(static_cast<JobDocument>(num-4));}		// �ش� ������ � �־�?

	const std::vector<int>& GetClients() const { return m_clients; }						// ���� ���� ��� Ŭ���̾�Ʈ
	const Monster&	GetMonsters(int monsterID) const { return m_monsters[monsterID]; }		// ����
	Monster& GetMonster(int monsterID) { return m_monsters[monsterID]; }

	// �κ��丮
	void	AddGold(int plusgold);					// +���
	void    SpendGold(int minusgold);				// -���
	void	ADDJobWeapon(int weapon);				// +����
	void	DecideJobWeapon(int weapon);			// -����
	void	AddJobDocument(int job);				// +������
	void	DecideJobDocument(int job);				// -������

	// ����
	void	InitChallengeMonsters();				// ���� �������� ���� �ʱ�ȭ
};