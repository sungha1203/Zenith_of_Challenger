// 몬스터 개별 행동
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
	Idle,				// 직선 왕복운동
	Aggro,				// 플레이어 어그로
	ReturnStart,		// 시작 지점으로 복귀
	Attack				// 공격
};

struct AggroInfo
{
	int		playerID = -1;		// 플레이어의 아이디
	float	distance = 0.0f;	// 해당 플레이어와의 거리 : 가장 가까운 플레이어을 타깃하기 위함
};

class Monster
{
private:
	int					m_version;								// 도전 - 0, 정점 - 1
	int					m_id;									// 고유 ID
	int					m_islived = true;						// 살아있는지
	NormalMonsterType	m_type;									// 몬스터 종류
	float				m_x, m_y, m_z;							// 좌표
	int					m_hp;									// 현재	체력
	int					m_attack;								// 공격력
	int					m_speed;								// 이동속도
	int					m_attackspeed;							// 공격속도
	float				m_attackrange;							// 공격범위
	float				m_bossAttackRange = 20.f;				// 보스 공격 범위
	int					m_targetplayer;							// 어그로X, 그냥 바라보고있는 방향(플레이어)
	MonsterState		m_state = MonsterState::Idle;			// 왕복 직선 운동

	std::vector<AggroInfo> m_AggroList;							// 어그로 리스트
	bool				m_aggro = false;						// 어그로 됐는지(정점스테이지에서만 사용)
	float				m_aggroRange = 40.f;					// 어그로 범위 40
	float				m_bossAggroRange = 100.f;				// 보스 어그로 범위 100
	int					m_aggroplayer = -1;						// 현재 어그로 끌린 플레이어의 id
	float				m_FirstLastCoord[2][2];					// 기본 돌아다니는 처음과 끝 좌표
	bool				m_direction = true;						// 기본 이동 루트 방향    // true : 정방향, false : 역방향
	bool				m_attackInProgress = false;				// 계속 공격 호출을 막기 위함
	float				m_attackCoolTime = 1.0f;				// 스킬 애니메이션 끝까지 걸리는 시간
	float				m_BossAttackCoolTime = 3.0f;			// 보스 스킬 애니메이션 끝까지 걸리는 시간
	bool				m_attackJustStart = false;				// 모든 클라에 몬스터 공격 브로드캐스트 체크
	bool				m_bossSkillType = false;				// 보스 스킬 유형 (false : 스킬1   true : 스킬2)
	bool				m_bossSkillCharging = false;			// 보스 몬스터 스킬 범위 차징중
	bool				m_bossSkillAnimation = false;			// 보스 몬스터 스킬 애니메이션 진행중
	std::chrono::steady_clock::time_point m_lastAttackTime;		// 스킬공격 마지막 시간 체크

public:
	Monster() = default;
	~Monster();

	std::mutex mx;

	void			Reset();
	void			SetMonster(int id, NormalMonsterType type, float x, float y, float z);
	void			TakeDamage(int dmg);
	void			Move();
	void			BossMove();
	int				UpdateTargetList();
	void			UpdateAggroList(const std::vector<PlayerInfo>& players);
	void			UpdateBossAggroList(const std::vector<PlayerInfo>& players);
	void			RealMove(const float x, const float z, const float X, const float Z);
	void			FirstMove();
	DropItemType	DropWHAT();
	void			SetFristLastCoord(float x1, float z1, float x2, float z2);
	bool			AttackAnimation();

	NormalMonsterType		GetType() const { return m_type; }
	bool					GetLived() const { return m_islived; }
	float					GetX() const { return m_x; }
	float					GetY() const { return m_y; }
	float					GetZ() const { return m_z; }
	int						GetHP() const { return m_hp; }
	int						GetAttack() const { return m_attack; }
	int						GetState() const { return (int)m_state; }
	bool					GetDirection() const { return m_direction; }
	float					GetFirstLastCoord(int FL, int XZ) const { return m_FirstLastCoord[FL][XZ]; }
	int						GetAggroPlayer() const { return m_aggroplayer; }
	bool					GetBossSkillType() const { return m_bossSkillType; }
};