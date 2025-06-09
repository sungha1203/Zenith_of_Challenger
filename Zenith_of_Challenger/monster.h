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

class Monster
{
public:
	Monster() = default;
	~Monster();

	std::mutex mx;

	void			Reset();
	void			SetMonster(int id, NormalMonsterType type, float x, float y, float z);
	void			TakeDamage(int dmg);
	DropItemType	DropWHAT();

	NormalMonsterType		GetType() const { return m_type; }
	float					GetX() const { return m_x; }
	float					GetY() const { return m_y; }
	float					GetZ() const { return m_z; }
	int						GetHP() const { return m_hp; }
	
private:
	int					m_id;					// 고유 ID
	int					m_islived = true;		// 살아있는지
	NormalMonsterType	m_type;					// 몬스터 종류
	float				m_x, m_y, m_z;			// 좌표
	int					m_hp;					// 현재	체력
	int					m_attack;				// 공격력
	int					m_speed;				// 이동속도
	int					m_attackspeed;			// 공격속도
};