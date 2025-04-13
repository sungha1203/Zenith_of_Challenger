#pragma once
#include <iostream>

class ClientState
{
public:
	ClientState();
	~ClientState();
	
	void SetClientId();
	void SetClientRoomNum();

	int GetClientId() const { return client_id; }
	int GetClientRoomNum()const { return room_num;  }

private:
	std::string		IDnPW;			// ID & PW
	int				client_id;		// 클라이언트 ID
	int				room_num;		// 방 번호
};