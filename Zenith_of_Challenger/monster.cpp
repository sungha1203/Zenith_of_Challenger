#include "monster.h"

Monster::~Monster()
{

}

void Monster::SetMonster(NormalMonsterType type)
{
	switch (type) {
	case NormalMonsterType::Mushroom:
		m_hp = 100;
		m_attack = 5;
		m_speed = 2;
		m_attackspeed = 1;
		break;
	case NormalMonsterType::FightFly:
		m_hp = 20;
		m_attack = 3;
		m_speed = 5;
		m_attackspeed = 2;
		break;
	case NormalMonsterType::PlantDionaea:
		m_hp = 50;
		m_attack = 3;
		m_speed = 3;
		m_attackspeed = 2;
		break;
	case NormalMonsterType::PeaShooter:
		m_hp = 50;
		m_attack = 3;
		m_speed = 3;
		m_attackspeed = 3;
		break;
	case NormalMonsterType::PlantVenus:
		m_hp = 80;
		m_attack = 4;
		m_speed = 2;
		m_attackspeed = 1;
		break;
	case NormalMonsterType::FlowerFairy:
		m_hp = 50;
		m_attack = 5;
		m_speed = 2;
		m_attackspeed = 1;
		break;
	default:
		m_hp = 10;
		m_attack = 1;
		m_speed = 1;
		m_attackspeed = 1;
		break;
	}
}


void Monster::TakeDamage(int dmg)
{
	m_hp -= dmg;
	if (m_hp < 0)
		m_hp = 0;
}