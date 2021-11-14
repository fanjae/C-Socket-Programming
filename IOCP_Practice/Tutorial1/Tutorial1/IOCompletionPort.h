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
	stOverlappedEx m_stSendOverlappedEx; // SEND Overlapped I/O 작업을 위한 변수

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
		// 사용을 끝냄
		WSACleanup();
	}

	// 소켓 초기화 함수
	bool InitSocket()
	{
		WSADATA wsaData;

		int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (nRet != 0)
		{
			printf("Error : WSAStartup() Error : %d\n", WSAGetLastError());
			return false;
		}

		// 연결지향형 TCP, Overlapped I/O Socket 생성
		mListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);

		if (INVALID_SOCKET == mListenSocket)
		{
			printf("Error : socket() Error : %d\n", WSAGetLastError());
			return false;
		}
		printf("소켓 초기화 성공\n");
		return true;
	}

	// Server 함수 //
	// Server 주소정보 및 소켓과 연결 시키고 접속 요청을 받기 위한 소켓을 등록하는 함수
	bool BindandListen(int nBindPort)
	{
		SOCKADDR_IN stServerAddr;
		stServerAddr.sin_family = AF_INET;
		stServerAddr.sin_port = htons(nBindPort); // 서버 포트를 설정함.
		// 어떤 주소에서 들어오는 접속이라도 받아들인다.
		// Server는 일반적으로 이렇게 설정함.
		// 특정 아이피에서 접속을 받고 싶다면 그 주소를 inet_addr 함수를 이용해 넣어야함.
		stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		// 지정한 서버 주소 정보와 cIOCompletionPort 소켓 연결
		int nRet = bind(mListenSocket, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
		if (nRet != 0)
		{
			printf("Error : bind() Error : %d\n", WSAGetLastError());
			return false;
		}

		// 접속 요청을 받기 위해 CIOCompletionPort 소켓 등록,
		// 접속 대기큐 5개로 설정함.
		nRet = listen(mListenSocket, 5);
		if (nRet != 0)
		{
			printf("Error : listen() Error : %d\n", WSAGetLastError());
			return false;
		}

		printf("서버 등록 성공!\n");
		return true;
	}

	// 접속 요청을 수락하고 메시지를 받아서 처리하는 함수
	bool StartServer(const UINT32 maxClientCount)
	{
		CreateClient(maxClientCount);

		// CompletionPort 객체 생성 요청.
		mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, MAX_WORKERTHREAD);
		if (mIOCPHandle == NULL)
		{
			printf("Error : CreateIOCompletionPort Error : %d\n", GetLastError());
			return false;
		}

		// 접속된 클라이언트 주소 정보를 저장할 구조체
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
		printf("서버 시작\n");
		return true;
	}

	// 생성되어있는 쓰레드 파괴
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

		//Accepter 쓰레드를 종료함.
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

	// WatingThread Queue에서 대기할 쓰레드 생성
	bool CreateWorkerThread()
	{
		unsigned int uiThreadId = 0;
		// WaitingThread Queue에 대기 상태로 넣을 쓰레드들 생성 권장되는 개수 : (cpu 개수 * 2) + 1
		for (int i = 0; i < MAX_WORKERTHREAD; i++)
		{
			mIOWorkerThreads.emplace_back([this]() {
				WokerThread();
			});
		}
		printf("WorkerThread 시작..\n");
		return true;
	}

	//accept 요청을 처리하는 Thread 생성
	bool CreateAccepterThread()
	{
		mAccepterThread = std::thread([this]() { AccepterThread(); });

		printf("AccepterThread 시작..\n");
		return true;
	}

	// 사용하지 않는 클라이언트 정보 구조체를 반환한다.
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

	// CompletionPort 객체와 소켓과 CompletionKey를 연결시키는 역할
	bool BindIOCompletionPort(stClientInfo* pClientInfo)
	{
		// socket과 pClientInfo를 CompletionPort 객체와 연결시킨다.
		auto hIOCP = CreateIoCompletionPort((HANDLE)pClientInfo->m_socketClient, mIOCPHandle, (ULONG_PTR)(pClientInfo), 0);
		if (hIOCP == NULL || mIOCPHandle != hIOCP)
		{
			printf("Error : CreateIoCompletionPort() : %d\n", GetLastError());
		}
		return true;
	}

	// WSARecv Overlapped I/O 작업 
	bool BindRecv(stClientInfo *pClientInfo)
	{
		DWORD dwFlag = 0;
		DWORD dwRecvNumBytes = 0;

		// Overlapped I/O를 위한 정보 셋팅
		pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.len = MAX_SOCKBUF;
		pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.buf = pClientInfo->m_stRecvOverlappedEx.m_szBuf;
		pClientInfo->m_stRecvOverlappedEx.m_eOperation = IOOperation::RECV;

		int nRet = WSARecv(pClientInfo->m_socketClient, &(pClientInfo->m_stRecvOverlappedEx.m_wsaBuf), 1, &dwRecvNumBytes, &dwFlag, (LPWSAOVERLAPPED) &(pClientInfo->m_stRecvOverlappedEx), NULL);

		// Socket_error이면 Client Socket이 끊어진걸로 처리.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("Error : WSARecv() : %d\n", WSAGetLastError());
			return false;
		}
		return true;
	}

	// WSASend Overlapped I/O 작업을 시킨다.
	bool SendMsg(stClientInfo * pClientInfo, char* pMsg, int nLen)
	{
		DWORD dwRecvNumBytes = 0;

		// 전송될 메시지를 복사
		CopyMemory(pClientInfo->m_stSendOVerlappedEx.m_szBuf, pMsg, nLen);

		// Overlapped I/O를 각 정보를 셋팅해 준다.
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

	// Overlapped I/O 작업에 대한 완료 통보를 받아 그에 해당하는 처리를 하는 함수
	void WorkerThread()
	{
		// CompletionKey를 받을 포인터 변수
		stClientInfo* pClientInfo = NULL;
		// 함수 호출 성공 여부
		BOOL bSuccess = TRUE;
		// Overlapped I/O 작업에서 전송된 데이터 크기
		DWORD dwIoSize = 0;
		// I/O 작업을 위해 요청한 Overlapped 구조체를 받을 포인터
		LPOVERLAPPED lpOverlapped = NULL;

		while (mIsWorkerRun)
		{
			/*
				이 함수로 인해 Thread들은 WaitinngThread Queue에
				대기 상태로 들어가게 된다.
				완료된 Overlapped I/O 작업이 발생하면 IOCP Queue에서
				완료된 작업을 가져와 뒤를 처리한다.
				그리고 PostQueueCompletionStatus() 함수에 의해서 사용자 메시지가 도착되면 쓰레드를 종료한다.
			*/

			bSuccess = GetQueuedCompletionStatus(mIOCPHandle, &dwIoSize, (PULONG_PTR)&pClientInfo, &lpOverlapped, INFINITE);
			// dwIOSize : 실제 전송된 바이트
			// pClientInfo : CompletionKey
			// lpOverlapped : Overlapped IO 객체
			// INFINITE : 대기할 시간

			if (bSuccess == TRUE && dwIoSize == 0 && lpOverlapped == NULL)
			{
				mIsWorkerRun = false;
				continue;
			}
			if (lpOverlapped == NULL)
			{
				continue;
			}

			// Client가 접속을 끊었을 때
			if (bSuccess == FALSE || (dwIoSize == 0 && bSuccess == TRUE))
			{
				printf("Socket(%d) 접속 끊김\n", (int)pClientInfo->m_socketClient);
				CloseSocket(pClientInfo);
				continue;
			}

			stOverlappedEx *pOverlappedEx = (stOverlappedEx*)lpOverlapped;

			// Overlapped I/O Recv 작업 결과 뒤 처리
			if (IOOperation::RECV == pOverlappedEx->m_eOperation)
			{
				pOverlappedEx->m_szBuf[dwIoSize] = NULL;
				printf("[수신] bytes : %d, msg : %s\n", dwIoSize, pOverlappedEx->m_szBuf);

				// Client 메시지를 에코한다.
				SendMsg(pClientInfo, pOverlappedEx->m_szBuf, dwIoSize);
				BindRecv(pClientInfo);
			}

			// Overlapped I/O Send 작업 결과 뒤 처리
			else if (IOOperation::SEND == pOverlappedEx->m_eOperation)
			{
				printf("[송신] bytes : %d, msg : %s\n", dwIoSize, pOverlappedEx->m_szBuf);
			}
			// 예외 처리
			else
			{
				printf("socket(%d) 예외 상황\n", (int)pClientInfo->m_socketClient);
			}
		}
	}

	// 사용자의 접속을 받는 쓰레드
	void AccepterThread()
	{
		SOCKADDR_IN		stClientAddr;
		int nAddrLen = sizeof(SOCKADDR_IN);

		while (mIsAccepterRun)
		{
			// 접속을 받을 구조체의 인덱스를 얻어온다.
			stClientInfo *pClientInfo = GetEmptyClientInfo();
			if (pClientInfo == NULL)
			{
				printf("Error : Client Full\n");
				return;
			}

			// Client 접속 요청이 들어올 때 까지를 기다린다.
			pClientInfo->m_socketClient = accept(mListenSock, (SOCKADDR*)&stClientAddr, &nAddrLen);
			if (INVALID_SOCKET == pClientInfo->m_socketClient)
			{
				continue;
			}

			// I/O Completion 객체와 소켓을 연결시킨다.
			bool bRet = BindIOCompletionPort(pClientInfo);
			if (bRet == false)
			{
				return;
			}

			// Recv Overlapped I/O 작업을 요청해 놓는다.
			bRet = BindRecv(pClientInfo);
			if (bRet == false)
			{
				return ;
			}

			char clientIP[32] = { 0, };
			inet_ntop(AF_INET, &(stClientAddr.sin_addr), clientIP, 32 - 1);
			printf("Client 접속 : IP(%s) SOCKET(%d)\n", clientIP, (int)pClientInfo->m_socketClient);

			// Client 갯수 증가
			++mClientCnt;

		}
	}

	// 소켓의 연결을 종료시킨다.




	// 클라이언트 정보를 저장할 구조체 
	std::vector<stClientInfo> mClientInfos;

	// 클라이언트 접속을 받기 위한 listen Socket
	SOCKET mListenSocket = INVALID_SOCKET;

	// 접속되어있는 Client 개수
	int mClientCnt = 0;

	// IO Worker Thread
	std::vector<std::thread> mIOWorkerThreads;

	// Accept Thread
	std::thread mAccepterThread;

	// CompletionPort 객체 핸들
	HANDLE mIOCPHandle = INVALID_HANDLE_VALUE;

	// 작업 쓰레드 동작 플래그
	bool mIsWorkerRun = true;

	// 접속 쓰레드 동작 플래그
	bool mIsAccepterRun = true;

	// 소켓 버퍼
	char mSocketBuf[1024] = { 0, };
};