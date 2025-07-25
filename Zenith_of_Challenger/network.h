#pragma once
#include "stdafx.h"
#include "session.h"

class Monster;

class Network{
public:
	Network();
	~Network();

	void			init();
	int				CreateClient();														// 클라이언트 idx 생성
	static void		CloseClient(int client_id);											// 클라이언트 idx 삭제

	void			WorkerThread();														// 클라이언트 네트워크 이벤트 처리
	static void		HandlePacket(int client_id, char * buffer, int length);				// 패킷 분석

	static void		ProcessLogin(int client_id, char * buffer, int length);				// 로그인 성공 여부
	static void		ProcessRoomJoin(int client_id, char * buffer, int length);			// 방 입장 성공 여부
	static void		ProcessCustomize(int client_id, char * buffer, int length);			// 커스터마이징
	static void		ProcessUpdatePlayer(int client_id, char * buffer, int length);		// 인게임 내 플레이어들한테 업데이트
	static void		ProcessGameStartButton(int client_id);								// 게임 시작 버튼 누른 직후
	static void		ProcessIngameReady(int client_id, char * buffer, int length);		// 도전 스테이지 입장 성공 여부
	static void     ProcessSkipChallenge(int client_id, char * buffer, int length);		// 도전 스테이지 스킵(정비 시간 입장)
	static void		ProcessZenithStartButton(int client_id);							// 정점 스테이지 입장 준비 완료 버튼 누른 직후
	static void		ProcessZenithReady(int client_id, char * buffer, int length);		// 정점 스테이지 입장 성공 여부
	static void		ProcessChat(int client_id, char * buffer, int length);				// 인게임 속 채팅
	static void		ProcessMonsterHP(int client_id, char * buffer, int length);			// 도전 몬스터 HP 업데이트
	static void		ProcessZMonsterHP(int client_id, char* buffer, int length);			// 정점 몬스터 HP 업데이트
	static void		ProcessInventorySelcet(int client_id, char * buffer, int length);	// 인벤토리 무기 및 전직서 선택
	static void		ProcessItemState(int client_id, char * buffer, int length);			// 강화 및 전직 설정(장비창)
	static void		ProcessDebugGold(int client_id, char * buffer, int length);			// 디버깅용 골드 추가
	static void		ProcessDebugItem(int client_id, char * buffer, int length);			// 디버깅용 무기 및 전직서 추가
	static void		ProcessAnimation(int client_id, char * buffer, int length);			// 애니메이션
	static void		ProcessAttackEffect(int client_id, char* buffer, int length);		// 스킬, 기본 공격 이펙트(전사, 마법사)
	static void		ProcessEatHealPack(int client_id, char* buffer, int length);		// 힐팩먹기
	static void		ProcessDamaged(int client_id, char* buffer, int length);			// 몬스터한테 피해 입었을 때

	// ---------패킷 뿌려주기---------
	static void		SendLoginResponse(int client_id, bool success);
	static void		SendRoomJoinResponse(int client_id, bool success, int room_id);		// 클라한테 방 입장 성공여부 뿌려주기
	static void		SendRoomInfo(int room_id, int client_num);							// 방 명수 관리
	static void		SendGameStart(const std::vector<int> & client_id);					// 게임 시작 버튼 누른 직후
	static void		SendWhoIsMyTeam(const std::vector<int> & client_id);				// 아이디 값 보내주기
	static void		SendInitialState(const std::vector<int> & client_id);				// 도전 스테이지 내 초기 상태
	static void		SendZenithState(const std::vector<int>& client_id);					// 정점 스테이지 내 초기 상태
	static void		SendStartRepairTime(const std::vector<int> & client_id);			// 정비 시간(8분 지나고 시작의 땅으로 이동)
	static void		SendStartZenithStage(const std::vector<int> & client_id);			// 도전 -> 정점 스테이지
	static void     SendPlayerAttack(const std::vector<int>& client_id);				// 정점 스테이지 이동 전 공격력 설정
	static void		SendPlayerRespone(int client_id);									// 도전, 정점에서 피가 0이 되면 다시 태어나는 곳 설정
	static void		SendUpdateGold(const std::vector<int> & client_id);					// 골드 업데이트
	static void     SendPlayerHP(int client_id);										// 플레이어 체력 업데이트				
	static void		SendInitMonster(const std::vector<int> & client_id, const std::array<Monster, 50> & monsters);		// 도전 몬스터 초기 좌표 설정
	static void		SendZenithMonster(const std::vector<int> & client_id, const std::array<Monster, 26> & monsters);	// 정점 몬스터 초기 좌표 설정
	static void		SendZMonsterAttack(const std::vector<int>& client_id, int MonsterID, int SkillType);				// 정점 몬스터 공격
	static void		SendEndGame(const std::vector<int>& client_id, int time);											// 게임 종료

public:
	SOCKET										m_client;								// 클라이언트 소켓
	HANDLE										h_iocp;									// CompletionPort 객체 핸들

	std::vector<std::thread>					workers;								// 통신 스레드
	std::array<SESSION, MAX_USER>				clients;								// 클라이언트 (세션 관리)
};