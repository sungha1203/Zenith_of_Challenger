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
protected:
	float	m_x, m_y;				// 좌표
	int		m_hp;					// 체력
	int		m_attack;				// 공격력
	int		m_speed;				// 이동속도
	int		m_attackspeed;			// 공격속도
	
public:
	Monster(int hp, int attack, int speed);
	~Monster();

	virtual void Update();


	std::pair<float, float> GetPosition() const { return { m_x, m_y }; }
};

class NormalMonster : public Monster 
{
private:
	NormalMonsterType type;

public:
	void Update() override;
};