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
	int		type;				// ���� ���� (1.�� / 2.������ / 3. ����)
	int		level;				// ���� ��ȭ ���� (1~9)
};

struct Player
{
	Classtype		classtype;				// ���� 
	Weapon			weapon;					// ����
	int				clothes[3] = {0,0,0};	// ������
	int				hp;						// ü��
	int				attack;					// ���ݷ�
	int				speed;					// �̵� �ӵ�
	int				attackspeed;			// ���� �ӵ�
	float			x, y, z;				// �ΰ��� ĳ���� ��ǥ
	float			dir;					// ����
};

class ClientInfo       
{
private:
	Player	m_ingameInfo;			// �ΰ��� ���� (1.���� / 2.���� / 3.������ / 4.ü�� / 5.���ݷ� / 6.�̵��ӵ� / 7.���ݼӵ� / 8.��ǥ)
	int		m_id;					// Ŭ���̾�Ʈ ID
	int		m_roomNum;				// ���� �� ��ȣ

public:
	ClientInfo();
	ClientInfo(int client_id, int roomNum);		// �ʱ� ĳ���� ����
	~ClientInfo();

	int GetID() const { return m_id; }

	void SetRoomNum(const int room_id);									// �� ����
	void SetClothes(const int clothes[3]);								// �� ����
	void SetCoord(float x, float y, float z);													// ��ǥ

	void LeverUpPlayer(Player& player);									// ���� ���� �� ĳ���� ����
	int GetHP()const { return m_ingameInfo.hp; }						// ĳ���� HP ��ȯ
	int GetAttack() const { return m_ingameInfo.attack; }				// ĳ���� ���ݷ� ��ȯ
	int GetSpeed() const { return m_ingameInfo.speed; }					// ĳ���� �̵��ӵ� ��ȯ
	int GetAttackSpeed() const { return m_ingameInfo.attackspeed; }		// ĳ���� ���ݼӵ� ��ȯ
};