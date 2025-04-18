// 몬스터 개별 행동
#pragma once
#include "stdafx.h"

enum class NormalMonsterType
{
	Mushroom,
	FightFly,
	PlantDionaea,
	PeaShooter,
	PlantVenus,
	FlowerFairy
};

class Monster
{
public:
	Monster() = default;
	Monster(int id, NormalMonsterType type, float x, float y, float z)
		:m_id(id), m_type(type), m_x(x), m_y(y), m_z(z) 
	{
		SetMonster(type);
	}
	~Monster();

	void		SetMonster(NormalMonsterType type);
	void		TakeDamage(int dmg);

	NormalMonsterType		GetType() const { return m_type; }
	float					GetX() const { return m_x; }
	float					GetY() const { return m_y; }
	float					GetZ() const { return m_z; }
	
private:
	int					m_id;					// 고유 ID
	NormalMonsterType	m_type;					// 몬스터 종류
	float				m_x, m_y, m_z;			// 좌표
	int					m_hp;					// 현재	체력
	int					m_attack;				// 공격력
	int					m_speed;				// 이동속도
	int					m_attackspeed;			// 공격속도
};