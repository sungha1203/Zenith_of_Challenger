#include "RoomManager.h"

extern std::unordered_map<int, ClientInfo> g_client;

RoomManager::RoomManager() 
{
    for (int i = 0; i < MAX_ROOMS; ++i) {
        m_rooms[i].Init(i);
    }
}

// Ŭ���̾�Ʈ�� �濡 ���� �õ�
bool RoomManager::JoinRoom(int client_id, int room_id) 
{
    if (room_id < 0 || room_id >= MAX_ROOMS) {
        std::cout << "[ERROR] �߸��� �� ��ȣ ��û: " << room_id << std::endl;
        return false;
    }

    if (m_rooms[room_id].GetClientsNum() >= MAX_ROOM_CAPACITY) {
        std::cout << "[ERROR] �� [" << room_id << "]�� ���� ��." << std::endl;
        return false;
    }

    m_rooms[room_id].AddClient(client_id);                                  // ����濡 Ŭ�� �����߾��
    m_client_room[client_id] = room_id;                                     // ����濡 � Ŭ�� �����ߴ���

    //g_network.SendRoomInfo(room_id, m_rooms[room_id].GetClientsNum());    // ��� Ŭ������ ���ӹ� ���� �ѷ��ֱ�

    std::cout << "[INFO] Ŭ���̾�Ʈ [" << client_id << "] �� [" << room_id << "] ���� ���� (" << m_rooms[room_id].GetClientsNum() << " / 3)" << std::endl;
    return true;
}

// Ŭ���̾�Ʈ�� ���� ���� ��
void RoomManager::LeaveRoom(int client_id) {
    if (m_client_room.find(client_id) == m_client_room.end()) return;

    int room_id = m_client_room[client_id];
    m_rooms[room_id].RemoveClient(client_id);
    m_client_room.erase(client_id);
    g_client.erase(client_id);
    
    //g_network.SendRoomInfo(room_id, m_rooms[room_id].GetClientsNum());      // ��� Ŭ������ ���ӹ� ���� �ѷ��ֱ�

    std::cout << "[INFO] Ŭ���̾�Ʈ [" << client_id << "] �� [" << room_id << "] ����." << std::endl;
}

// Ŭ���̾�Ʈ�� ���� � �濡 �ִ��� ��ȸ
int RoomManager::GetRoomID(int client_id) {
    if (m_client_room.find(client_id) != m_client_room.end()) {
        return m_client_room[client_id];
    }
    return -1;  // �濡 �������� ����
}
