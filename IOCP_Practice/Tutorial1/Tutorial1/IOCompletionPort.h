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

// WSAOVERLAPPED ����ü�� Ȯ�� ���Ѽ� �ʿ��� ������ ���� ����
struct stOverlappedEx
{
	WSAOVERLAPPED m_wsaOverlapped; // Overlapped I/O ����ü
	SOCKET		  m_socketClient;  // Client Socket
	WSABUF		  m_wsaBuf;		   // Overlapped I/O �۾� Buffer
	char		  m_szBuf[MAX_SOCKBUF]; // Data Buffer
	IOOperation   m_eOperation;		// �۾� ���� ����
};

// Client ������ ������� ����ü
struct stClientInfo
{
	SOCKET		  m_socketClient;  // Client�� ����Ǵ� ����
	stOverlappedEx m_stRecvOverlappedEx; // RECV Overlapped I/O �۾��� ���� ����
	stOverlappedEx m_stSendOVerlappedEx; // SEND Overlapped I/O �۾��� ���� ����

	stClientInfo()
	{
		ZeroMemory(&m_stRecvOverlappedEx, sizeof(stOverlappedEx));
		ZeroMemory(&m_stSendOverlappedEx, sizeof(stOVerlappedEx));
		m_socketClient = INVALID_SOCKET;
	}
};

class IOCompletionPort
{
public:
	IOCompletionPort(void) {}

	~IOCompletionPort(void)
	{
		// ����� ����
		WSACleanup();
	}

	// ���� �ʱ�ȭ �Լ�
	bool InitSocket()
	{
		WSADATA wsaData;

		int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (nRet != 0)
		{
			printf("Error : WSAStartup() Error : %d\n", WSAGetLastError());
			return false;
		}

		// ���������� TCP, Overlapped I/O Socket ����
		mListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);

		if (INVALID_SOCKET == mListenSocket)
		{
			printf("Error : socket() Error : %d\n", WSAGetLastError());
			return false;
		}
		printf("���� �ʱ�ȭ ����\n");
		return true;
	}
}
std::vector<stClientInfo> mClientInfos;
SOCKET mListenSocket = INVALID_SOCKET;