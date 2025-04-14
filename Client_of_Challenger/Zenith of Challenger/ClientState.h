#pragma once
#include <iostream>

class ClientState
{
public:
	ClientState();
	~ClientState();
	
	void SetClientId();
	void SetClientRoomNum(int RoomNum);

	void	SetIsLogin(bool)			{ m_IsLogin = true; }

	int		GetClientId() const			{ return m_clientId; }
	int		GetClientRoomNum() const	{ return m_roomNum;  }
	bool	GetIsLogin() const			{ return m_IsLogin; }

private:
	//std::string		IDnPW;		// ID & PW
	int				m_clientId;		// 클라이언트 ID
	int				m_roomNum;		// 방 번호
	bool			m_IsLogin;		// 로그인 성공 여부
};