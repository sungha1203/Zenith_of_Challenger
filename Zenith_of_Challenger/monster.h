// ���� ���� �ൿ
#include "stdafx.h"

extern std::unordered_map<int, ClientInfo> g_client;

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

enum class MonsterState
{
	Idle,				// ���� �պ��
	Aggro,				// �÷��̾� ��׷�
	ReturnStart,		// ���� �������� ����
	Attack				// ����
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
	void			Move();
	int				UpdateTargetList();
	void			UpdateAggroList(const std::vector<PlayerInfo>& players);
	void			RealMove(const float x, const float z, const float X, const float Z);
	void			FirstMove();
	DropItemType	DropWHAT();
	void			SetFristLastCoord(float x1, float z1, float x2, float z2);

	NormalMonsterType		GetType() const { return m_type; }
	bool					GetLived() const { return m_islived; }
	float					GetX() const { return m_x; }
	float					GetY() const { return m_y; }
	float					GetZ() const { return m_z; }
	int						GetHP() const { return m_hp; }
	int						GetAttack() const { return m_attack; }
	
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
	float				m_attackrange;			// ���ݹ���
	int					m_targetplayer;			// ��׷�X, �׳� �ٶ󺸰��ִ� ����(�÷��̾�)
	MonsterState		m_state = MonsterState::Idle;	// �պ� ���� �

	std::vector<AggroInfo> m_AggroList;			// ��׷� ����Ʈ
	bool				m_aggro = false;		// ��׷� �ƴ���(������������������ ���)
	float				m_aggroRange = 40.f;	// ��׷� ���� 40
	int					m_aggroplayer = -1;		// ���� ��׷� ���� �÷��̾��� id
	float				m_FirstLastCoord[2][2];	// �⺻ ���ƴٴϴ� ó���� �� ��ǥ
	//float				m_StopCoord;			// �⺻ �̵� ��Ʈ���� ��׷ο� ���� ������ ��(��׷ΰ� Ǯ���� �ٽ� ���ư� ��ġ)
	bool				m_direction = true;		// �⺻ �̵� ��Ʈ ����    // true : ������, false : ������
};