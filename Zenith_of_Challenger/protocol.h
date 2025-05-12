#pragma once
#pragma pack(push,1)

constexpr int SERVER_PORT = 4000;

constexpr int BUF_SIZE = 1400;
constexpr int MAX_USER = 10;				// ���� ������ �� ���� ��
constexpr int NAME_SIZE = 20;				// �̸� ���� ��

// ��Ŷ Ÿ�� ���� (Ŭ�� -> ����)
#define CS_PACKET_LOGIN				 1			// �α��� ��û
#define CS_PACKET_ROOM				 2			// ���ӹ� ��û
#define CS_PACKET_CUSTOMIZE			 3			// Ŀ���͸���¡
#define CS_PACKET_UPDATECOORD		 4			// ��ǥ ���� ������Ʈ
#define CS_PACKET_GAMESTART			 5			// ���� ���� ��ư
#define CS_PACKET_INGAMEREADY		 6			// ���� ���� �� ���� �Ϸ�
#define CS_PACKET_STARTZENITH		 7			// ���� �������� ���� �Ϸ�
#define CS_PACKET_MONSTERHP			 8			// ���� HP
#define CS_PACKET_CHAT				 9			// �ΰ��� �� ä��
#define CS_PACKET_ITEMSTATE			 10			// ���â���� ��ȭ
#define CS_PACKET_INVENTORY			 11			// �κ��丮���� ������ ����
#define CS_PACKET_DEBUGGOLD			 12			// ������ ��� �߰�
#define CS_PACKET_DEBUGITEM			 13			// ������ ������ �߰�
#define CS_PACKET_ANIMATION			 14			// �ִϸ��̼�

#define CS_PACKET_SKIPCHALLENGE		 99			// ������������ ��ŵ
#define CS_PACKET_LOGOUT			 100		// �α׾ƿ�

// ��Ŷ Ÿ�� ���� (���� -> Ŭ��)
#define SC_PACKET_LOGIN_RESPONSE	 101		// �α��� ����
#define SC_PACKET_ROOM_RESPONSE		 102		// ���ӹ� ����
#define SC_PACKET_ROOMLIST			 103		// ���ӹ� �ο��� ����
#define SC_PACKET_WHOISMYTEAM		 104		// ���� ���� �ִ��� �����ٰ�
#define SC_PACKET_INITIALSTATE		 105		// �ٸ� �÷��̾������ �� �ʱ� ����
#define SC_PACKET_UPDATE2PLAYER		 106		// �ٸ� �÷��̾������ �ٲ� ��ǥ ����
#define SC_PACKET_GAMESTART			 107		// ���� ����
#define SC_PACKET_REPAIRTIME		 108		// ���� �ð�
#define SC_PACKET_ZENITHSTAGE		 109		// ���� -> ����
#define SC_PACKET_INITMONSTER		 110		// ���� �ʱ� ����
#define SC_PACKET_MONSTERHP			 111		// ���� HP
#define SC_PACKET_DROPITEM			 112		// ���� ��� ������
#define SC_PACKET_GOLD				 113		// ��� �� ��Ȳ ����
#define SC_PACKET_INVENTORY			 114		// �κ��丮 ���� ��Ȳ
#define SC_PACKET_SELECTITEM		 115		// �κ��丮���� ���� or ������ ����
#define SC_PACKET_ITEMSTATE			 116		// ���� ��ȭ ���� ����
#define SC_PACKET_CHAT				 117		// �ΰ��� �� ä��
#define SC_PACKET_DEBUGITEM			 118		// ������ ������ �߰�
#define SC_PACKET_ANIMATION			 119		// �ִϸ��̼�

#define SC_PACKET_SKIPCHALLENGE		 998		// ������������ ��ŵ
#define SC_PACKET_LOGOUT			 999		// �α׾ƿ�

//---------------------------------------
struct CS_Packet_Login{
	char	type;
	int		size;
	char	loginData[256];
};

struct CS_Packet_Room{
	char	type;
	int	size;
	UCHAR	room_id;
};

struct CS_Packet_Customize{
	char	type;
	int		size;
	int		clothes[3];
};

struct CS_Packet_UPDATECOORD{
	char	type;
	int		size;
	float	x, y, z;  // ��ġ
	float	angle;
};

struct CS_Packet_GameStart{
	char	type;
	int		size;
};

struct CS_Packet_GameReady{
	char	type;
	int		size;
	bool	ReadySuccess;
};

struct CS_Packet_ZenithReady{
	char	type;
	int		size;
	bool	ReadySuccess;
};

struct CS_Packet_Chat{
	char	type;
	int		size;
	char	msg[256];
};

struct CS_Packet_SkipChallenge{
	char	type;
	int		size;
	bool	skip;
};

struct CS_Packet_MonsterHP{
	char	type;
	int		size;
	int		monsterID;
	int		damage;
};

struct CS_Packet_Inventory{
	char	type;
	int		size;
	int		item;	// 1.�� / 2.������ / 3.���� / 4.���� ������ / 5.������ ������ / 6.����Ŀ ������ 
};

struct CS_Packet_ItemState{
	char	type;
	int		size;
	bool	enhanceTry;		// ��ȭ �õ�
};

struct CS_Packet_DebugGold{
	char	type;
	int		size;
	bool	plusGold;
};

struct CS_Packet_DebugItem{
	char	type;
	int		size;
	int		item;
};

struct CS_Packet_Animaition{
	char	type;
	int		size;
	int		animation;
};

struct CS_Packet_Logout{
	char	type;
	int	size;
};

//---------------------------------------

struct SC_Packet_LoginResponse{
	char	type;
	int	size;
	bool	success;			// �α��� ���� ����
	int		clientID;			// ���� Ŭ���̾�Ʈ ID
};

struct SC_Packet_RoomResponse{
	char	type;
	int	size;
	bool	success;			// ���ӹ� ���� ���� ����
	int		room_id;			// ���ȣ
};

struct SC_Packet_Logout{
	char	type;
	int	size;
};

struct SC_Packet_RoomList{
	char	type;
	int	size;
	int		room_id;			// ���ӹ� �ѹ�
	int		current_players;	// ���� �÷��̾� ��
};

struct SC_Packet_MyTeam{
	char	type;
	int		size;
	int		teamID[3];
	int		teamSize;
};

struct SC_Packet_initialstate{
	char	type;
	int		size;
	int		client_id;
	float	x, y, z;
};

struct SC_Packet_Update2Player{
	char	type;
	int		size;
	int		client_id;
	float	x, y, z;
	float   angle;
};

struct SC_Packet_GameStart{
	char	type;
	int	size;
	bool	startCS;			// ���� ��������(���� ����)
};

struct SC_Packet_RepairTime{
	char	type;
	int	size;
	bool	startRT;			// ���� �ð�
	int		client_id;
	float	x, y, z;
};

struct SC_Packet_ZenithStage{
	char	type;
	int	size;
	bool	startZS;			// ���� ��������
};

struct InitMonsterInfo{
	int		monsterid;
	int		monstertype;
	float	x;
	float	y;
	float	z;
};

struct SC_Packet_InitMonster{
	char	type;
	int		size;
	InitMonsterInfo monsters[50];
};

struct SC_Packet_MonsterHP{
	char	type;
	int		size;
	int		monsterID;
	int		monsterHP;
};

struct SC_Packet_DropItem{
	char	type;
	int		size;
	int		item;
	int		itemNum;
	float	x;
	float	y;
	float	z;
};

struct SC_Packet_Gold{
	char	type;
	int		size;
	int		gold;
};

struct SC_Packet_Inventory{
	char	type;
	int		size;
	int		item;	// 1.�� / 2.������ / 3.���� / 4.���� ������ / 5.������ ������ / 6.����Ŀ ������ 
	int		num;
};

struct SC_Packet_SelectItem{
	char	type;
	int		size;
	int		item;
};

struct SC_Packet_ItemState{
	char	type;
	int		size;
	int		result;
};

struct SC_Packet_DebugItem{
	char	type;
	int		size;
	int		item;
	int		itemNum;
};

struct SC_Packet_Animaition{
	char	type;
	int		size;
	int		client_id;
	int		animation;
};

struct SC_Packet_Chat{
	char	type;
	int		size;
	char	msg[256];
};
#pragma pack(pop)