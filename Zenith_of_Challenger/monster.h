// 몬스터 개별 행동
#include "stdafx.h"

enum class NormalMonsterType
{
	Mushroom,
	FightFly,
	PlantDionaea,
	PlantVenus,
	FlowerFairy,
	BossMonster
};

enum class DropItemType {
	None,
	SWORD, WAND, SHIELD,
	WARRIOR, MAGE, HEALTANKER
};

struct AggroInfo
{
	int		playerID = -1;		// 플레이어의 아이디
	float	distance = 0.0f;	// 해당 플레이어와의 거리 : 가장 가까운 플레이어을 타깃하기 위함
};

class Monster
{
public:
	Monster() = default;
	~Monster();

	std::mutex mx;

	void			Reset();
	void			SetMonster(int id, NormalMonsterType type, float x, float y, float z);
	void			TakeDamage(int dmg);
	void			AIMove();
	void			UpdateAggroList(const std::vector<PlayerInfo>& players);
	DropItemType	DropWHAT();

	NormalMonsterType		GetType() const { return m_type; }
	bool					GetLived() const { return m_islived; }
	float					GetX() const { return m_x; }
	float					GetY() const { return m_y; }
	float					GetZ() const { return m_z; }
	int						GetHP() const { return m_hp; }
	
private:
	int					m_version;				// 도전 - 0, 정점 - 1
	int					m_id;					// 고유 ID
	int					m_islived = true;		// 살아있는지
	NormalMonsterType	m_type;					// 몬스터 종류
	float				m_x, m_y, m_z;			// 좌표
	int					m_hp;					// 현재	체력
	int					m_attack;				// 공격력
	int					m_speed;				// 이동속도
	int					m_attackspeed;			// 공격속도

	std::vector<AggroInfo> m_AggroList;			// 어그로 리스트
	bool				m_aggro = false;		// 어그로 됐는지(정점스테이지에서만 사용)
	float				m_aggroRange = 5.f;		// 어그로 범위
	int					m_aggroplayer = -1;		// 현재 어그로 끌린 플레이어의 id
};