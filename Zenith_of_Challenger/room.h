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

struct InventoryItem								// 공통 인벤토리
{
	int gold = 0;									// 재화
	std::map<JobWeapon, int> JobWeapons;			// 무기
	std::map<JobDocument, int> JobDocuments;		// 전직서

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

// 게임 진행, 스테이지 변경, 클라이언트 관리
class Room
{
private:
	bool				m_IsGaming;					// 게임중인지 확인
	int					m_room_id;					// 몇번째 방인지
	Stage				m_RoomState;				// 현재 게임방 상태(로비, 도전, 정비시간, 정점)
	std::vector<int>	m_clients;					// 입장한 클라이언트 ID들
	InventoryItem		m_inventory;				// 아이템 인벤토리 창

	std::atomic<bool>	m_stopTimer = false;		// 클라이언트 모두 접속을 끊었을 때를 대비
	std::atomic<bool>	m_skipTimer = false;		// 도전스테이지 스킵을 눌렀을 때 -> true
	std::atomic<bool>	m_skipButton = false;		// 스킵 버튼 두번 이상 못누름
	std::thread			m_timer;
	std::thread			m_ZmonsterPosTimer;
	std::atomic<bool>	m_stopMonsterPosThread = false;
	int					m_playerNum = 0;			// 게임 시작하는 총 플레이어 인원수
	int					m_enterClientNum = 0;		// 도전 스테이지에 입장한 클라이언트 명수 (다 들어와야지 타이머 시작)
	int					m_enterZenithNum = 0;		// 정점스테이지 입장 대기 명수 (다 들어와야지 타이머 시작)
	bool				m_bossDie = false;			// 보스 몬스터 처치
	int					m_clearTime = 0;			// 보스 몬스터 클리어 타임

	std::mutex			m_PlayerMx;
	std::mutex			m_inventoryMx;

	static constexpr int CHALLENGE_TIME = 480;		// 8분
	static constexpr int ZENITH_TIME	= 300;		// 5분
	
	std::array<Monster,50>	m_Cmonsters;			// 방에서 관리하는 일반몬스터(도전)
	std::array<Monster,26>	m_Zmonsters;			// 방에서 관리하는 일반+보스몬스터(정점)
	int						m_CMonsterNum;			// 일반몬스터 수(도전)
	int						m_ZMonsterNum;			// 일반몬스터 수(정점)

	std::vector<PlayerInfo> m_PlayerCoord;			// 각 플레이어의 현재 좌표
	std::chrono::steady_clock::time_point m_UpdatelastAggro;
	const float AGGRO_N_TARGET_UPDATE_TIME = 3.0f;	// 3초마다 어그로, 바라보는 방향 리스트 갱신

public:
	Room() : m_room_id(-1), m_IsGaming(false), m_RoomState(Stage::LOBBY) {}

	void    Init(int room_id);							// 방 초기화
	void	ResetRoom();								// 방 클리어

	void	AddClient(int client_id);					// 클라 접속
	void    RemoveClient(int client_id);				// 클라 접속 해제

	void	PushStartGameButton(int RoomMasterID);		// 게임 시작 시도
	void	PushStartZenithButton(int RoomMasterID);	// 방장이 모든 플레이어 정점으로 이동 시도

	void    AllPlayerNum(int playerNum);				// 총 플레이어 인원수
	void    PlusPlayerReadyNum();						// 플레이어 입장 ++1
	void	PlusPlayerZenithReadyNum();					// 정점 스테이지 입장 대기 명수 ++1
	void    ChallengeTimerThread();						// 도전 스테이지 타이머 스레드
	void	ZenithTimerThread();						// 정점 스테이지 타이머 스레드
	void	m_ZmonsterPosTimerThread();						// 정점 스테이지 몬스터 좌표 갱신 스레드
	void	StartGame();								// 로비			-> 도전스테이지
	void	RepairTime();								// 도전 스테이지 내 재정비 시간
	void	StartZenithStage();							// 도전 스테이지 -> 정점스테이지
	void	EndGame();									// 정점 스테이지 -> 로비
	
	void	UpdateMonsterTargetList();					// 몬스터가 현재 바라보고 있는 플레이어 리스트 전달
	void	UpdateMonsterAggroList();					// 몬스터의 어그로 리스트 갱신 및 전달

	void	SetStopTimer(bool check);
	void	SetSkipTimer(bool check);
	void	SetSkipButton(bool check);
	void	SetClearBoss();

	int		GetClientsNum() const { return m_clients.size(); }								// 이 방에 몇명있어?
	int		GetRoomMasterID() const { return m_clients.empty() ? -1 : m_clients.front(); }	// 방장 누구야?
	int		GetPlayerNum() const { return m_playerNum; }									// 게임 시작할때 몇명에서 시작했어?
	int		GetEnterClientNum() const { return m_enterClientNum; }							// 지금 몇명이 게임 대기중이야?
	int		GetEnterZenithNum() const { return m_enterZenithNum;  }							// 지금 몇명이 정점 스테이지 대기중이야?
	bool	GetSkipButton() const { return m_skipButton;  }									// 스킵 버튼 눌렀어 안눌렀어?
	int		GetMode() const { return (int)m_RoomState; }									// 지금 무슨 스테이지야?
	int		GetGold() const { return m_inventory.gold; }									// 골드 얼마있어?
	int		GetWeaponTypeNum(int num) const { return m_inventory.JobWeapons.at(static_cast<JobWeapon>(num-1));}			// 해당 무기 몇개 있어?
	int		GetJobTypeNum(int num) const { return m_inventory.JobDocuments.at(static_cast<JobDocument>(num-4));}		// 해당 전직서 몇개 있어?

	const std::vector<int>& GetClients() const { return m_clients; }						// 게임 중인 모든 클라이언트
	const Monster&	GetCMonsters(int monsterID) const { return m_Cmonsters[monsterID]; }		// 도전 스테이지 몬스터
	const Monster&	GetZMonsters(int monsterID) const { return m_Zmonsters[monsterID]; }		// 정점 스테이지 몬스터
	Monster& GetCMonster(int monsterID) { return m_Cmonsters[monsterID]; }
	Monster& GetZMonster(int monsterID) { return m_Zmonsters[monsterID]; }

	// 인벤토리
	void	AddGold(int plusgold);					// +골드
	void    SpendGold(int minusgold);				// -골드
	void	ADDJobWeapon(int weapon);				// +무기
	void	DecideJobWeapon(int weapon);			// -무기
	void	AddJobDocument(int job);				// +전직서
	void	DecideJobDocument(int job);				// -전직서

	// 몬스터
	void	InitChallengeMonsters();				// 도전 스테이지 몬스터 초기화
	void	InitZenithMonsters();					// 정점 스테이지 몬스터 초기화(보스 몬스터 포함)
	void	InitZMonsterFirstLastCoord();			// 정점 스테이지 몬스터 기존 루트 좌표 설정
	void	BroadcastMonsterPosition(int idx);		// 정점 스테이지 몬스터 일정 시간 간격으로 좌표 갱신
};