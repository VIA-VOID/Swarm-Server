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
	// Bind
	static bool BindSocket(SOCKET socket, uint16 port);
	// Listen
	static bool ListenSocket(SOCKET socket);

private:
	// 소켓 옵션 설정
	static bool SetServerSockOpt(SOCKET socket);
	static bool SetClientSockOpt(SOCKET socket);
	// send & recv 버퍼 사이즈 설정
	static bool SetSendRecvBuffSize(SOCKET socket, int32 buffSize);
};
