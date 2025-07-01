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

struct AggroInfo
{
	int		playerID = -1;		// �÷��̾��� ���̵�
	float	distance = 0.0f;	// �ش� �÷��̾���� �Ÿ� : ���� ����� �÷��̾��� Ÿ���ϱ� ����
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
	int					m_version;				// ���� - 0, ���� - 1
	int					m_id;					// ���� ID
	int					m_islived = true;		// ����ִ���
	NormalMonsterType	m_type;					// ���� ����
	float				m_x, m_y, m_z;			// ��ǥ
	int					m_hp;					// ����	ü��
	int					m_attack;				// ���ݷ�
	int					m_speed;				// �̵��ӵ�
	int					m_attackspeed;			// ���ݼӵ�

	std::vector<AggroInfo> m_AggroList;			// ��׷� ����Ʈ
	bool				m_aggro = false;		// ��׷� �ƴ���(������������������ ���)
	float				m_aggroRange = 5.f;		// ��׷� ����
	int					m_aggroplayer = -1;		// ���� ��׷� ���� �÷��̾��� id
};