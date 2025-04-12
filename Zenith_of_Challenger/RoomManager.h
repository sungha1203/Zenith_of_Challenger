#pragma once
#include "stdafx.h"
#include "room.h"
#include "network.h"

#define MAX_ROOMS 3
#define MAX_ROOM_CAPACITY 3


// 클라 방 연결 관리
class RoomManager {
private:
    Room                            m_rooms[MAX_ROOMS];                 // 방 번호 → Room 객체 매핑
    std::unordered_map<int, int>    m_client_room;                      // 클라이언트 ID -> 방 번호 매핑

public:
    RoomManager();                                                      // 방이 몇번째 방인지 넘버링

    bool        JoinRoom(int client_id, int room_id);                   // 클라이언트가 방에 입장 시도
    void        LeaveRoom(int client_id);                               // 클라이언트가 방을 떠날 때
    int         GetRoomID(int client_id);                               // 클라이언트가 현재 어떤 방에 있는지 조회
    Room&       GetRoom(int room_id) { return m_rooms[room_id]; }       // 게임방 반환   
};