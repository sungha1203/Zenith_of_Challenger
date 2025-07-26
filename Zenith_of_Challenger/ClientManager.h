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
	int		type;				// 무기 종류 (0. 맨손 / 1.검 / 2.지팡이 / 3. 방패)
	int		level;				// 무기 강화 레벨 (0~9)
};

struct Player
{
	int				clientID = -1;			// 아이디
	Classtype		classtype;				// 직업 
	Weapon			weapon;					// 무기
	int				clothes = 0;			// 옷종류
	int				maxhp;					// 최대 체력
	int				hp;						// 현재 체력
	int				attack;					// 공격력
	int				speed;					// 이동 속도
	int				attackspeed;			// 공격 속도
	float			x, y, z;				// 인게임 캐릭터 좌표
	float			angle;					// 방향
};

// 몬스터와 어그로 상태를 알기위한 구조체
struct PlayerInfo
{
	int		clientID;
	float	x, z;
};

class ClientInfo       
{
private:
	Player	m_ingameInfo;			// 인게임 정보 (1.직업 / 2.무기 / 3.옷종류 / 4.체력 / 5.공격력 / 6.이동속도 / 7.공격속도 / 8.좌표)
	int		m_normalAttack = 0;		// 정점 스테이지 평타 공격력
	int		m_skillAttack = 0;		// 정점 스테이지 스킬 공격력

	std::chrono::steady_clock::time_point m_lastSkillTime;

public:
	ClientInfo();
	ClientInfo(int client_id);		// 초기 캐릭터 설정
	~ClientInfo();

	void	SetClothes(const int clothes);									// 옷 선택
	void	SetJobType(int JobNum);											// 직업 선택
	void	SetWeapon(int weaponNum);										// 무기 선택
	void	SetWeaponGrade();												// 무기 강화
	bool	SetEnhanceGradeUp(int weapongrade);								// 무기 강화 로직(성공 여부 반환)
	void	SetSpawnCoord(int idx);											// 초기 스폰 좌표
	void	SetRepairCoord(int idx);										// 시작의 땅 스폰 좌표
	void	SetZenithCoord(int idx);										// 정점 스테이지 스폰 좌표
	void	SetCoord(float x, float y, float z);							// 좌표 갱신
	void	SetAngle(float angle);											// 방향 갱신
	void	SetAttack();													// 정점 들어가기 전에 공격력 설정
	
	void	LeverUpPlayer(int classtype);									// 직업 전직 후 캐릭터 설정

	void	AddHP(int heal);												// 피 회복
	void	MinusHP(int damage, int gamemode);								// 피 깎임

	Classtype GetJobType() const { return m_ingameInfo.classtype; }			// 캐릭터 직업 반환
	int		GetWeaponType() const { return m_ingameInfo.weapon.type; }		// 캐릭터 무기 반환
	int		GetWeaponGrade() const { return m_ingameInfo.weapon.level; }	// 캐릭터 무기 등급 반환
	int		GetID() const { return m_ingameInfo.clientID;  }				// 캐릭터 아이디 반환
	float	GetX() const { return m_ingameInfo.x; }							// 캐릭터 X좌표 반환
	float	GetY() const { return m_ingameInfo.y; }							// 캐릭터 y좌표 반환
	float	GetZ() const { return m_ingameInfo.z; }							// 캐릭터 z좌표 반환
	int		GetHP() const { return m_ingameInfo.hp; }						// 캐릭터 HP 반환
	int		GetNormalAttack() const { return m_normalAttack; }				// 캐릭터 평타 공격력 반환
	int		GetSkillAttack() const { return m_skillAttack; }				// 캐릭터 스킬 공격력 반환
	int		GetSpeed() const { return m_ingameInfo.speed; }					// 캐릭터 이동속도 반환
	int		GetAttackSpeed() const { return m_ingameInfo.attackspeed; }		// 캐릭터 공격속도 반환
	float	GetAngle() const { return m_ingameInfo.angle; }					// 캐릭터 방향 반환

	bool	CanUseSkill();													// 스킬 쿨타임이 찼는지
	void	StartCoolTime();												// 스킬 쿨타임 돌리기
};