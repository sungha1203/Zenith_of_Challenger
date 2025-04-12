#include "stdafx.h"
#include "session.h"
#include "network.h"

SESSION::SESSION()
{
	m_id = -1;
	m_used = false;
	m_socket = INVALID_SOCKET;
	m_prev_remain = 0;
}

SESSION::~SESSION()
{
	if (m_socket != INVALID_SOCKET) {
		closesocket(m_socket);
	}
}

void SESSION::do_recv()
{
	ZeroMemory(&m_recv_over._over, sizeof(m_recv_over._over));
	m_recv_over._wsabuf.len = BUF_SIZE;
	m_recv_over._wsabuf.buf = m_recv_over._send_buf;
	m_recv_over._comp_type = OP_RECV;

	DWORD flags = 0;
	DWORD recv_bytes = 0;
	int ret = WSARecv(m_socket, &m_recv_over._wsabuf, 1, &recv_bytes, &flags, &m_recv_over._over, NULL);

	if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
		std::cout << "[ERROR] WSARecv 실패! 클라이언트[" << m_id << "] 종료" << std::endl;
		closesocket(m_socket);
	}
	if (recv_bytes > 0) {
		g_network.HandlePacket(m_id, m_recv_over._send_buf, recv_bytes);
	}
}