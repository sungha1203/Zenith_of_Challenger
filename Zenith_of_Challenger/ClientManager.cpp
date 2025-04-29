#include "stdafx.h"
#include "ClientManager.h"

ClientInfo::ClientInfo()
{
}

ClientInfo::ClientInfo(int client_id, int roomNum)
{
    m_ingameInfo.classtype = Classtype::CHALLENGER;     // �ʱ� ���� : ������
    m_ingameInfo.weapon.type = 0;                       // �⺻ ���� : �Ǽ�
    m_ingameInfo.weapon.level = 1;                      // ���� ���� : 1
    m_ingameInfo.clothes[0] = 0;                        // �⺻ �Ӹ� : X
    m_ingameInfo.clothes[1] = 0;                        // �⺻ ���� : X
    m_ingameInfo.clothes[2] = 0;                        // �⺻ ���� : X
    m_ingameInfo.hp = 50;                               // ������ ü��       : 50
    m_ingameInfo.attack = 20;                           // ������ ���ݷ�     : 20
    m_ingameInfo.speed = 1;                             // ������ �̵� �ӵ�  : 1
    m_ingameInfo.attackspeed = 1;                       // ������ ���� �ӵ�  : 1
    m_ingameInfo.z = 0;                                 // ���� �������� ������ x
    m_ingameInfo.x = 0;                                 // ���� �������� ������ y
    m_ingameInfo.y = 0;                                 // ���� �������� ������ z
    m_id = client_id;                                   // Ŭ���̾�Ʈ id
    m_roomNum = roomNum;                                // ���ӹ� ��ȣ
}

ClientInfo::~ClientInfo()
{
}

void ClientInfo::SetRoomNum(const int room_id)
{
    m_roomNum = room_id;
}

void ClientInfo::SetRoomIdx(const int room_idx)
{
    m_roomidx = room_idx;
}

void ClientInfo::SetSpawnCoord(int idx)
{
    switch (idx) {
    case 0:
        m_ingameInfo.x = 181.01f;
        m_ingameInfo.y = 0.21f;
        m_ingameInfo.z = -186.98f;
        break;
    case 1:
        m_ingameInfo.x = 195.10f;
        m_ingameInfo.y = 0.21f;
        m_ingameInfo.z = -183.04f;
        break;
    case 2:
        m_ingameInfo.x = 193.70f;
        m_ingameInfo.y = 0.21f;
        m_ingameInfo.z = -199.20f;
        break;
    default:
        break;
    }
}

void ClientInfo::SetRepairCoord(int idx)
{
    switch (idx) {
    case 0:
        m_ingameInfo.x = -168.f;
        m_ingameInfo.y = 0.21f;
        m_ingameInfo.z = 140.f;
        break;
    case 1:
        m_ingameInfo.x = -151.f;
        m_ingameInfo.y = 0.21f;
        m_ingameInfo.z = 153.f;
        break;
    case 2:
        m_ingameInfo.x = -158.f;
        m_ingameInfo.y = 0.21f;
        m_ingameInfo.z = 131.f;
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

void ClientInfo::SetClothes(const int clothes[3])
{
    memcpy(m_ingameInfo.clothes, clothes, sizeof(int) * 3);
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
