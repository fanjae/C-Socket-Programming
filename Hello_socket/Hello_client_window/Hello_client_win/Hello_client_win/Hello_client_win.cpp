#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment (lib,"ws2_32.lib")
void Handle_Error(const char* message);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET ServerSocket;
	SOCKADDR_IN ServAddr;

	char message[30];
	int string_length;

	if (argc != 3)
	{
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		Handle_Error("WSAStartup() Error!");
	}

	ServerSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (ServerSocket == INVALID_SOCKET)
	{
		Handle_Error("socket() Error");
	}

	memset(&ServAddr, 0, sizeof(ServAddr));
	ServAddr.sin_family = AF_INET;
	ServAddr.sin_addr.s_addr = inet_addr(argv[1]);
	ServAddr.sin_port = htons(atoi(argv[2]));

	if (connect(ServerSocket, (SOCKADDR*)&ServAddr, sizeof(ServAddr)) == SOCKET_ERROR) // 연결요청
	{
		Handle_Error("connect() Error");
	}

	string_length = recv(ServerSocket, message, sizeof(message) - 1, 0); // 윈도우는 리눅스와 달리 소켓 입출력 함수가 구분되어 있음에 유의
	if (string_length == -1)
	{
		Handle_Error("read() Error!");
	}
	printf("Message From server : %s\n", message);

	closesocket(ServerSocket);
	WSACleanup();
	return 0;
}

void Handle_Error(const char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}