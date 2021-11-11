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

	}
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