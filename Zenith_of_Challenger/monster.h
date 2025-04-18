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
	int					m_id;					// ���� ID
	NormalMonsterType	m_type;					// ���� ����
	float				m_x, m_y, m_z;			// ��ǥ
	int					m_hp;					// ����	ü��
	int					m_attack;				// ���ݷ�
	int					m_speed;				// �̵��ӵ�
	int					m_attackspeed;			// ���ݼӵ�
};