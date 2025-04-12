#include "monster.h"

Monster::Monster(int hp, int attack, int speed)
{
	m_hp = hp;
	m_attack = attack;
	m_speed = speed;
}

Monster::~Monster()
{

}

void Monster::Update()
{

}


//------------------------------------------------------------

void NormalMonster::Update()
{

}
