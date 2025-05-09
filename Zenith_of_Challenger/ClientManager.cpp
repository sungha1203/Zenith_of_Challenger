#include "stdafx.h"
#include "ClientManager.h"

ClientInfo::ClientInfo()
{
}

ClientInfo::ClientInfo(int client_id)
{
    m_ingameInfo.classtype = Classtype::CHALLENGER;     // 초기 직업 : 도전자
    m_ingameInfo.weapon.type = 0;                       // 기본 무기 : 맨손
    m_ingameInfo.weapon.level = 0;                      // 무기 레벨 : 0
    m_ingameInfo.clothes[0] = 0;                        // 기본 머리 : X
    m_ingameInfo.clothes[1] = 0;                        // 기본 하의 : X
    m_ingameInfo.clothes[2] = 0;                        // 기본 하의 : X
    m_ingameInfo.hp = 50;                               // 도전자 체력       : 50
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

//void ClientInfo::SetRoomNum(const int room_id)
//{
//    m_roomNum = room_id;
//}

//void ClientInfo::SetRoomIdx(const int room_idx)
//{
//    m_roomidx = room_idx;
//}

void ClientInfo::SetSpawnCoord(int idx)
{
    switch (idx) {
    case 0:
        m_ingameInfo.x = -172.79f;
        m_ingameInfo.y = 0.21f;
        m_ingameInfo.z = 77.81f;
        break;
    case 1:
        m_ingameInfo.x = -164.28f;
        m_ingameInfo.y = 0.21f;
        m_ingameInfo.z = 83.06f;
        break;
    case 2:
        m_ingameInfo.x = -155.07f;
        m_ingameInfo.y = 0.21f;
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
        m_ingameInfo.x = -570.f;
        m_ingameInfo.y = 44.f;
        m_ingameInfo.z = -20.f;
        break;
    case 1:
        //m_ingameInfo.x = -151.f;
        //m_ingameInfo.y = 0.21f;
        //m_ingameInfo.z = 153.f;
        m_ingameInfo.x = -560.f;
        m_ingameInfo.y = 44.f;
        m_ingameInfo.z = -20.f;
        break;
    case 2:
        //m_ingameInfo.x = -158.f;
        //m_ingameInfo.y = 0.21f;
        //m_ingameInfo.z = 131.f;
        m_ingameInfo.x = -550.f;
        m_ingameInfo.y = 44.f;
        m_ingameInfo.z = -20.f;
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

void ClientInfo::SetClothes(const int clothes[3])
{
    memcpy(m_ingameInfo.clothes, clothes, sizeof(int) * 3);
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

void ClientInfo::LeverUpPlayer(Player& player)
{
    switch (player.classtype) {
    case Classtype::WARRIOR:
        player.hp = 300;
        player.attack = 50;
        player.speed = 2;
        break;
    case Classtype::MAGE:
        player.hp = 200;
        player.attack = 100;
        player.speed = 2;
        break;
    case Classtype::HEALTANKER:
        player.hp = 1000;
        player.attack = 0;
        player.speed = 1;
        break;
    }
}
