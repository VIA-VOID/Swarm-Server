#include "pch.h"
#include "SocketUtil.h"
#include "Log/LogManager.h"
#include "Utils/Utils.h"

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

// Bind & Listen
bool SocketUtil::BindAndListenSocket(SOCKET socket, uint16 port)
{
	// 소켓 옵션 설정
	if (SetSockOpt(socket) == false)
	{
		return false;
	}
	// bind
	SOCKADDR_IN serverAddr = {};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = ::htons(port);

	// IP/PORT 바인딩
	if (::bind(socket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
	{
		return false;
	}

	// 수신대기
	if (::listen(socket, SOMAXCONN) == SOCKET_ERROR)
	{
		LOG_ERROR(L"Socket Listen 실패, closeSocket: " + Utils::ToWString(static_cast<int64>(socket)));
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
		LOG_ERROR(L"SocketOption SO_REUSEADDR 설정 실패, Socket: " + Utils::ToWString(static_cast<int64>(socket)));
		return false;
	}
	// linger 설정
	// closesocket() 즉시 리턴, 송신버퍼 데이터 삭제 및 강제종료
	LINGER linger = { 1, 0 };
	if (::setsockopt(socket, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&linger), sizeof(linger)) == SOCKET_ERROR)
	{
		LOG_ERROR(L"SocketOption SO_LINGER 설정 실패, Socket: " + Utils::ToWString(static_cast<int64>(socket)));
		return false;
	}
	// Nagle 알고리즘 OFF
	BOOL nagleOff = TRUE;
	if (::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&nagleOff), sizeof(nagleOff)) == SOCKET_ERROR)
	{
		LOG_ERROR(L"SocketOption TCP_NODELAY 설정 실패, Socket: " + Utils::ToWString(static_cast<int64>(socket)));
		return false;
	}
	return true;
}
