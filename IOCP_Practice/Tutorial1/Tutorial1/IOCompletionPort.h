#pragma once
#pragma comment(lib,"ws2_32")
#include <WinSock2.h>
#include <WS2tcpip.h>

#include <thread>
#include <vector>

#define MAX_SOCKBUF 1024
#define MAX_WORKERTHREAD 4

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

	// Server �Լ� //
	// Server �ּ����� �� ���ϰ� ���� ��Ű�� ���� ��û�� �ޱ� ���� ������ ����ϴ� �Լ�
	bool BindandListen(int nBindPort)
	{
		SOCKADDR_IN stServerAddr;
		stServerAddr.sin_family = AF_INET;
		stServerAddr.sin_port = htons(nBindPort); // ���� ��Ʈ�� ������.
		// � �ּҿ��� ������ �����̶� �޾Ƶ��δ�.
		// Server�� �Ϲ������� �̷��� ������.
		// Ư�� �����ǿ��� ������ �ް� �ʹٸ� �� �ּҸ� inet_addr �Լ��� �̿��� �־����.
		stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		// ������ ���� �ּ� ������ cIOCompletionPort ���� ����
		int nRet = bind(mListenSocket, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
		if (nRet != 0)
		{
			printf("Error : bind() Error : %d\n", WSAGetLastError());
			return false;
		}

		// ���� ��û�� �ޱ� ���� CIOCompletionPort ���� ���,
		// ���� ���ť 5���� ������.
		nRet = listen(mListenSocket, 5);
		if (nRet != 0)
		{
			printf("Error : listen() Error : %d\n", WSAGetLastError());
			return false;
		}

		printf("���� ��� ����!\n");
		return true;
	}

	// ���� ��û�� �����ϰ� �޽����� �޾Ƽ� ó���ϴ� �Լ�
	bool StartServer(const UINT32 maxClientCount)
	{
		CreateClient(maxClientCount);

		// CompletionPort ��ü ���� ��û.
		mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, MAX_WORKERTHREAD);
		if (mIOCPHandle == NULL)
		{
			printf("Error : CreateIOCompletionPort Error : %d\n", GetLastError());
			return false;
		}

		// ���ӵ� Ŭ���̾�Ʈ �ּ� ������ ������ ����ü
		bool bRet = CreateWorkerThread();
		if (bRet == false)
		{
			return false;
		}
		bRet = CreateAccepterThread();
		if (bRet == false)
		{
			return false;
		}
		printf("���� ����\n");
		return true;
	}

	// �����Ǿ��ִ� ������ �ı�
	void DestoryThread()
	{
		mIsWorkerRun = false;
		CloseHandle(mIOCPHandle);

		for (auto& th : mIOWorkerThreads)
		{
			if (th.joinable())
			{
				th.join();
			}
		}

		//Accepter �����带 ������.
		mIsAccepterRun = false;
		closesocket(mListenSocket);

		if (mAccepterThread.joinable())
		{
			mAccepterThread.join();
		}
	}
private:
	void Createclient(const UINT32 maxClientCount)
	{
		for (UINT32 i = 0; i < maxClientCount; i++)
		{
			mClientInfos.emplace_back();
		}
	}

	// WatingThread Queue���� ����� ������ ����
	bool CreateWorkerThread()
	{

	}
	// Ŭ���̾�Ʈ ������ ������ ����ü 
	std::vector<stClientInfo> mClientInfos;

	// Ŭ���̾�Ʈ ������ �ޱ� ���� listen Socket
	SOCKET mListenSocket = INVALID_SOCKET;

	// ���ӵǾ��ִ� Client ����
	int mClientCnt = 0;

	// IO Worker Thread
	std::vector<std::thread> mIOWorkerThreads;

	// Accept Thread
	std::thread mAccepterThread;

	// CompletionPort ��ü �ڵ�
	HANDLE mIOCPHandle = INVALID_HANDLE_VALUE;

	// �۾� ������ ���� �÷���
	bool mIsWorkerRun = true;

	// ���� ������ ���� �÷���
	bool mIsAccepterRun = true;

	// ���� ����
	char mSocketBuf[1024] = { 0, };
};