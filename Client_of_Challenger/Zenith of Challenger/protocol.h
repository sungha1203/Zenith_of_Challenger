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
#define CS_PACKET_STARTZENITH		 7			// ���� �������� ���� ��ư
#define CS_PACKET_ZENITHREADY		 8			// ���� �������� ���� �Ϸ�
#define CS_PACKET_SKIPCHALLENGE		 9			// ������������ ��ŵ
#define CS_PACKET_MONSTERHP			 10			// ���� ���� HP
#define CS_PACKET_ZMONSTERHP		 11			// ���� ���� HP
#define CS_PACKET_CHAT				 12			// �ΰ��� �� ä��
#define CS_PACKET_ITEMSTATE			 13			// ���â���� ��ȭ
#define CS_PACKET_INVENTORY			 14			// �κ��丮���� ������ ����
#define CS_PACKET_DEBUGGOLD			 15			// ������ ��� �߰�
#define CS_PACKET_DEBUGITEM			 16			// ������ ������ �߰�
#define CS_PACKET_ANIMATION			 17			// �ִϸ��̼�
#define CS_PACKET_ATTACKEFFECT		 18			// ��ų, �⺻ ���� ����Ʈ(����, ������)
#define CS_PACKET_DAMAGED			 19			// ���͵�����	

#define CS_PACKET_LOGOUT			 100		// �α׾ƿ�

// ��Ŷ Ÿ�� ���� (���� -> Ŭ��)
#define SC_PACKET_LOGIN_RESPONSE	 51		// �α��� ����
#define SC_PACKET_ROOM_RESPONSE		 52		// ���ӹ� ����
#define SC_PACKET_CUSTOMIZE			 53		// Ŀ���͸���¡
#define SC_PACKET_ROOMLIST			 54		// ���ӹ� �ο��� ����
#define SC_PACKET_WHOISMYTEAM		 55		// ���� ���� �ִ��� �����ٰ�
#define SC_PACKET_INITIALSTATE		 56		// ���� - �ٸ� �÷��̾������ �� �ʱ� ����
#define SC_PACKET_ZENITHSTATE		 57		// ���� - �ٸ� �÷��̾������ �� �ʱ� ����
#define SC_PACKET_UPDATE2PLAYER		 58		// �ٸ� �÷��̾������ �ٲ� ��ǥ ����
#define SC_PACKET_GAMESTART			 59		// ���� ����
#define SC_PACKET_REPAIRTIME		 60		// ���� �ð�
#define SC_PACKET_ZENITHSTAGE		 61		// ���� -> ����
#define SC_PACKET_INITMONSTER		 62		// ���� ���� �ʱ� ����
#define SC_PACKET_ZENITHMONSTER		 63		// ���� ���� �ʱ� ����
#define SC_PACKET_MONSTERHP			 64		// ���� ���� HP
#define SC_PACKET_ZMONSTERHP         65		// ���� ���� HP
#define SC_PACKET_DROPITEM			 66		// ���� ��� ������
#define SC_PACKET_GOLD				 67		// ��� �� ��Ȳ ����
#define SC_PACKET_INVENTORY			 68		// �κ��丮 ���� ��Ȳ
#define SC_PACKET_SELECTITEM		 69		// �κ��丮���� ���� or ������ ����
#define SC_PACKET_ITEMSTATE			 70		// ���� ��ȭ ���� ����
#define SC_PACKET_CHAT				 71		// �ΰ��� �� ä��
#define SC_PACKET_DEBUGITEM			 72		// ������ ������ �߰�
#define SC_PACKET_ANIMATION			 73		// �ִϸ��̼�
#define SC_PACKET_CMONSTERTARGET	 74		// ������������ ���Ͱ� �ٶ󺸴� ����
#define SC_PACKET_ZMONSTERTARGET	 75		// ������������ ���Ͱ� �ٶ󺸴� ����
#define SC_PACKET_RESPONE			 76		// ����or���� ���� �� ������
#define SC_PACKET_ZMONSTERMOVE		 77		// ���� �������� ������ �̵�
#define SC_PACKET_ZMONSTERATTACK	 78		// ���� �������� ������ ����
#define SC_PACKET_ATTACKEFFECT		 79		// ��ų, �⺻ ���� ����Ʈ(����, ������)
#define SC_PACKET_PLAYERHP			 80		// �÷��̾� ü�� ������Ʈ
#define SC_PACKET_ENDGAME			 81		// ���� ����

#define SC_PACKET_LOGOCHUT			 999		// �α׾ƿ�

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
	int	size;
	UCHAR	room_id;
};

struct CS_Packet_Customize
{
	char	type;
	int		size;
	int		clothes;
};

struct CS_Packet_UPDATECOORD
{
	char	type;
	int		size;
	float	x, y, z;  // ��ġ
	float	angle;
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

struct CS_Packet_StartZenith
{
	char	type;
	int		size;
};

struct CS_Packet_ZenithReady
{
	char	type;
	int		size;
	bool	ReadySuccess;
};

struct CS_Packet_Chat
{
	char	type;
	int		size;
	char	msg[256];
};

struct CS_Packet_SkipChallenge
{
	char	type;
	int		size;
	bool	skip;
};

struct CS_Packet_MonsterHP
{
	char	type;
	int		size;
	int		monsterID;
	int		damage;
};

struct CS_Packet_ZMonsterHP
{
	char	type;
	int		size;
	int		monsterID;
	int		damage;
};

struct CS_Packet_Inventory
{
	char	type;
	int		size;
	int		item;	// 1.�� / 2.������ / 3.���� / 4.���� ������ / 5.������ ������ / 6.����Ŀ ������ 
};

struct CS_Packet_ItemState
{
	char	type;
	int		size;
	bool	enhanceTry;		// ��ȭ �õ�
};

struct CS_Packet_DebugGold
{
	char	type;
	int		size;
	bool	plusGold;
};

struct CS_Packet_DebugItem
{
	char	type;
	int		size;
	int		item;
};

struct CS_Packet_Animaition
{
	char type;
	int size;
	int animation;
};

struct CS_Packet_AttackEffect
{
	char	type;
	int		size;
	int		skill;   // 0. ���� �⺻ ���� ����Ʈ,    1. ���� ��ų ���� ����Ʈ,    2. ������ �⺻ ���� ����Ʈ,    3. ������ ��ų ���� ����Ʈ
};

struct CS_Packet_Logout
{
	char	type;
	int		size;
};

struct CS_Packet_HealPack
{
	char	type;
	int		size;
	bool	eat;
};

struct CS_Packet_Damaged
{
	char	type;
	int		size;
	int		monsterID;
};

//--------------------------------------------------------------------------------------------------------------------------------------

struct SC_Packet_LoginResponse
{
	char	type;
	int	size;
	bool	success;			// �α��� ���� ����
	int		clientID;			// ���� Ŭ���̾�Ʈ ID
};

struct SC_Packet_RoomResponse
{
	char	type;
	int	size;
	bool	success;			// ���ӹ� ���� ���� ����
	int		room_id;			// ���ȣ
};

struct SC_Packet_Customize
{
	char	type;
	int		size;
	int		clientID;
	int		clothes;
};

struct SC_Packet_Logout
{
	char	type;
	int	size;
};

struct SC_Packet_RoomList
{
	char	type;
	int	size;
	int		room_id;			// ���ӹ� �ѹ�
	int		current_players;	// ���� �÷��̾� ��
};

struct SC_Packet_MyTeam
{
	char	type;
	int		size;
	int		teamID[3];
	int		teamSize;
};

struct SC_Packet_initialstate
{
	char	type;
	int		size;
	int		client_id;
	float	x, y, z;
};

struct SC_Packet_Zenithstate
{
	char	type;
	int		size;
	int		client_id;
	float	x, y, z;
};

struct SC_Packet_Update2Player
{
	char	type;
	int		size;
	int		client_id;
	float	x, y, z;
	float   angle;
};

struct SC_Packet_GameStart
{
	char	type;
	int	size;
	bool	startCS;			// ���� ��������(���� ����)
};

struct SC_Packet_RepairTime
{
	char	type;
	int	size;
	bool	startRT;			// ���� �ð�
	int		client_id;
	float	x, y, z;
};

struct SC_Packet_ZenithStage
{
	char	type;
	int	size;
	bool	startZS;			// ���� ��������
};

struct InitMonsterInfo
{
	int		monsterid;
	int		monstertype;
	float	x;
	float	y;
	float	z;
};

struct SC_Packet_InitMonster
{
	char	type;
	int		size;
	InitMonsterInfo monsters[50];
};

struct SC_Packet_ZenithMonster
{
	char	type;
	int		size;
	InitMonsterInfo monsters[50];
};

struct SC_Packet_MonsterHP
{
	char	type;
	int		size;
	int		monsterID;
	int		monsterHP;
};

struct SC_Packet_ZMonsterHP
{
	char	type;
	int		size;
	int		monsterID;
	int		monsterHP;
};

struct SC_Packet_DropItem
{
	char	type;
	int		size;
	int		item;
	int		itemNum;
	float	x;
	float	y;
	float	z;
};

struct SC_Packet_Gold
{
	char	type;
	int		size;
	int		gold;
};

struct SC_Packet_Inventory
{
	char	type;
	int		size;
	int		item;	// 1.�� / 2.������ / 3.���� / 4.���� ������ / 5.������ ������ / 6.����Ŀ ������ 
	int		num;
};

struct SC_Packet_SelectItem
{
	char	type;
	int		size;
	int		item;
	int		clientID;
};

struct SC_Packet_ItemState
{
	char	type;
	int		size;
	int		result;
};

struct SC_Packet_DebugItem
{
	char	type;
	int		size;
	int		item;
	int		itemNum;
};

struct SC_Packet_Animaition
{
	char	type;
	int		size;
	int		client_id;
	int		animation;
};

struct SC_Packet_AttackEffect
{
	char	type;
	int		size;
	int		targetID;
	int		skill;		// 0. ���� �⺻ ���� ����Ʈ,    1. ���� ��ų ���� ����Ʈ,    2. ������ �⺻ ���� ����Ʈ,    3. ������ ��ų ���� ����Ʈ
	float	angle;
};

struct SC_Packet_Chat
{
	char	type;
	int		size;
	char	msg[256];
};

struct SC_Packet_CMonsterTarget
{
	char	type;
	int		size;
	int		monsterID;
	int		targetID;
};

struct SC_Packet_ZMonsterTarget
{
	char	type;
	int		size;
	int		monsterID;
	int		targetID;
};

struct SC_Packet_Respone
{
	char	type;
	int		size;
	int		clientID;
	float	x, y, z;
};

struct SC_Packet_ZMonsterMove
{
	char	type;
	int		size;
	int		monsterID;
	float	x, y, z;
	float   targetX, targetY, targetZ;
};

struct SC_Packet_ZMonsterAttack
{
	char	type;
	int		size;
	int		monsterID;
	int		bossmonsterSkill;
};

struct SC_Packet_PlayerHP
{
	char	type;
	int		size;
	int		hp;
};

struct SC_Packet_EndGame
{
	char	type;
	int		size;
	int		time;
};

#pragma pack(pop)