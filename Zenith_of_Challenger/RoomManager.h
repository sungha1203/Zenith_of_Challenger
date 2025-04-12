#pragma once
#include "stdafx.h"
#include "room.h"
#include "network.h"

#define MAX_ROOMS 3
#define MAX_ROOM_CAPACITY 3


// Ŭ�� �� ���� ����
class RoomManager {
private:
    Room                            m_rooms[MAX_ROOMS];                 // �� ��ȣ �� Room ��ü ����
    std::unordered_map<int, int>    m_client_room;                      // Ŭ���̾�Ʈ ID -> �� ��ȣ ����

public:
    RoomManager();                                                      // ���� ���° ������ �ѹ���

    bool        JoinRoom(int client_id, int room_id);                   // Ŭ���̾�Ʈ�� �濡 ���� �õ�
    void        LeaveRoom(int client_id);                               // Ŭ���̾�Ʈ�� ���� ���� ��
    int         GetRoomID(int client_id);                               // Ŭ���̾�Ʈ�� ���� � �濡 �ִ��� ��ȸ
    Room&       GetRoom(int room_id) { return m_rooms[room_id]; }       // ���ӹ� ��ȯ   
};