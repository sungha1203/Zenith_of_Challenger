#pragma once
#include "stdafx.h"
#include "session.h"

class Monster;

class Network
{
public:
	Network();
	~Network();

	void		init();
	int			CreateClient();													// 클라이언트 idx 생성
	void		CloseClient(int client_id);										// 클라이언트 idx 삭제

	void		WorkerThread();													// 클라이언트 네트워크 이벤트 처리
	void		HandlePacket(int client_id, char* buffer, int length);			// 패킷 분석

	void		ProcessLogin(int client_id, char* buffer, int length);			// 로그인 성공 여부
	void		ProcessRoomJoin(int client_id, char* buffer, int length);		// 방 입장 성공 여부
	void		ProcessCustomize(int client_id, char* buffer, int length);		// 커스터마이징
	void		ProcessUpdatePlayer(int client_id, char* buffer, int length);	// 인게임 내 플레이어들한테 업데이트
	void		ProcessGameStartButton(int client_id);							// 게임 시작 버튼 누른 직후
	void		ProcessIngameReady(int client_id, char* buffer, int length);	// 도전 스테이지 입장 성공 여부
	void		ProcessZenithStartButton(int client_id);						// 정점 스테이지 입장 준비 완료 버튼 누른 직후
	void		ProcessZenithReady(int client_id, char* buffer, int length);	// 정점 스테이지 입장 성공 여부

	// ---------패킷 뿌려주기---------
	void		SendLoginResponse(int client_id, bool success);
	void		SendRoomJoinResponse(int client_id, bool success, int room_id); // 클라한테 방 입장 성공여부 뿌려주기
	void		SendRoomInfo(int room_id, int client_num);						// 방 명수 관리
	void		SendGameStart(const std::vector<int>& client_id);				// 게임 시작 버튼 누른 직후
	void		SendInitialState(const std::vector<int>& client_id);			// 다른 플레이어들한테 내 초기 상태
	void		SendStartRepairTime(const std::vector<int>& client_id);			// 정비 시간(8분 지나고 시작의 땅으로 이동)
	void		SendStartZenithStage(const std::vector<int>& client_id);		// 도전 -> 정점 스테이지
	void		SendUpdateInventory(const std::vector<int>& client_id);			// 인벤토리 업데이트
	void		SendInitMonster(const std::vector<int>& client_id, const std::unordered_map<int, Monster>& monsters);	// 몬스터 초기 좌표 설정
	// ---------패킷 뿌려주기---------

public:
	SOCKET										m_client;						// 클라이언트 소켓
	HANDLE										h_iocp;							// CompletionPort 객체 핸들

	std::vector<std::thread>					workers;						// 통신 스레드
	std::array<SESSION, MAX_USER>				clients;						// 클라이언트 (세션 관리)
};