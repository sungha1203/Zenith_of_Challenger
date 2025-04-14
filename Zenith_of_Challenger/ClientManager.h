#pragma once
#include "stdafx.h"

enum class Classtype
{
	CHALLENGER,					// 도전자
	WARRIOR,					// 전사
	MAGE,						// 마법사
	HEALTANKER					// 힐탱커
};

struct Weapon
{
	int		type;				// 무기 종류 (1.검 / 2.지팡이 / 3. 방패)
	int		level;				// 무기 강화 레벨 (1~9)
};

struct Player
{
	Classtype		classtype;				// 직업 
	Weapon			weapon;					// 무기
	int				clothes[3] = {0,0,0};	// 옷종류
	int				hp;						// 체력
	int				attack;					// 공격력
	int				speed;					// 이동 속도
	int				attackspeed;			// 공격 속도
	float			x, y, z;				// 인게임 캐릭터 좌표
	float			dir;					// 방향
};

class ClientInfo       
{
private:
	Player	m_ingameInfo;			// 인게임 정보 (1.직업 / 2.무기 / 3.옷종류 / 4.체력 / 5.공격력 / 6.이동속도 / 7.공격속도 / 8.좌표)
	int		m_id;					// 클라이언트 ID
	int		m_roomNum;				// 입장 방 번호

public:
	ClientInfo();
	ClientInfo(int client_id, int roomNum);		// 초기 캐릭터 설정
	~ClientInfo();

	int GetID() const { return m_id; }

	void SetRoomNum(const int room_id);									// 방 선택
	void SetClothes(const int clothes[3]);								// 옷 선택
	void SetCoord(float x, float y, float z);													// 좌표

	void LeverUpPlayer(Player& player);									// 직업 전직 후 캐릭터 설정
	int GetHP()const { return m_ingameInfo.hp; }						// 캐릭터 HP 반환
	int GetAttack() const { return m_ingameInfo.attack; }				// 캐릭터 공격력 반환
	int GetSpeed() const { return m_ingameInfo.speed; }					// 캐릭터 이동속도 반환
	int GetAttackSpeed() const { return m_ingameInfo.attackspeed; }		// 캐릭터 공격속도 반환
};