#pragma once
#pragma pack(push,1)

constexpr int SERVER_PORT = 4000;
constexpr int WORKER_THREAD_COUNT = 4;

constexpr int BUF_SIZE = 1024;
constexpr int MAX_USER = 10;				// 접속 가능한 총 유저 수
constexpr int NAME_SIZE = 20;				// 이름 글자 수

// 패킷 타입 정의 (클라 -> 서버)
#define CS_PACKET_LOGIN				 1			// 로그인 요청
#define CS_PACKET_ROOM				 2			// 게임방 요청
#define CS_PACKET_CUSTOMIZE			 3			// 커스터마이징
#define CS_PACKET_UPDATEPLAYER		 4			// 플레이어 상태 업데이트
#define CS_PACKET_GAMESTART			 5			// 게임 시작 버튼
#define CS_PACKET_INGAMEREADY		 6			// 게임 시작 후 입장 완료
#define CS_PACKET_STARTZENITH		 7			// 정점 스테이지 입장 완료
#define CS_PACKET_LOGOUT			 100		// 로그아웃

// 패킷 타입 정의 (서버 -> 클라)
#define SC_PACKET_LOGIN_RESPONSE	 101		// 로그인 응답
#define SC_PACKET_ROOM_RESPONSE		 102		// 게임방 응답
#define SC_PACKET_ROOMLIST			 103		// 게임방 인원수 갱신
#define SC_PACKET_UPDATE2PlAYER		 104		// 다른 플레이어들한테 내 상태 갱신
#define SC_PACKET_GAMESTART			 105		// 게임 시작
#define SC_PACKET_REPAIRTIME		 106		// 정비 시간
#define SC_PACKET_ZENITHSTAGE		 107		// 도전 -> 정점
#define SC_PACKET_LOGOUT			 110		// 로그아웃

// [임시]
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

struct CS_Packet_UpdatePlayer
{
	float x, y, z;  // 위치
	float dir;      // 방향
	int animState;  // 애니메이션 상태
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
	bool	success;			// 로그인 성공 여부
};

struct SC_Packet_RoomResponse
{
	char	type;
	UCHAR	size;
	bool	success;			// 게임방 입장 성공 여부
	int		room_id;			// 방번호
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
	int		room_id;			// 게임방 넘버
	int		current_players;	// 현재 플레이어 수
};

struct SC_Packet_Update2Player
{
	int client_id;
	float x, y, z;
	float dir;
	int animState;
};

struct SC_Packet_GameStart
{
	char	type;
	UCHAR	size;
	bool	startCS;			// 도전 스테이지(게임 시작)
};

struct SC_Packet_RepairTime
{
	char	type;
	UCHAR	size;
	bool	startRT;			// 정비 시간
};

struct SC_Packet_ZenithStage
{
	char type;
	UCHAR	size;
	bool	startZS;			// 정점 스테이지
};

// [임시]
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