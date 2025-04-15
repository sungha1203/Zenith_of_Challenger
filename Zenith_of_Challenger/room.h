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

struct InventoryItem								// 공통 인벤토리
{
	int gold = 0;									// 재화
	std::map<JobWeapon, int> JobWeapons;			// 무기
	std::map<JobDocument, int> JobDocuments;		// 전직서
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
	std::thread			m_timer;
	int					m_playerNum = 0;			// 게임 시작하는 총 플레이어 인원수
	int					m_enterClientNum = 0;		// 도전 스테이지에 입장한 클라이언트 명수 (다 들어와야지 타이머 시작)
	int					m_enterZenithNum = 0;		// 정점스테이지 입장 대기 명수 (다 들어와야지 타이머 시작)
	std::mutex			m_PlayerMx;
	std::mutex			m_inventoryMx;

	static constexpr int CHALLENGE_TIME = 480;		// 8분
	//static constexpr int REPAIR_TIME	= 120;		// 2분
	static constexpr int ZENITH_TIME	= 300;		// 5분
	 

public:
	Room() : m_room_id(-1), m_IsGaming(false), m_RoomState(Stage::LOBBY) {}

	void    Init(int room_id);							// 방 초기화

	void	AddClient(int client_id);					// 클라 접속
	void    RemoveClient(int client_id);				// 클라 접속 해제

	void	PushStartGameButton(int RoomMasterID);		// 게임 시작 시도
	void	PushStartZenithButton(int RoomMasterID);	// 방장이 모든 플레이어 정점으로 이동 시도

	void    AllPlayerNum(int playerNum);				// 총 플레이어 인원수
	void    PlusPlayerReadyNum();						// 플레이어 입장 ++1
	void	PlusPlayerZenithReadyNum();					// 정점 스테이지 입장 대기 명수 ++1
	void    ChallengeTimerThread();						// 도전 스테이지 타이머 스레드
	void	ZenithTimerThread();						// 정점 스테이지 타이머 스레드
	void	StartGame();								// 로비			-> 도전스테이지
	void	RepairTime();								// 도전 스테이지 내 재정비 시간
	void	StartZenithStage();							// 도전 스테이지 -> 정점스테이지
	void	EndGame();									// 정점 스테이지 -> 로비
	
	int		GetClientsNum() const { return m_clients.size(); }								// 이 방에 몇명있어?
	int		GetRoomMasterID() const { return m_clients.empty() ? -1 : m_clients.front(); }	// 방장 누구야?
	int		GetPlayerNum() const { return m_playerNum; }									// 게임 시작할때 몇명에서 시작했어?
	int		GetEnterClientNum() const { return m_enterClientNum; }							// 지금 몇명이 게임 대기중이야?
	int		GetEnterZenithNum() const { return m_enterZenithNum;  }							// 지금 몇명이 정점 스테이지 대기중이야?
	const std::vector<int>& GetClients() const { return m_clients; }						// 게임 중인 모든 클라이언트

	// 인벤토리
	void	AddGold(int plusgold);				// +골드
	void    SpendGold(int minusgold);			// -골드
	void	ADDJobWeapon(JobWeapon weapon);		// +무기
	void	DecideJobWeapon(JobWeapon weapon);	// -무기
	void	AddJobDocument(JobDocument job);	// +전직서
	void	DecideJobDocument(JobDocument job);	// -전직서
};