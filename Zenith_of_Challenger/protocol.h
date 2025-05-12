#pragma once
#pragma pack(push,1)

constexpr int SERVER_PORT = 4000;

constexpr int BUF_SIZE = 1400;
constexpr int MAX_USER = 10;				// 접속 가능한 총 유저 수
constexpr int NAME_SIZE = 20;				// 이름 글자 수

// 패킷 타입 정의 (클라 -> 서버)
#define CS_PACKET_LOGIN				 1			// 로그인 요청
#define CS_PACKET_ROOM				 2			// 게임방 요청
#define CS_PACKET_CUSTOMIZE			 3			// 커스터마이징
#define CS_PACKET_UPDATECOORD		 4			// 좌표 상태 업데이트
#define CS_PACKET_GAMESTART			 5			// 게임 시작 버튼
#define CS_PACKET_INGAMEREADY		 6			// 게임 시작 후 입장 완료
#define CS_PACKET_STARTZENITH		 7			// 정점 스테이지 입장 완료
#define CS_PACKET_MONSTERHP			 8			// 몬스터 HP
#define CS_PACKET_CHAT				 9			// 인게임 속 채팅
#define CS_PACKET_ITEMSTATE			 10			// 장비창에서 강화
#define CS_PACKET_INVENTORY			 11			// 인벤토리에서 아이템 선택
#define CS_PACKET_DEBUGGOLD			 12			// 디버깅용 골드 추가
#define CS_PACKET_DEBUGITEM			 13			// 디버깅용 아이템 추가
#define CS_PACKET_ANIMATION			 14			// 애니메이션

#define CS_PACKET_SKIPCHALLENGE		 99			// 도전스테이지 스킵
#define CS_PACKET_LOGOUT			 100		// 로그아웃

// 패킷 타입 정의 (서버 -> 클라)
#define SC_PACKET_LOGIN_RESPONSE	 101		// 로그인 응답
#define SC_PACKET_ROOM_RESPONSE		 102		// 게임방 응답
#define SC_PACKET_ROOMLIST			 103		// 게임방 인원수 갱신
#define SC_PACKET_WHOISMYTEAM		 104		// 팀에 누구 있는지 말해줄게
#define SC_PACKET_INITIALSTATE		 105		// 다른 플레이어들한테 내 초기 상태
#define SC_PACKET_UPDATE2PLAYER		 106		// 다른 플레이어들한테 바뀐 좌표 갱신
#define SC_PACKET_GAMESTART			 107		// 게임 시작
#define SC_PACKET_REPAIRTIME		 108		// 정비 시간
#define SC_PACKET_ZENITHSTAGE		 109		// 도전 -> 정점
#define SC_PACKET_INITMONSTER		 110		// 몬스터 초기 설정
#define SC_PACKET_MONSTERHP			 111		// 몬스터 HP
#define SC_PACKET_DROPITEM			 112		// 몬스터 드랍 아이템
#define SC_PACKET_GOLD				 113		// 골드 현 상황 갱신
#define SC_PACKET_INVENTORY			 114		// 인벤토리 현재 상황
#define SC_PACKET_SELECTITEM		 115		// 인벤토리에서 무기 or 전직서 결정
#define SC_PACKET_ITEMSTATE			 116		// 무기 강화 성공 여부
#define SC_PACKET_CHAT				 117		// 인게임 속 채팅
#define SC_PACKET_DEBUGITEM			 118		// 디버깅용 아이템 추가
#define SC_PACKET_ANIMATION			 119		// 애니메이션

#define SC_PACKET_SKIPCHALLENGE		 998		// 도전스테이지 스킵
#define SC_PACKET_LOGOUT			 999		// 로그아웃

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
	float	x, y, z;  // 위치
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
	int		item;	// 1.검 / 2.지팡이 / 3.방패 / 4.전사 전직서 / 5.마법사 전직서 / 6.힐탱커 전직서 
};

struct CS_Packet_ItemState{
	char	type;
	int		size;
	bool	enhanceTry;		// 강화 시도
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
	bool	success;			// 로그인 성공 여부
	int		clientID;			// 고유 클라이언트 ID
};

struct SC_Packet_RoomResponse{
	char	type;
	int	size;
	bool	success;			// 게임방 입장 성공 여부
	int		room_id;			// 방번호
};

struct SC_Packet_Logout{
	char	type;
	int	size;
};

struct SC_Packet_RoomList{
	char	type;
	int	size;
	int		room_id;			// 게임방 넘버
	int		current_players;	// 현재 플레이어 수
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
	bool	startCS;			// 도전 스테이지(게임 시작)
};

struct SC_Packet_RepairTime{
	char	type;
	int	size;
	bool	startRT;			// 정비 시간
	int		client_id;
	float	x, y, z;
};

struct SC_Packet_ZenithStage{
	char	type;
	int	size;
	bool	startZS;			// 정점 스테이지
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
	int		item;	// 1.검 / 2.지팡이 / 3.방패 / 4.전사 전직서 / 5.마법사 전직서 / 6.힐탱커 전직서 
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