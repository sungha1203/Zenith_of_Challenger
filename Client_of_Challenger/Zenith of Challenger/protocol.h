#pragma once
#pragma once
#include <Windows.h>
#pragma pack(push,1)

constexpr int SERVER_PORT = 4000;
constexpr int WORKER_THREAD_COUNT = 4;

constexpr int BUF_SIZE = 1024;
constexpr int MAX_USER = 10;				// ���� ������ �� ���� ��
constexpr int NAME_SIZE = 20;				// �̸� ���� ��

// ��Ŷ Ÿ�� ���� (Ŭ�� -> ����)
#define CS_PACKET_LOGIN				 1			// �α��� ��û
#define CS_PACKET_ROOM				 2			// ���ӹ� ��û
#define CS_PACKET_CUSTOMIZE			 3			// Ŀ���͸���¡
#define CS_PACKET_GAMESTART			 4			// ���� ���� ��ư
#define CS_PACKET_INGAMEREADY		 5			// ���� ���� �� ���� �Ϸ�
#define CS_PACKET_STARTZENITH		 6			// ���� �������� ���� �Ϸ�
#define CS_PACKET_LOGOUT			 100		// �α׾ƿ�

// ��Ŷ Ÿ�� ���� (���� -> Ŭ��)
#define SC_PACKET_LOGIN_RESPONSE	 101		// �α��� ����
#define SC_PACKET_ROOM_RESPONSE		 102		// ���ӹ� ����
#define SC_PACKET_ROOMLIST			 103		// ���ӹ� �ο��� ����
#define SC_PACKET_GAMESTART			 104		// ���� ����
#define SC_PACKET_REPAIRTIME		 105		// ���� �ð�
#define SC_PACKET_ZENITHSTAGE		 106		// ���� -> ����
#define SC_PACKET_LOGOUT			 110		// �α׾ƿ�

// [�ӽ�]
#define SC_PACKET_CLIENTINFORMATION	 999

//---------------------------------------
struct CS_Packet_Login
{
	char	type;
	int		size;
	char	loginData[256];
};

struct CS_Packet_Room
{
	char	type;
	UCHAR	size;
	UCHAR	room_id;
};

struct CS_Packet_Customize
{
	char	type;
	int		size;
	int		clothes[3];
};

struct CS_Packet_GameStart
{
	char	type;
	int		size;

};

struct CS_Packet_GameReady
{
	char	type;
	int		size;
	bool	ReadySuccess;
};

struct CS_Packet_ZenithReady
{
	char	type;
	int		size;
	bool	ReadySuccess;
};

struct CS_Packet_Logout
{
	char	type;
	UCHAR	size;
};

//---------------------------------------

struct SC_Packet_LoginResponse
{
	char	type;
	UCHAR	size;
	UCHAR	success;			// �α��� ���� ����
};

struct SC_Packet_RoomResponse
{
	char	type;
	UCHAR	size;
	UCHAR	success;			// ���ӹ� ���� ���� ����
	UCHAR	room_id;			// ���ȣ
};

struct SC_Packet_Logout
{
	char	type;
	UCHAR	size;
};

struct SC_Packet_RoomList
{
	char	type;
	UCHAR	size;
	int		room_id;			// ���ӹ� �ѹ�
	int		current_players;	// ���� �÷��̾� ��
};

struct SC_Packet_GameStart
{
	char	type;
	UCHAR	size;
	bool	startCS;			// ���� ��������(���� ����)
};

struct SC_Packet_RepairTime
{
	char	type;
	UCHAR	size;
	bool	startRT;			// ���� �ð�
};

struct SC_Packet_ZenithStage
{
	char type;
	UCHAR	size;
	bool	startZS;			// ���� ��������
};

// [�ӽ�]
struct SC_Packet_ClientInformation
{
	char	type;
	UCHAR   size;
	int		hp;
	int		attack;
	int		speed;
	int		attackspeed;
};
#pragma pack(pop)