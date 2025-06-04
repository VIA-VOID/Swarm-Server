#include "pch.h"
#include "SocketUtil.h"
#include "Log/LogManager.h"

// WSAStartup
bool SocketUtil::InitWinSocket()
{
	WSADATA wsaData;
	return (::WSAStartup(MAKEWORD(2, 2), &wsaData) == 0);
}

// Socket 생성
SOCKET SocketUtil::CreateSocket()
{
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

// Bind
bool SocketUtil::BindSocket(SOCKET socket, uint16 port)
{
	SOCKADDR_IN addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = ::htons(port);
	// 소켓 옵션 설정
	if (SetSockOpt(socket) == false)
	{
		return false;
	}
	// IP/PORT 바인딩
	if (::bind(socket, reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr)) == SOCKET_ERROR)
	{
		return false;
	}

	return true;
}

// Listen
bool SocketUtil::ListenSocket(SOCKET socket)
{
	// 수신대기
	if (::listen(socket, SOMAXCONN) == SOCKET_ERROR)
	{
		LOG_ERROR("Socket Listen 실패, closeSocket: " + std::to_string(static_cast<int64>(socket)));
		::closesocket(socket);
		return false;
	}
	return true;
}

// 소켓 옵션 설정
bool SocketUtil::SetSockOpt(SOCKET socket)
{
	// 지역 주소 재사용 허용
	BOOL reuse = TRUE;
	if (::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&reuse), sizeof(reuse)) == SOCKET_ERROR)
	{
		LOG_ERROR("SocketOption SO_REUSEADDR 설정 실패, Socket: " + std::to_string(static_cast<int64>(socket)));
		return false;
	}
	// linger 설정
	// closesocket() 즉시 리턴, 송신버퍼 데이터 삭제 및 강제종료
	LINGER linger = { 1, 0 };
	if (::setsockopt(socket, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&linger), sizeof(linger)) == SOCKET_ERROR)
	{
		LOG_ERROR("SocketOption SO_LINGER 설정 실패, Socket: " + std::to_string(static_cast<int64>(socket)));
		return false;
	}
	// Nagle 알고리즘 OFF
	BOOL nagleOff = TRUE;
	if (::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&nagleOff), sizeof(nagleOff)) == SOCKET_ERROR)
	{
		LOG_ERROR("SocketOption TCP_NODELAY 설정 실패, Socket: " + std::to_string(static_cast<int64>(socket)));
		return false;
	}
	return true;
}
