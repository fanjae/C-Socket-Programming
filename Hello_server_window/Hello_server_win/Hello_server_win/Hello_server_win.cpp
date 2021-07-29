#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#pragma comment (lib, "ws2_32.lib") // ws2_32.lib dependencies list 추가.
void Handle_Error(const char* ErrorMessage);

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET ServerSocket, ClientSocket;
	SOCKADDR_IN ServerAddr, ClientAddr;

	int ClientAddr_size;
	char message[] = "Hello Server!";
	if (argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) // Socket 라이브러리 초기화
	{
		Handle_Error("WSAStartup() Error!");
	}

	ServerSocket = socket(PF_INET, SOCK_STREAM, 0); // 소켓 생성
	if (ServerSocket == INVALID_SOCKET)
	{
		Handle_Error("socket() Error");
	}

	memset(&ServerAddr, 0, sizeof(ServerAddr));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	ServerAddr.sin_port = htons(atoi(argv[1]));

	if (bind(ServerSocket, (SOCKADDR *)&ServerAddr, sizeof(ServerAddr)) == SOCKET_ERROR) // 소켓에 주소정보 할당.
	{
		Handle_Error("bind() Error");
	}

	if (listen(ServerSocket, 5) == SOCKET_ERROR) // 소켓을 연결 가능한 상태로 만듦
	{
		Handle_Error("listen() Error");
	}

	ClientAddr_size = sizeof(ClientAddr);
	ClientSocket = accept(ServerSocket, (SOCKADDR *)&ClientAddr, &ClientAddr_size); // 연결 요청에 대한 수락
	if (ClientSocket == INVALID_SOCKET)
	{
		Handle_Error("Accept() Error");
	}

	send(ClientSocket, message, sizeof(message), 0);
	closesocket(ClientSocket);
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