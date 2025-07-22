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
	m_attackrange = 2.0f;

	switch (type) {
	case NormalMonsterType::Mushroom:
		m_hp = 100;
		m_attack = 5;
		m_speed = 3;
		m_attackspeed = 1;
		break;
	case NormalMonsterType::FightFly:
		m_hp = 100;
		m_attack = 3;
		m_speed = 8;
		m_attackspeed = 2;
		break;
	case NormalMonsterType::PlantDionaea:
		m_hp = 50;
		m_attack = 3;
		m_speed = 2;
		m_attackspeed = 2;
		break;
	case NormalMonsterType::PlantVenus:
		m_hp = 80;
		m_attack = 4;
		m_speed = 1;
		m_attackspeed = 1;
		break;
	case NormalMonsterType::FlowerFairy:
		m_hp = 50;
		m_attack = 5;
		m_speed = 6;
		m_attackspeed = 1;
		break;
	case NormalMonsterType::BossMonster:
		m_hp = 100;
		m_attack = 10;
		m_speed = 5;
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

// �÷��̾� �ٶ󺸴� ����Ʈ (���� ����������)
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
	m_targetplayer = nearestID; // ���� ����� �÷��̾�
	return m_targetplayer;
}

// ��׷� ����Ʈ(���� �i�ư����� �����ִ�..)
void Monster::UpdateAggroList(const std::vector<PlayerInfo>& players)
{
	m_AggroList.clear();
	float mindist = FLT_MAX;
	int nearID = -1;

	for (const auto& p : players) {
		float dx = m_x - p.x;	// �÷��̾�� ���� x����
		float dz = m_z - p.z;	// �÷��̾�� ���� z����
		float dist = sqrtf(dx * dx + dz * dz);

		if (dist <= m_aggroRange)
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
	else {
		m_aggroplayer = nearID;			// ���� ���� ��׷� ���� �÷��̾�
		m_state = MonsterState::Aggro;
	}
}

// ���� ���� ��׷� ����Ʈ
void Monster::UpdateBossAggroList(const std::vector<PlayerInfo>& players)
{
	m_AggroList.clear();
	for (const auto& p : players) {
		float dx = 0.0f - p.x;	// �÷��̾�� ���� ���� ó�� ��ġ x����
		float dz = 0.0f - p.z;	// �÷��̾�� ���� ���� ó�� ��ġ z����
		float dist = sqrtf(dx * dx + dz * dz);

		if (dist <= m_bossAggroRange)
		{
			float dx2 = m_x - p.x;	// �÷��̾�� ���� ���� x����
			float dz2 = m_z - p.z;	// �÷��̾�� ���� ���� z����
			float dist2 = sqrtf(dx2 * dx2 + dz2 * dz2);
			m_AggroList.push_back({ p.clientID, dist2 });

			std::sort(m_AggroList.begin(), m_AggroList.end(), [](const AggroInfo& a, const AggroInfo& b) { return a.distance < b.distance; });
		}
	}

	if (m_AggroList.size() == 0)
		m_aggroplayer = -1;
	else {
		m_aggroplayer = m_AggroList[0].playerID;		// ���� ���� ���Ϳ� ��׷� ���� �÷��̾�
		m_state = MonsterState::Aggro;
	}

}

// ȣ�� Ÿ�� �÷��̾ �̵��ϸ� ���� �������� �����ϱ� ����.
void Monster::Move()
{
	switch (m_state)
	{
	case MonsterState::Idle:		// �⺻ �պ� �
	{
		FirstMove();
		break;
	}
	case MonsterState::Aggro:		// Ÿ�� �÷��̾� ��׷�
	{
		if (m_aggroplayer == -1) {
			m_state = MonsterState::ReturnStart;
			return;
		}
		float targetX = g_client[m_aggroplayer].GetX();
		float targetZ = g_client[m_aggroplayer].GetZ();

		// �÷��̾�� ������ �Ÿ�
		float dx = targetX - m_x;
		float dz = targetZ - m_z;
		float dist = sqrtf(dx * dx + dz * dz);

		// ���Ϳ� ���� �������� ��ġ(�ʹ� ���� ������ �ʰ� �ϱ� ����)
		float dx2 = m_FirstLastCoord[0][0] - m_x;
		float dz2 = m_FirstLastCoord[0][1] - m_z;
		float dist2 = sqrtf(dx2 * dx2 + dz2 * dz2);

		if (dist > 45.0f) {						// ��׷ο� ���� �÷��̾ ���� �Ÿ� �������� ����
			m_state = MonsterState::ReturnStart;
			m_aggroplayer = -1;
			return;
		}
		else if (dist2 > 60.0f) {				// ���Ͱ� �÷��̾ ���� �Ÿ� ���󰡸� ����
			m_state = MonsterState::ReturnStart;
			m_aggroplayer = -1;
			return;
		}
		else if (dist < m_attackrange) {		// ���� ��Ÿ��� ������ �� ����
			m_state = MonsterState::Attack;
			return;
		}
		else									// �÷��̾� ������!
			RealMove(m_x, m_z, targetX, targetZ);
		break;
	}
	case MonsterState::ReturnStart:	// ��׷� ���� �� ���� �պ� ��ҷ� �̵�
	{
		float StartX = m_FirstLastCoord[0][0];
		float StartZ = m_FirstLastCoord[0][1];
		float dx = StartX - m_x;
		float dz = StartZ - m_z;
		float dist = sqrtf(dx * dx + dz * dz);

		if (dist < 1.0f) {
			m_state = MonsterState::Idle;
		}
		else
			RealMove(m_x, m_z, StartX, StartZ);
		break;
	}
	case MonsterState::Attack:
	{
		if (m_aggroplayer == -1) {
			m_state = MonsterState::ReturnStart;
			return;
		}

		auto now = std::chrono::steady_clock::now();
		float elapsed = std::chrono::duration<float>(now - m_lastAttackTime).count();

		if (!m_attackInProgress) {				// ���� �ִϸ��̼� �������� �ƴ϶��
			m_attackInProgress = true;			// ���� �ִϸ��̼� ������
			m_lastAttackTime = now;
			m_attackJustStart = true;			// ���� ���� �������� ��� Ŭ������ �� ��Ȳ ������
		}
		else if (elapsed >= m_attackCoolTime) {	// ���� �ִϸ��̼� ���� �� �˻� �� ���Ͱ� ���� �̵��ؾ��ϴ���
			m_attackInProgress = false;			// ���� �ִϸ��̼� ����
			if (m_AggroList.empty()) {			// ��׷� ����Ʈ�� �ƹ��� ���� �� ���� �������� ����
				m_state = MonsterState::ReturnStart;
			}
			else {
				m_state = MonsterState::Aggro;	// ��׷� ����Ʈ�� ����� ������ ��� ���󰡼�
			}
		}
		break;
	}
	default:
	{
		break;
	}
	}
}

// ���� ���� �ൿƮ��
void Monster::BossMove()
{
	switch (m_state)
	{
	case MonsterState::Idle:		// ù ��ġ�� ������ �ֱ�
	{
		break;
	}
	case MonsterState::Aggro:		// 
	{
		if (m_aggroplayer == -1) {
			m_state = MonsterState::ReturnStart;
			return;
		}
		float targetX = g_client[m_aggroplayer].GetX();
		float targetZ = g_client[m_aggroplayer].GetZ();

		// �÷��̾�� ������ �Ÿ�
		float dx = targetX - m_x;
		float dz = targetZ - m_z;
		float dist = sqrtf(dx * dx + dz * dz);

		if (dist < m_bossAttackRange) {
			m_state = MonsterState::Attack;
			return;
		}

		RealMove(m_x, m_z, targetX, targetZ);
		break;
	}
	case MonsterState::ReturnStart:	// ���� ù ��ġ�� �̵�
	{
		float StartX = 0.f;
		float StartZ = -23.f;
		float dx = StartX - m_x;
		float dz = StartZ - m_z;
		float dist = sqrtf(dx * dx + dz * dz);

		if (dist < 1.0f) {
			m_state = MonsterState::Idle;
		}
		else
			RealMove(m_x, m_z, StartX, StartZ);
		break;
	}
	case MonsterState::Attack:
	{
		if (m_aggroplayer == -1) {
			m_state = MonsterState::ReturnStart;
			return;
		}

		auto now = std::chrono::steady_clock::now();
		float elapsed = std::chrono::duration<float>(now - m_lastAttackTime).count();

		if (!m_attackInProgress) {				// ���� �ִϸ��̼� �������� �ƴ϶��
			m_attackInProgress = true;			// ���� �ִϸ��̼� ������
			m_lastAttackTime = now;
			m_bossSkillType = true;
			m_attackJustStart = true;			// ���� ���� �������� ��� Ŭ������ �� ��Ȳ ������
		}
		else if (elapsed >= m_attackCoolTime) {	// ���� �ִϸ��̼� ���� �� �˻� �� ���Ͱ� ���� �̵��ؾ��ϴ���
			m_attackInProgress = false;			// ���� �ִϸ��̼� ����
			if (m_AggroList.empty()) {			// ��׷� ����Ʈ�� �ƹ��� ���� �� ���� �������� ����
				m_state = MonsterState::ReturnStart;
			}
			else {
				m_state = MonsterState::Aggro;	// ��׷� ����Ʈ�� ����� ������ ��� ���󰡼�
			}
		}
		break;
	}
	default:
	{
		break;
	}
	}
}

// ���� ��ǥ �̵�
void Monster::RealMove(const float x, const float z, const float X, const float Z)
{
	float dx = X - x;
	float dz = Z - z;
	float len = sqrtf(dx * dx + dz * dz);
	if (len < 0.001f) return;

	dx /= len;
	dz /= len;

	m_x += dx * m_speed * 0.2f;
	m_z += dz * m_speed * 0.2f;
}

// �⺻ ��Ʈ �պ� �̵�
void Monster::FirstMove()
{
	float* to = m_direction ? m_FirstLastCoord[1] : m_FirstLastCoord[0];

	RealMove(m_x, m_z, to[0], to[1]);

	float dx = to[0] - m_x;
	float dz = to[1] - m_z;
	float distSq = sqrtf(dx * dx + dz * dz);

	if (distSq < 1.0f)
		m_direction = !m_direction;
}

// �⺻ ��Ʈ �ʱ�ȭ
void Monster::SetFristLastCoord(float x1, float z1, float x2, float z2)
{
	m_FirstLastCoord[0][0] = x1;
	m_FirstLastCoord[0][1] = z1;
	m_FirstLastCoord[1][0] = x2;
	m_FirstLastCoord[1][1] = z2;
}

// ��� Ŭ�� ���� �ִϸ��̼�
bool Monster::AttackAnimation()
{
	if (m_attackJustStart) {
		m_attackJustStart = false;
		return true;
	}
	return false;
}

DropItemType Monster::DropWHAT()
{
	std::random_device rd;
	std::default_random_engine dre{ rd() };
	std::uniform_int_distribution<int> uid{ 1,100 };
	int num = uid(dre);

	if (num <= 10) {							// ����(��)
		return DropItemType::SWORD;
	}
	else if (num > 10 && num <= 20) {			// ����(������)
		return DropItemType::WAND;
	}
	else if (num > 20 && num <= 30) {			// ����(����)
		return DropItemType::SHIELD;
	}
	else if (num > 30 && num <= 40) {			// ������(����)
		return DropItemType::WARRIOR;
	}
	else if (num > 40 && num <= 50) {			// ������(������)
		return DropItemType::MAGE;
	}
	else if (num > 50 && num <= 60) {			// ������(����Ŀ)
		return DropItemType::HEALTANKER;
	}
	else {										// ��
		return DropItemType::None;
	}
}