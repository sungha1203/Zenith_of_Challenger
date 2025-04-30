#include "RoomManager.h"

extern std::unordered_map<int, ClientInfo> g_client;

RoomManager::RoomManager() 
{
    for (int i = 0; i < MAX_ROOMS; ++i) {
        m_rooms[i].Init(i);
    }
}

// 클라이언트가 방에 입장 시도
bool RoomManager::JoinRoom(int client_id, int room_id) 
{
    if (room_id < 0 || room_id >= MAX_ROOMS) {
        std::cout << "[ERROR] 잘못된 방 번호 요청: " << room_id << std::endl;
        return false;
    }

    if (m_rooms[room_id].GetClientsNum() >= MAX_ROOM_CAPACITY) {
        std::cout << "[ERROR] 방 [" << room_id << "]이 가득 참." << std::endl;
        return false;
    }

    m_rooms[room_id].AddClient(client_id);                                  // 몇번방에 클라가 입장했어요
    m_client_room[client_id] = room_id;                                     // 몇번방에 어떤 클라가 입장했는지

    //g_network.SendRoomInfo(room_id, m_rooms[room_id].GetClientsNum());    // 모든 클라한테 게임방 상태 뿌려주기

    std::cout << "[INFO] 클라이언트 [" << client_id << "] 방 [" << room_id << "] 입장 성공 (" << m_rooms[room_id].GetClientsNum() << " / 3)" << std::endl;
    return true;
}

// 클라이언트가 방을 떠날 때
void RoomManager::LeaveRoom(int client_id) {
    if (m_client_room.find(client_id) == m_client_room.end()) return;

    int room_id = m_client_room[client_id];
    m_rooms[room_id].RemoveClient(client_id);
    m_client_room.erase(client_id);
    g_client.erase(client_id);
    
    //g_network.SendRoomInfo(room_id, m_rooms[room_id].GetClientsNum());      // 모든 클라한테 게임방 상태 뿌려주기

    std::cout << "[INFO] 클라이언트 [" << client_id << "] 방 [" << room_id << "] 퇴장." << std::endl;
}

// 클라이언트가 현재 어떤 방에 있는지 조회
int RoomManager::GetRoomID(int client_id) {
    if (m_client_room.find(client_id) != m_client_room.end()) {
        return m_client_room[client_id];
    }
    return -1;  // 방에 입장하지 않음
}
