#include "monster.h"

Monster::~Monster()
{

}

void Monster::Reset()
{
	m_hp = 0;
	m_attack = 0;
	m_speed = 0;
	m_attackspeed = 0;
	m_x = m_y = m_z = 0.0f;
	m_id = -1;
}

void Monster::SetMonster(int id, NormalMonsterType type, float x, float y, float z)
{
	m_id = id;
	m_type = type;
	m_x = x;
	m_y = y;
	m_z = z;

	switch (type) {
	case NormalMonsterType::Mushroom:
		m_hp = 100;
		m_attack = 5;
		m_speed = 2;
		m_attackspeed = 1;
		break;
	case NormalMonsterType::FightFly:
		m_hp = 100;
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
	mx.lock();
	m_hp -= dmg;
	if (m_hp < 0) {
		m_islived = false;
		m_hp = 0;
	}
	mx.unlock();
}

void Monster::AIMove()
{
	if (m_aggroplayer == -1) return;

	float targetX = g_client[m_aggroplayer].GetX();
	float targetZ = g_client[m_aggroplayer].GetZ();

	Astar(m_x, m_z, targetX, targetZ);
}

int Monster::UpdateTargetList()
{
	float mindist = FLT_MAX;
	int nearestID = -1;

	for (const auto& player : g_client)
	{
		float dx = player.second.GetX() - m_x;
		float dz = player.second.GetZ() - m_z;
		float distSq = dx * dx + dz * dz;

		if (distSq < mindist)
		{
			mindist = distSq;
			nearestID = player.second.GetID();
		}
	}
	m_targetplayer = nearestID; // 가장 가까운 플레이어
	return m_targetplayer;
}

void Monster::UpdateAggroList(const std::vector<PlayerInfo>& players)
{
	m_AggroList.clear();
	float mindist = FLT_MAX;
	int nearID = -1;

	for (const auto& p : players) {
		float dx = m_x - p.x;	// 플레이어와 몬스터 x차이
		float dz = m_z - p.z;	// 플레이어와 몬스터 z차이
		float dist = dx * dx + dz * dz;

		if (dist <= m_aggroRange * m_aggroRange)
		{
			m_AggroList.push_back({ p.clientID, dist });

			if (dist < mindist)
			{
				mindist = dist;
				nearID = p.clientID;
			}
		}
	}

	if (m_AggroList.size() == 0)
		m_aggroplayer = -1;
	else
		m_aggroplayer = nearID;			// 지금 현재 어그로 끌린 플레이어
}

void Monster::Astar(const float x, const float z, const float targetX, const float targetZ)
{

}

DropItemType Monster::DropWHAT()
{
	std::random_device rd;
	std::default_random_engine dre{ rd() };
	std::uniform_int_distribution<int> uid{ 1,100 };
	int num = uid(dre);

	if (num <= 10) {							// 무기(검)
		return DropItemType::SWORD;
	}
	else if (num > 10 && num <= 20) {			// 무기(지팡이)
		return DropItemType::WAND;
	}
	else if (num > 20 && num <= 30) {			// 무기(방패)
		return DropItemType::SHIELD;
	}
	else if (num > 30 && num <= 40) {			// 전직서(전사)
		return DropItemType::WARRIOR;
	}
	else if (num > 40 && num <= 50) {			// 전직서(마법사)
		return DropItemType::MAGE;
	}
	else if (num > 50 && num <= 60) {			// 전직서(힐탱커)
		return DropItemType::HEALTANKER;
	}
	else {										// 꽝
		return DropItemType::None;
	}
}