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

void Monster::SetMonster(int id, NormalMonsterType type, float x, float y, float z, int version)
{
	m_id = id;
	m_type = type;
	m_x = x;
	m_y = y;
	m_z = z;
	m_attackspeed = 1;
	m_version = version;

	switch (type) {
	case NormalMonsterType::Mushroom:
		if(m_version == 0) m_hp = 60;
		else			   m_hp = 150;
		m_attack = 20;
		m_speed = 3;
		break;
	case NormalMonsterType::FightFly:
		if (m_version == 0) m_hp = 60;
		else			    m_hp = 150;
		m_attack = 10;
		m_speed = 8;
		break;
	case NormalMonsterType::PlantDionaea:
		if (m_version == 0) m_hp = 90;
		else			    m_hp = 200;
		m_attack = 25;
		m_speed = 2;
		break;
	case NormalMonsterType::PlantVenus:
		if (m_version == 0) m_hp = 90;
		else			    m_hp = 250;
		m_attack = 30;
		m_speed = 1;
		break;
	case NormalMonsterType::FlowerFairy:
		if (m_version == 0) m_hp = 30;
		else			    m_hp = 100;
		m_attack = 60;
		m_speed = 6;
		break;
	case NormalMonsterType::BossMonster:
		m_hp = 3000;
		m_attack = 140;
		m_speed = 5;
		break;
	default:
		m_hp = 10;
		m_attack = 1;
		m_speed = 1;
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
	if (m_state == MonsterState::Attack) return;
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
	if (m_state == MonsterState::Attack) return;
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
			std::cout << "[INFO] �������� ���� ���� ���� �Ϸ�" << std::endl;
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

		// ���� �ִϸ��̼� �������� �ƴ϶��
		if (!m_attackInProgress) {				
			m_attackInProgress = true;			// ���� �ִϸ��̼� ������
			m_attackJustStart = true;			// ���� ���� �������� ��� Ŭ������ �� ��Ȳ ������
			m_lastAttackTime = now;
			m_bossSkillCharging = true;
			m_baseY = m_y;
			std::cout << "[INFO] �������� ��ų" << m_bossSkillType << " ���� ����" << std::endl;

			if (m_bossSkillType == 1) {
				float tx = g_client[m_aggroplayer].GetX();
				float tz = g_client[m_aggroplayer].GetZ();
				float dx = tx - m_x;
				float dz = tz - m_z;
				float len = sqrtf(dx * dx + dz * dz);
				if (len > 0.0001f) {
					dx /= len;
					dz /= len;
				}
				m_skillStartX = m_x;
				m_skillStartZ = m_z;
				m_skillTargetX = m_x + dx * m_skillDashDistance;
				m_skillTargetZ = m_z + dz * m_skillDashDistance;
			}
			else if (m_bossSkillType == 2) {
				m_skillTargetX = m_x;
				m_skillTargetZ = m_z;
			}

		}
		// ��ų �������� 2�� ���� �� ���� ��ų ��� ����
		else if (m_bossSkillCharging && elapsed >= 2.0f) {		
			std::cout << "[INFO] �������� ��ų" << m_bossSkillType << " ���� ��" << std::endl;
			m_bossSkillCharging = false;
			m_bossSkillAnimation = true;
			m_lastAttackTime = now;
			std::cout << "[INFO] �������� ��ų" << m_bossSkillType << " ���" << std::endl;
		}
		// ��ų �����
		else if (m_bossSkillAnimation && elapsed < 2.0f) {
			float animElapsed = std::chrono::duration<float>(now - m_lastAttackTime).count();
			float ratio = animElapsed / 2.0f;					// ���� �����

			if (m_bossSkillType == 1) {							// ��ų1(����)
				if (ratio <= 0.5f) {
					float t = ratio * 2.0f;
					m_x = m_skillStartX + (m_skillTargetX - m_skillStartX) * t;
					m_z = m_skillStartZ + (m_skillTargetZ - m_skillStartZ) * t;
				}
				else if (ratio > 0.5f && !m_playerDamaged) {
					m_playerDamaged = true;
					m_damageThisFrame = true;
				}
				else {
					float t = (ratio - 0.5f) * 2.0f;
					m_x = m_skillTargetX + (m_skillStartX - m_skillTargetX) * t;
					m_z = m_skillTargetZ + (m_skillStartZ - m_skillTargetZ) * t;
				}
			}
			else if (m_bossSkillType == 2) {					// ��ų2(����)
				float t = ratio;
				float jumpY = m_skillJumpHeight * 4 * t * (1 - t);
				m_y = m_baseY + jumpY;
				m_x = m_skillTargetX;
				m_z = m_skillTargetZ;
			}

		}
		// ��ų ��� ���� ����
		else if (m_bossSkillAnimation && elapsed >= 2.0f) {		
			if (m_bossSkillType == 2 && !m_playerDamaged) {
				m_playerDamaged = true;
				m_damageThisFrame = true;
			}

			if (elapsed >= 2.5f) {
				std::cout << "[INFO] �������� ��ų" << m_bossSkillType << " ��" << std::endl;
				m_playerDamaged = false;
				m_attackInProgress = false;
				m_bossSkillAnimation = false;
				m_y = m_baseY;

				if (m_bossSkillType == 1)
					m_bossSkillType = 2;
				else if (m_bossSkillType == 2)
					m_bossSkillType = 1;

				if (m_AggroList.empty()) {
					m_state = MonsterState::ReturnStart;
				}
				else {
					m_state = MonsterState::Aggro;
					std::cout << "[INFO] �������� �÷��̾� ����" << std::endl;
				}
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

void Monster::SetAttackJustStart(bool check)
{
	m_attackJustStart = false;
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

void Monster::BossSkillDamage(const std::vector<int>& clients)
{
	if (m_bossSkillType == 1) BossSkillDashDamage(clients);
	else if (m_bossSkillType == 2) BossSkillJumpDamage(clients);
}

// ���� ���� ��ų1 ���� ������ ����
void Monster::BossSkillDashDamage(const std::vector<int>& clients)
{
	
}

// ���� ���� ��ų2 ���� ������ ����
void Monster::BossSkillJumpDamage(const std::vector<int>& clients)
{
	for (const int clinetID : clients) {
		float targetX = g_client[clinetID].GetX();
		float targetZ = g_client[clinetID].GetZ();

		float dx = m_x - targetX;
		float dz = m_z - targetZ;
		float dist = sqrt(dx * dx + dz * dz);

		if (dist <= 100.f) {			// ���� ��ų ������ ����
			g_client[clinetID].MinusHP(m_attack, 3);
			std::cout << "[DAMAGE] [" << clinetID << "]�� �÷��̾� ������ ����" << std::endl;
		}
	}
}

void Monster::SetDamageThisFrame(bool check)
{
	m_damageThisFrame = check;
}
