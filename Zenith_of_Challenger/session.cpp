#include "stdafx.h"
#include "session.h"
#include "network.h"
#include <cassert>

SESSION::SESSION(){
	m_id = -1;
	m_used = false;
	m_socket = INVALID_SOCKET;
	m_prev_remain = 0;
}

SESSION::~SESSION(){
	if(m_socket != INVALID_SOCKET){
		closesocket(m_socket);
	}
}

void SESSION::do_recv(){
	ZeroMemory(&m_recv_over._over, sizeof(m_recv_over._over));
	m_recv_over._wsabuf.len = BUF_SIZE - m_prev_remain;
	m_recv_over._wsabuf.buf = m_recv_over._send_buf + m_prev_remain;
	m_recv_over._comp_type = OP_RECV;

	DWORD flags = 0;
	DWORD recv_bytes = 0;
	int ret = WSARecv(m_socket, &m_recv_over._wsabuf, 1, &recv_bytes, &flags, &m_recv_over._over, NULL);

	if(ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING){
		std::cout << "[ERROR] WSARecv 실패! 클라이언트[" << m_id << "] 종료" << std::endl;
		closesocket(m_socket);
	}
}

void SESSION::RecvComplete(DWORD ioByte){
	size_t remainSize = ioByte + static_cast<uint64_t> ( m_prev_remain );
	char * bufferPosition = m_recv_over._send_buf;
	while(remainSize >= 5){
		int size = 0;
		memcpy_s(&size, 4, &bufferPosition[1], 4);
		if(remainSize < size){
			break;
		}
		Network::HandlePacket(m_id, bufferPosition, size);
		remainSize -= size;
		bufferPosition = bufferPosition + size;
	}
	m_prev_remain = static_cast<uint32_t>(remainSize);
	if(remainSize > 0){
		std::memcpy(m_recv_over._send_buf, bufferPosition, remainSize);
	}
	do_recv();
}
