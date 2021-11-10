#pragma once
#pragma comment(lib,"ws2_32")
#include <WinSock2.h>
#include <WS2tcpip.h>

#include <thread>
#include <vector>

#define MAX_SOCKBUF 1024
#define MAX_WORKTHREAD 4

enum class IOOperation
{
	RECV,
	SEND
};

// WSAOVERLAPPED 구조체를 확장 시켜서 필요한 정보를 넣은 상태
struct stOverlappedEx
{
	WSAOVERLAPPED m_wsaOverlapped; // Overlapped I/O 구조체
	SOCKET		  m_socketClient;  // Client Socket
	WSABUF		  m_wsaBuf;		   // Overlapped I/O 작업 Buffer
	char		  m_szBuf[MAX_SOCKBUF]; // Data Buffer
	IOOperation   m_eOperation;		// 작업 동작 종류
};

// Client 정보를 담기위한 구조체
struct stClientInfo
{
	SOCKET		  m_socketClient;  // Client와 연결되는 소켓
	stOverlappedEx m_stRecvOverlappedEx; // RECV Overlapped I/O 작업을 위한 변수
	stOverlappedEx m_stSendOVerlappedEx; // SEND Overlapped I/O 작업을 위한 변수

	stClientInfo()
	{
		ZeroMemory(&m_stRecvOverlappedEx, sizeof(stOverlappedEx));
		ZeroMemory(&m_stSendOverlappedEx, sizeof(stOVerlappedEx));
		m_socketClient = INVALID_SOCKET;
	}
};

class IOCompletionPort
{

}