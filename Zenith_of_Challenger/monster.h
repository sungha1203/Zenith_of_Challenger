// ���� ���� �ൿ
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
	float	m_x, m_y;				// ��ǥ
	int		m_hp;					// ü��
	int		m_attack;				// ���ݷ�
	int		m_speed;				// �̵��ӵ�
	int		m_attackspeed;			// ���ݼӵ�
	
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