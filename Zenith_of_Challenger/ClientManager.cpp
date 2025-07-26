#include "stdafx.h"
#include "ClientManager.h"

ClientInfo::ClientInfo()
{
}

ClientInfo::ClientInfo(int client_id)
{
	m_ingameInfo.clientID = client_id;                  // 아이디
	m_ingameInfo.classtype = Classtype::CHALLENGER;     // 초기 직업 : 도전자
	m_ingameInfo.weapon.type = 0;                       // 기본 무기 : 맨손
	m_ingameInfo.weapon.level = 0;                      // 무기 레벨 : 0
	m_ingameInfo.clothes = 0;							// 기본 머리 : X
	m_ingameInfo.hp = 100;                              // 도전자 체력       : 100
	m_ingameInfo.attack = 30;                           // 도전자 공격력     : 30
	m_ingameInfo.speed = 1;                             // 도전자 이동 속도  : 1
	m_ingameInfo.attackspeed = 1;                       // 도전자 공격 속도  : 1
	m_ingameInfo.z = 0;                                 // 도전 스테이지 리스폰 x
	m_ingameInfo.x = 0;                                 // 도전 스테이지 리스폰 y
	m_ingameInfo.y = 0;                                 // 도전 스테이지 리스폰 z
}

ClientInfo::~ClientInfo()
{
}

void ClientInfo::SetSpawnCoord(int idx)
{
	switch (idx) {
	case 0:
		m_ingameInfo.x = -172.79f;
		m_ingameInfo.y = 0.1f;
		m_ingameInfo.z = 77.81f;
		break;
	case 1:
		m_ingameInfo.x = -164.28f;
		m_ingameInfo.y = 0.1f;
		m_ingameInfo.z = 83.06f;
		break;
	case 2:
		m_ingameInfo.x = -155.07f;
		m_ingameInfo.y = 0.1f;
		m_ingameInfo.z = 88.75f;
		break;
	default:
		break;
	}
}

void ClientInfo::SetRepairCoord(int idx)
{
	switch (idx) {
	case 0:
		//m_ingameInfo.x = -168.f;
		//m_ingameInfo.y = 0.21f;
		//m_ingameInfo.z = 140.f;
		m_ingameInfo.x = -580.f;
		m_ingameInfo.y = 44.f;
		m_ingameInfo.z = -13.f;
		break;
	case 1:
		//m_ingameInfo.x = -151.f;
		//m_ingameInfo.y = 0.21f;
		//m_ingameInfo.z = 153.f;
		m_ingameInfo.x = -568.f;
		m_ingameInfo.y = 44.f;
		m_ingameInfo.z = 4.f;
		break;
	case 2:
		//m_ingameInfo.x = -158.f;
		//m_ingameInfo.y = 0.21f;
		//m_ingameInfo.z = 131.f;
		m_ingameInfo.x = -559.f;
		m_ingameInfo.y = 44.f;
		m_ingameInfo.z = -12.f;
		break;
	default:
		break;
	}
}

void ClientInfo::SetZenithCoord(int idx)
{
	switch (idx) {
	case 0:
		m_ingameInfo.x = 557.f;
		m_ingameInfo.y = 44.f;
		m_ingameInfo.z = -11.f;
		break;
	case 1:
		m_ingameInfo.x = 562.f;
		m_ingameInfo.y = 44.f;
		m_ingameInfo.z = 9.f;
		break;
	case 2:
		m_ingameInfo.x = 573.f;
		m_ingameInfo.y = 44.f;
		m_ingameInfo.z = -7.f;
		break;
	default:
		break;
	}
}

void ClientInfo::SetCoord(float x, float y, float z)
{
	m_ingameInfo.x = x;
	m_ingameInfo.y = y;
	m_ingameInfo.z = z;
}

void ClientInfo::SetAngle(float angle)
{
	m_ingameInfo.angle = angle;
}

void ClientInfo::SetClothes(const int clothes)
{
	m_ingameInfo.clothes = clothes;
}

void ClientInfo::SetJobType(int JobNum)
{
	m_ingameInfo.classtype = (Classtype)(JobNum - 3);
}

void ClientInfo::SetWeapon(int weaponNum)
{
	m_ingameInfo.weapon.type = weaponNum + 1;
}

void ClientInfo::SetWeaponGrade()
{
	bool res = SetEnhanceGradeUp(GetWeaponGrade());
	if (res) {
		++m_ingameInfo.weapon.level;                    // 성공했으므로 강화 1업
	}
}

bool ClientInfo::SetEnhanceGradeUp(int weapongrade)
{
	int probabilities[] = { 100, 90, 81, 64, 50, 26, 15, 7, 5 };
	// 0 -> 1 : 100%   |   3 -> 4 : 64%   |   6 -> 7 : 15%
	// 1 -> 2 : 90%    |   4 -> 5 : 50%   |   7 -> 8 : 7%
	// 2 -> 3 : 81%    |   5 -> 6 : 26%   |   8 -> 9 : 5%

	std::random_device rd;
	std::default_random_engine dre{ rd() };
	std::uniform_int_distribution<int> uid{ 1, 100 };

	int roll = uid(dre);
	return roll <= probabilities[weapongrade];
}

void ClientInfo::SetAttack()
{
//  [조합]
	{
		// 	전직서 없으면(도전자)         -> 30 고정
		//  무기 전직서 조합 맞으면       -> + 0
		//	무기 없으면                   -> - 50
		//	무기 전직서 조합 안맞으면     -> - 20
		//	무기 + 1                      -> + 5
		//	----------------------
		//	도전자
		//	평타 공격력 -> 30
		//	스킬 공격력 -> 30
		//  ----------------------
		//	전사
		//	평타 공격력 -> 100
		//	스킬 공격력 -> 150
		//  ----------------------
		//	법사
		//	평타 공격력 -> 70
		//	스킬 공격력 -> 120
		//  ----------------------
		//	힐탱커
		//	평타 공격력 -> 0
		//	스킬 공격력 -> 0
	}
	switch ((int)m_ingameInfo.classtype) {
	// 전직 못했을 때(도전자)
	case 0:						
	{
		m_normalAttack = 30 + m_ingameInfo.weapon.level * 5;
		m_normalAttack = 30 + m_ingameInfo.weapon.level * 5;
		break;
	}
	// 전사
	case 1:						
	{
		if ((int)m_ingameInfo.weapon.type == 0) {		// 주먹
			m_normalAttack = 100 + m_ingameInfo.weapon.level * 5 - 50;
			m_normalAttack = 150 + m_ingameInfo.weapon.level * 5 - 50;
		}
		else if ((int)m_ingameInfo.weapon.type == 1) {	// 칼
			m_normalAttack = 100 + m_ingameInfo.weapon.level * 5;
			m_normalAttack = 150 + m_ingameInfo.weapon.level * 5;
		}
		else {											// 그 외
			m_normalAttack = 100 + m_ingameInfo.weapon.level * 5 - 20;
			m_normalAttack = 150 + m_ingameInfo.weapon.level * 5 - 20;
		}
		break;
	}
	// 법사
	case 2:
	{
		if ((int)m_ingameInfo.weapon.type == 0) {		// 주먹
			m_normalAttack = 70 + m_ingameInfo.weapon.level * 5 - 50;
			m_normalAttack = 120 + m_ingameInfo.weapon.level * 5 - 50;
		}
		else if ((int)m_ingameInfo.weapon.type == 2) {	// 지팡이
			m_normalAttack = 70 + m_ingameInfo.weapon.level * 5;
			m_normalAttack = 120 + m_ingameInfo.weapon.level * 5;
		}
		else {											// 그 외
			m_normalAttack = 70 + m_ingameInfo.weapon.level * 5 - 20;
			m_normalAttack = 120 + m_ingameInfo.weapon.level * 5 - 20;
		}
		break;
	}
	// 힐탱커
	case 3:						
	{
		m_normalAttack = 0;
		m_normalAttack = 0;
		break;
	}
	}
}

void ClientInfo::LeverUpPlayer(int classtype)
{
	switch (classtype) {
	case (int)Classtype::WARRIOR:
		m_ingameInfo.hp = 300;
		m_ingameInfo.attack = 50;
		m_ingameInfo.speed = 2;
		break;
	case (int)Classtype::MAGE:
		m_ingameInfo.hp = 200;
		m_ingameInfo.attack = 100;
		m_ingameInfo.speed = 2;
		break;
	case (int)Classtype::HEALTANKER:
		m_ingameInfo.hp = 1000;
		m_ingameInfo.attack = 0;
		m_ingameInfo.speed = 1;
		break;
	}
}

void ClientInfo::AddHP(int heal)
{
	if (m_ingameInfo.hp < 100)
		m_ingameInfo.hp += heal;
	if (m_ingameInfo.hp > 100) // 피100이 넘으면
		m_ingameInfo.hp = 100;
	g_network.SendPlayerHP(GetID());
}

void ClientInfo::MinusHP(int damage, int gamemode)
{
	if (m_ingameInfo.hp > 0)
		m_ingameInfo.hp -= damage;
	if (m_ingameInfo.hp <= 0) { // 피0이 되면
		switch ((int)GetJobType()) {
		case 0:
			m_ingameInfo.hp = 100;
			break;
		case 1:
			m_ingameInfo.hp = 300;
			break;
		case 2:
			m_ingameInfo.hp = 200;
			break;
		case 3:
			m_ingameInfo.hp = 1000;
			break;
		}
		g_network.SendPlayerHP(GetID());

		if (gamemode == 1) {
			// 클라에서 죽는 애니메이션 나온 뒤 3초뒤에 부활
			m_ingameInfo.x = -172.79f;
			m_ingameInfo.y = 0.1f;
			m_ingameInfo.z = 77.81f;
			g_network.SendPlayerRespone(GetID());
		}
		else if (gamemode == 3) {
			// 클라에서 죽는 애니메이션 나온 뒤 3초뒤에 부활
			m_ingameInfo.x = 557.f;
			m_ingameInfo.y = 44.f;
			m_ingameInfo.z = -11.f;
			g_network.SendPlayerRespone(GetID());
		}
	}

	g_network.SendPlayerHP(GetID());
}

bool ClientInfo::CanUseSkill()
{
	auto now = std::chrono::steady_clock::now();

	if (m_lastSkillTime.time_since_epoch().count() == 0) return true;

	const auto cooldown = std::chrono::seconds(4);
	return now - m_lastSkillTime >= cooldown;
}

void ClientInfo::StartCoolTime()
{
	m_lastSkillTime = std::chrono::steady_clock::now();
}