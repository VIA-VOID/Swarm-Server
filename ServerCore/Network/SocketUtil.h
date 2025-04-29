#pragma once

/*-------------------------------------------------------
				SocketUtil

- Socket 생성, 바인딩, 옵션설정 등
--------------------------------------------------------*/

class SocketUtil
{
public:
	// WSAStartup
	static bool InitWinSocket();
	// Socket 생성
	static SOCKET CreateSocket();
	// Bind & Listen
	static bool BindAndListenSocket(SOCKET socket, uint16 port);

private:
	// 소켓 옵션 설정
	static bool SetSockOpt(SOCKET socket);
};
