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
	int				m_clientId;		// Ŭ���̾�Ʈ ID
	int				m_roomNum;		// �� ��ȣ
	bool			m_IsLogin;		// �α��� ���� ����
};