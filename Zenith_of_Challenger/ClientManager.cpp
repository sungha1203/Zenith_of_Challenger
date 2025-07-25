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
	m_ingameInfo.hp = 100;                              // 도전자 체력       : 50
	m_ingameInfo.attack = 20;                           // 도전자 공격력     : 20
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

int ClientInfo::MinusHP(int damage, int gamemode)
{
	if (m_ingameInfo.hp > 0)
		m_ingameInfo.hp -= damage;
	if (m_ingameInfo.hp < 0) { // 피0이 되면
		m_ingameInfo.hp = 100;
		g_network.SendPlayerHP(GetID());

		if (gamemode == 1) {
			// 클라에서 죽는 애니메이션 나온 뒤 3초뒤에 부활
			m_ingameInfo.x = -172.79f;
			m_ingameInfo.y = 0.1f;
			m_ingameInfo.z = 77.81f;
			return 1;
		}
		else if (gamemode == 3) {
			// 클라에서 죽는 애니메이션 나온 뒤 3초뒤에 부활
			m_ingameInfo.x = 557.f;
			m_ingameInfo.y = 44.f;
			m_ingameInfo.z = -11.f;
			return 1;
		}
	}

	g_network.SendPlayerHP(GetID());
	return 0;
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