#pragma once
#include "stdafx.h"

enum class Classtype
{
	CHALLENGER,					// ������
	WARRIOR,					// ����
	MAGE,						// ������
	HEALTANKER					// ����Ŀ
};

struct Weapon
{
	int		type;				// ���� ���� (0. �Ǽ� / 1.�� / 2.������ / 3. ����)
	int		level;				// ���� ��ȭ ���� (0~9)
};

struct Player
{
	int				clientID = -1;			// ���̵�
	Classtype		classtype;				// ���� 
	Weapon			weapon;					// ����
	int				clothes = 0;			// ������
	int				maxhp;					// �ִ� ü��
	int				hp;						// ���� ü��
	int				attack;					// ���ݷ�
	int				speed;					// �̵� �ӵ�
	int				attackspeed;			// ���� �ӵ�
	float			x, y, z;				// �ΰ��� ĳ���� ��ǥ
	float			angle;					// ����
};

// ���Ϳ� ��׷� ���¸� �˱����� ����ü
struct PlayerInfo
{
	int		clientID;
	float	x, z;
};

class ClientInfo       
{
private:
	Player	m_ingameInfo;			// �ΰ��� ���� (1.���� / 2.���� / 3.������ / 4.ü�� / 5.���ݷ� / 6.�̵��ӵ� / 7.���ݼӵ� / 8.��ǥ)
	int		m_normalAttack = 0;		// ���� �������� ��Ÿ ���ݷ�
	int		m_skillAttack = 0;		// ���� �������� ��ų ���ݷ�

	std::chrono::steady_clock::time_point m_lastSkillTime;

public:
	ClientInfo();
	ClientInfo(int client_id);		// �ʱ� ĳ���� ����
	~ClientInfo();

	void	SetClothes(const int clothes);									// �� ����
	void	SetJobType(int JobNum);											// ���� ����
	void	SetWeapon(int weaponNum);										// ���� ����
	void	SetWeaponGrade();												// ���� ��ȭ
	bool	SetEnhanceGradeUp(int weapongrade);								// ���� ��ȭ ����(���� ���� ��ȯ)
	void	SetSpawnCoord(int idx);											// �ʱ� ���� ��ǥ
	void	SetRepairCoord(int idx);										// ������ �� ���� ��ǥ
	void	SetZenithCoord(int idx);										// ���� �������� ���� ��ǥ
	void	SetCoord(float x, float y, float z);							// ��ǥ ����
	void	SetAngle(float angle);											// ���� ����
	void	SetAttack();													// ���� ���� ���� ���ݷ� ����
	
	void	LeverUpPlayer(int classtype);									// ���� ���� �� ĳ���� ����

	void	AddHP(int heal);												// �� ȸ��
	void	MinusHP(int damage, int gamemode);								// �� ����

	Classtype GetJobType() const { return m_ingameInfo.classtype; }			// ĳ���� ���� ��ȯ
	int		GetWeaponType() const { return m_ingameInfo.weapon.type; }		// ĳ���� ���� ��ȯ
	int		GetWeaponGrade() const { return m_ingameInfo.weapon.level; }	// ĳ���� ���� ��� ��ȯ
	int		GetID() const { return m_ingameInfo.clientID;  }				// ĳ���� ���̵� ��ȯ
	float	GetX() const { return m_ingameInfo.x; }							// ĳ���� X��ǥ ��ȯ
	float	GetY() const { return m_ingameInfo.y; }							// ĳ���� y��ǥ ��ȯ
	float	GetZ() const { return m_ingameInfo.z; }							// ĳ���� z��ǥ ��ȯ
	int		GetHP() const { return m_ingameInfo.hp; }						// ĳ���� HP ��ȯ
	int		GetNormalAttack() const { return m_normalAttack; }				// ĳ���� ��Ÿ ���ݷ� ��ȯ
	int		GetSkillAttack() const { return m_skillAttack; }				// ĳ���� ��ų ���ݷ� ��ȯ
	int		GetSpeed() const { return m_ingameInfo.speed; }					// ĳ���� �̵��ӵ� ��ȯ
	int		GetAttackSpeed() const { return m_ingameInfo.attackspeed; }		// ĳ���� ���ݼӵ� ��ȯ
	float	GetAngle() const { return m_ingameInfo.angle; }					// ĳ���� ���� ��ȯ

	bool	CanUseSkill();													// ��ų ��Ÿ���� á����
	void	StartCoolTime();												// ��ų ��Ÿ�� ������
};