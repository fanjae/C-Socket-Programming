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
	stOverlappedEx m_stSendOverlappedEx; // SEND Overlapped I/O �۾��� ���� ����

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
		unsigned int uiThreadId = 0;
		// WaitingThread Queue�� ��� ���·� ���� ������� ���� ����Ǵ� ���� : (cpu ���� * 2) + 1
		for (int i = 0; i < MAX_WORKERTHREAD; i++)
		{
			mIOWorkerThreads.emplace_back([this]() {
				WokerThread();
			});
		}
		printf("WorkerThread ����..\n");
		return true;
	}

	//accept ��û�� ó���ϴ� Thread ����
	bool CreateAccepterThread()
	{
		mAccepterThread = std::thread([this]() { AccepterThread(); });

		printf("AccepterThread ����..\n");
		return true;
	}

	// ������� �ʴ� Ŭ���̾�Ʈ ���� ����ü�� ��ȯ�Ѵ�.
	stClientInfo* GetEmptyClientInfo()
	{
		for (auto& client : mClientInfos)
		{
			if (INVALID_SOCKET == client.m_socketClient)
			{
				return &client;
			}
		}
		return nullptr;
	}

	// CompletionPort ��ü�� ���ϰ� CompletionKey�� �����Ű�� ����
	bool BindIOCompletionPort(stClientInfo* pClientInfo)
	{
		// socket�� pClientInfo�� CompletionPort ��ü�� �����Ų��.
		auto hIOCP = CreateIoCompletionPort((HANDLE)pClientInfo->m_socketClient, mIOCPHandle, (ULONG_PTR)(pClientInfo), 0);
		if (hIOCP == NULL || mIOCPHandle != hIOCP)
		{
			printf("Error : CreateIoCompletionPort() : %d\n", GetLastError());
		}
		return true;
	}

	// WSARecv Overlapped I/O �۾� 
	bool BindRecv(stClientInfo *pClientInfo)
	{
		DWORD dwFlag = 0;
		DWORD dwRecvNumBytes = 0;

		// Overlapped I/O�� ���� ���� ����
		pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.len = MAX_SOCKBUF;
		pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.buf = pClientInfo->m_stRecvOverlappedEx.m_szBuf;
		pClientInfo->m_stRecvOverlappedEx.m_eOperation = IOOperation::RECV;

		int nRet = WSARecv(pClientInfo->m_socketClient, &(pClientInfo->m_stRecvOverlappedEx.m_wsaBuf), 1, &dwRecvNumBytes, &dwFlag, (LPWSAOVERLAPPED) &(pClientInfo->m_stRecvOverlappedEx), NULL);

		// Socket_error�̸� Client Socket�� �������ɷ� ó��.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("Error : WSARecv() : %d\n", WSAGetLastError());
			return false;
		}
		return true;
	}

	// WSASend Overlapped I/O �۾��� ��Ų��.
	bool SendMsg(stClientInfo * pClientInfo, char* pMsg, int nLen)
	{
		DWORD dwRecvNumBytes = 0;

		// ���۵� �޽����� ����
		CopyMemory(pClientInfo->m_stSendOVerlappedEx.m_szBuf, pMsg, nLen);

		// Overlapped I/O�� �� ������ ������ �ش�.
		pClientInfo->m_stSendOverlappedEx.m_wsaBuf.len = nLen;
		pClientInfo->m_stSendOverlappedEx.m_wsaBuf.buf = pClientInfo->m_stSendOverlappedEx.m_szBuf;
		pClientInfo->m_stSendOverlappedEx.m_eOperation = IOOperation::SEND;

		int nRet = WSASend(pClientInfo->m_socketClient, &(pClientInfo->m_stSendOverlappedEx.m_wsaBuf), 1, &dwRecvNumBytes, 0, (LPWSAOVERLAPPED) &(pClientInfo->m_stSendOverlappedEx), NULL);

		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("Error : WSASend() : %d\n", WSAGetLastError());
			return false;
		}
		return true;
	}

	// Overlapped I/O �۾��� ���� �Ϸ� �뺸�� �޾� �׿� �ش��ϴ� ó���� �ϴ� �Լ�
	void WorkerThread()
	{
		// CompletionKey�� ���� ������ ����
		stClientInfo* pClientInfo = NULL;
		// �Լ� ȣ�� ���� ����
		BOOL bSuccess = TRUE;
		// Overlapped I/O �۾����� ���۵� ������ ũ��
		DWORD dwIoSize = 0;
		// I/O �۾��� ���� ��û�� Overlapped ����ü�� ���� ������
		LPOVERLAPPED lpOverlapped = NULL;

		while (mIsWorkerRun)
		{
			/*
				�� �Լ��� ���� Thread���� WaitinngThread Queue��
				��� ���·� ���� �ȴ�.
				�Ϸ�� Overlapped I/O �۾��� �߻��ϸ� IOCP Queue����
				�Ϸ�� �۾��� ������ �ڸ� ó���Ѵ�.
				�׸��� PostQueueCompletionStatus() �Լ��� ���ؼ� ����� �޽����� �����Ǹ� �����带 �����Ѵ�.
			*/

			bSuccess = GetQueuedCompletionStatus(mIOCPHandle, &dwIoSize, (PULONG_PTR)&pClientInfo, &lpOverlapped, INFINITE);
			// dwIOSize : ���� ���۵� ����Ʈ
			// pClientInfo : CompletionKey
			// lpOverlapped : Overlapped IO ��ü
			// INFINITE : ����� �ð�

			if (bSuccess == TRUE && dwIoSize == 0 && lpOverlapped == NULL)
			{
				mIsWorkerRun = false;
				continue;
			}
			if (lpOverlapped == NULL)
			{
				continue;
			}

			// Client�� ������ ������ ��
			if (bSuccess == FALSE || (dwIoSize == 0 && bSuccess == TRUE))
			{
				printf("Socket(%d) ���� ����\n", (int)pClientInfo->m_socketClient);
				CloseSocket(pClientInfo);
				continue;
			}

			stOverlappedEx *pOverlappedEx = (stOverlappedEx*)lpOverlapped;

			// Overlapped I/O Recv �۾� ��� �� ó��
			if (IOOperation::RECV == pOverlappedEx->m_eOperation)
			{
				pOverlappedEx->m_szBuf[dwIoSize] = NULL;
				printf("[����] bytes : %d, msg : %s\n", dwIoSize, pOverlappedEx->m_szBuf);

				// Client �޽����� �����Ѵ�.
				SendMsg(pClientInfo, pOverlappedEx->m_szBuf, dwIoSize);
				BindRecv(pClientInfo);
			}

			// Overlapped I/O Send �۾� ��� �� ó��
			else if (IOOperation::SEND == pOverlappedEx->m_eOperation)
			{
				printf("[�۽�] bytes : %d, msg : %s\n", dwIoSize, pOverlappedEx->m_szBuf);
			}
			// ���� ó��
			else
			{
				printf("socket(%d) ���� ��Ȳ\n", (int)pClientInfo->m_socketClient);
			}
		}
	}

	// ������� ������ �޴� ������
	void AccepterThread()
	{
		SOCKADDR_IN		stClientAddr;
		int nAddrLen = sizeof(SOCKADDR_IN);

		while (mIsAccepterRun)
		{
			// ������ ���� ����ü�� �ε����� ���´�.
			stClientInfo *pClientInfo = GetEmptyClientInfo();
			if (pClientInfo == NULL)
			{
				printf("Error : Client Full\n");
				return;
			}

			// Client ���� ��û�� ���� �� ������ ��ٸ���.
			pClientInfo->m_socketClient = accept(mListenSock, (SOCKADDR*)&stClientAddr, &nAddrLen);
			if (INVALID_SOCKET == pClientInfo->m_socketClient)
			{
				continue;
			}

			// I/O Completion ��ü�� ������ �����Ų��.
			bool bRet = BindIOCompletionPort(pClientInfo);
			if (bRet == false)
			{
				return;
			}

			// Recv Overlapped I/O �۾��� ��û�� ���´�.
			bRet = BindRecv(pClientInfo);
			if (bRet == false)
			{
				return ;
			}

			char clientIP[32] = { 0, };
			inet_ntop(AF_INET, &(stClientAddr.sin_addr), clientIP, 32 - 1);
			printf("Client ���� : IP(%s) SOCKET(%d)\n", clientIP, (int)pClientInfo->m_socketClient);

			// Client ���� ����
			++mClientCnt;

		}
	}

	// ������ ������ �����Ų��.




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