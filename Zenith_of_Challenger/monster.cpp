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

// 플레이어 바라보는 리스트 (도전 스테이지만)
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

// 어그로 리스트(누구 쫒아갈건지 정해주는..)
void Monster::UpdateAggroList(const std::vector<PlayerInfo>& players)
{
	if (m_state == MonsterState::Attack) return;
	m_AggroList.clear();
	float mindist = FLT_MAX;
	int nearID = -1;

	for (const auto& p : players) {
		float dx = m_x - p.x;	// 플레이어와 몬스터 x차이
		float dz = m_z - p.z;	// 플레이어와 몬스터 z차이
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
		m_aggroplayer = nearID;			// 지금 현재 어그로 끌린 플레이어
		m_state = MonsterState::Aggro;
	}
}

// 보스 몬스터 어그로 리스트
void Monster::UpdateBossAggroList(const std::vector<PlayerInfo>& players)
{
	if (m_state == MonsterState::Attack) return;
	m_AggroList.clear();
	for (const auto& p : players) {
		float dx = 0.0f - p.x;	// 플레이어와 보스 몬스터 처음 위치 x차이
		float dz = 0.0f - p.z;	// 플레이어와 보스 몬스터 처음 위치 z차이
		float dist = sqrtf(dx * dx + dz * dz);

		if (dist <= m_bossAggroRange)
		{
			float dx2 = m_x - p.x;	// 플레이어와 보스 몬스터 x차이
			float dz2 = m_z - p.z;	// 플레이어와 보스 몬스터 z차이
			float dist2 = sqrtf(dx2 * dx2 + dz2 * dz2);
			m_AggroList.push_back({ p.clientID, dist2 });

			std::sort(m_AggroList.begin(), m_AggroList.end(), [](const AggroInfo& a, const AggroInfo& b) { return a.distance < b.distance; });
		}
	}

	if (m_AggroList.size() == 0)
		m_aggroplayer = -1;
	else {
		m_aggroplayer = m_AggroList[0].playerID;		// 현재 보스 몬스터에 어그로 끌린 플레이어
		m_state = MonsterState::Aggro;
	}

}

// 호출 타겟 플레이어가 이동하면 최종 도착지를 변경하기 위함.
void Monster::Move()
{
	switch (m_state)
	{
	case MonsterState::Idle:		// 기본 왕복 운동
	{
		FirstMove();
		break;
	}
	case MonsterState::Aggro:		// 타겟 플레이어 어그로
	{
		if (m_aggroplayer == -1) {
			m_state = MonsterState::ReturnStart;
			return;
		}
		float targetX = g_client[m_aggroplayer].GetX();
		float targetZ = g_client[m_aggroplayer].GetZ();

		// 플레이어와 몬스터의 거리
		float dx = targetX - m_x;
		float dz = targetZ - m_z;
		float dist = sqrtf(dx * dx + dz * dz);

		// 몬스터와 시작 지점과의 위치(너무 많이 따라가지 않게 하기 위함)
		float dx2 = m_FirstLastCoord[0][0] - m_x;
		float dz2 = m_FirstLastCoord[0][1] - m_z;
		float dist2 = sqrtf(dx2 * dx2 + dz2 * dz2);

		if (dist > 45.0f) {						// 어그로에 끌린 플레이어가 일정 거리 떨어지면 복귀
			m_state = MonsterState::ReturnStart;
			m_aggroplayer = -1;
			return;
		}
		else if (dist2 > 60.0f) {				// 몬스터가 플레이어를 일정 거리 따라가면 복귀
			m_state = MonsterState::ReturnStart;
			m_aggroplayer = -1;
			return;
		}
		else if (dist < m_attackrange) {		// 공격 사거리에 들어왔을 때 공격
			m_state = MonsterState::Attack;
			return;
		}
		else									// 플레이어 따라가자!
			RealMove(m_x, m_z, targetX, targetZ);
		break;
	}
	case MonsterState::ReturnStart:	// 어그로 해제 및 시작 왕복 장소로 이동
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

		if (!m_attackInProgress) {				// 공격 애니메이션 진행중이 아니라면
			m_attackInProgress = true;			// 공격 애니메이션 진행중
			m_lastAttackTime = now;
			m_attackJustStart = true;			// 지금 공격 시작했음 모든 클라한테 이 상황 보내줘
		}
		else if (elapsed >= m_attackCoolTime) {	// 공격 애니메이션 끝난 후 검사 후 몬스터가 어디로 이동해야하는지
			m_attackInProgress = false;			// 공격 애니메이션 종료
			if (m_AggroList.empty()) {			// 어그로 리스트에 아무도 없을 때 시작 지점으로 복귀
				m_state = MonsterState::ReturnStart;
			}
			else {
				m_state = MonsterState::Aggro;	// 어그로 리스트에 사람이 있으면 계속 따라가셈
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

// 보스 몬스터 행동트리
void Monster::BossMove()
{
	switch (m_state)
	{
	case MonsterState::Idle:		// 첫 위치에 가만히 있기
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

		// 플레이어와 몬스터의 거리
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
	case MonsterState::ReturnStart:	// 보스 첫 위치로 이동
	{
		float StartX = 0.f;
		float StartZ = -23.f;
		float dx = StartX - m_x;
		float dz = StartZ - m_z;
		float dist = sqrtf(dx * dx + dz * dz);

		if (dist < 1.0f) {
			std::cout << "[INFO] 보스몬스터 시작 지점 복귀 완료" << std::endl;
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

		// 공격 애니메이션 진행중이 아니라면
		if (!m_attackInProgress) {				
			m_attackInProgress = true;			// 공격 애니메이션 진행중
			m_attackJustStart = true;			// 지금 공격 시작했음 모든 클라한테 이 상황 보내줘
			m_lastAttackTime = now;
			m_bossSkillCharging = true;
			m_baseY = m_y;
			std::cout << "[INFO] 보스몬스터 스킬" << m_bossSkillType << " 예고 시작" << std::endl;

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
		// 스킬 범위시전 2초 지난 후 실제 스킬 사용 시점
		else if (m_bossSkillCharging && elapsed >= 2.0f) {		
			std::cout << "[INFO] 보스몬스터 스킬" << m_bossSkillType << " 예고 끝" << std::endl;
			m_bossSkillCharging = false;
			m_bossSkillAnimation = true;
			m_lastAttackTime = now;
			std::cout << "[INFO] 보스몬스터 스킬" << m_bossSkillType << " 사용" << std::endl;
		}
		// 스킬 사용중
		else if (m_bossSkillAnimation && elapsed < 2.0f) {
			float animElapsed = std::chrono::duration<float>(now - m_lastAttackTime).count();
			float ratio = animElapsed / 2.0f;					// 공격 진행률

			if (m_bossSkillType == 1) {							// 스킬1(돌진)
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
			else if (m_bossSkillType == 2) {					// 스킬2(점프)
				float t = ratio;
				float jumpY = m_skillJumpHeight * 4 * t * (1 - t);
				m_y = m_baseY + jumpY;
				m_x = m_skillTargetX;
				m_z = m_skillTargetZ;
			}

		}
		// 스킬 사용 끝난 직후
		else if (m_bossSkillAnimation && elapsed >= 2.0f) {		
			if (m_bossSkillType == 2 && !m_playerDamaged) {
				m_playerDamaged = true;
				m_damageThisFrame = true;
			}

			if (elapsed >= 2.5f) {
				std::cout << "[INFO] 보스몬스터 스킬" << m_bossSkillType << " 끝" << std::endl;
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
					std::cout << "[INFO] 보스몬스터 플레이어 따라감" << std::endl;
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

// 실제 좌표 이동
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

// 기본 루트 왕복 이동
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

// 기본 루트 초기화
void Monster::SetFristLastCoord(float x1, float z1, float x2, float z2)
{
	m_FirstLastCoord[0][0] = x1;
	m_FirstLastCoord[0][1] = z1;
	m_FirstLastCoord[1][0] = x2;
	m_FirstLastCoord[1][1] = z2;
}

// 모든 클라에 공격 애니메이션
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

void Monster::BossSkillDamage(const std::vector<int>& clients)
{
	if (m_bossSkillType == 1) BossSkillDashDamage(clients);
	else if (m_bossSkillType == 2) BossSkillJumpDamage(clients);
}

// 보스 몬스터 스킬1 돌진 데미지 판정
void Monster::BossSkillDashDamage(const std::vector<int>& clients)
{
	
}

// 보스 몬스터 스킬2 점프 데미지 판정
void Monster::BossSkillJumpDamage(const std::vector<int>& clients)
{
	for (const int clinetID : clients) {
		float targetX = g_client[clinetID].GetX();
		float targetZ = g_client[clinetID].GetZ();

		float dx = m_x - targetX;
		float dz = m_z - targetZ;
		float dist = sqrt(dx * dx + dz * dz);

		if (dist <= 100.f) {			// 점프 스킬 데미지 범위
			g_client[clinetID].MinusHP(m_attack, 3);
			std::cout << "[DAMAGE] [" << clinetID << "]번 플레이어 데미지 받음" << std::endl;
		}
	}
}

void Monster::SetDamageThisFrame(bool check)
{
	m_damageThisFrame = check;
}
