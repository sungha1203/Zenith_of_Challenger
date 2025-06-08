// ���� ���� �ൿ
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
	int					m_id;					// ���� ID
	int					m_islived = true;		// ����ִ���
	NormalMonsterType	m_type;					// ���� ����
	float				m_x, m_y, m_z;			// ��ǥ
	int					m_hp;					// ����	ü��
	int					m_attack;				// ���ݷ�
	int					m_speed;				// �̵��ӵ�
	int					m_attackspeed;			// ���ݼӵ�
};