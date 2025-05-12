#pragma once
#include "stdafx.h"
#include "protocol.h"

enum COMP_TYPE{ OP_ACCEPT, OP_RECV, OP_SEND };

class OVER_EXP{
public:
	WSAOVERLAPPED		_over;
	WSABUF				_wsabuf;
	char				_send_buf[BUF_SIZE];
	COMP_TYPE			_comp_type;
public:
	OVER_EXP(){
		_wsabuf.len = BUF_SIZE;
		_wsabuf.buf = _send_buf;
		_comp_type = OP_RECV;
		ZeroMemory(&_over, sizeof(_over));
	}
	OVER_EXP(const char * packet, size_t packet_size){
		_wsabuf.len = static_cast<ULONG>( packet_size );
		_wsabuf.buf = _send_buf;
		_comp_type = OP_SEND;
		ZeroMemory(&_over, sizeof(_over));
		memcpy(_send_buf, packet, packet_size);
	}
};

class SESSION{
public:
	SESSION();
	~SESSION();

	void do_recv();

	void RecvComplete(DWORD ioByte);

	template <typename T>
	void do_send(const T & packet);

public:
	int				m_id;							// 클라이언트 idx
	bool			m_used;							// 사용중 여부
	SOCKET			m_socket;						// 소켓
	std::string		m_login_id;						// 로그인된 ID (임시)

	OVER_EXP		m_recv_over;
	uint32_t				m_prev_remain;
};

template <typename T>
void SESSION::do_send(const T & packet){
	OVER_EXP * send_over = new OVER_EXP(reinterpret_cast<const char *>( &packet ), sizeof(T));

	DWORD send_bytes = 0;
	int ret = WSASend(m_socket, &send_over->_wsabuf, 1, &send_bytes, 0, &send_over->_over, NULL);

	if(ret != 0){
		if(WSAGetLastError() != WSA_IO_PENDING){
			std::cout << "[ERROR] WSASend 실패! 클라이언트[" << m_id << "] 전송 오류" << std::endl;
			delete send_over;
		}
	}
}
