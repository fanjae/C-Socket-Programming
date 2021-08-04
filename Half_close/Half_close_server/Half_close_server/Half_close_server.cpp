#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>

#define BUF_SIZE 30
#pragma warning (disable:4996) // fopen 에러 무시
#pragma comment (lib,"ws2_32.lib")
void Handle_Error(const char *message);

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET serversocket, clientsocket;
	FILE *fp;

	char buf[BUF_SIZE];
	int readCnt;

	SOCKADDR_IN server_addr, client_addr;
	int client_addr_size;

	if (argc != 2)
	{
		printf("Usage : %s <port> \n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		Handle_Error("WSAStartup() error!");
	}

	fp = fopen("Half_close_server.cpp", "rb");
	serversocket = socket(PF_INET, SOCK_STREAM, 0);

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(argv[1]));

	bind(serversocket, (SOCKADDR*)&server_addr, sizeof(server_addr));
	listen(serversocket, 5);

	client_addr_size = sizeof(client_addr);
	clientsocket = accept(serversocket, (SOCKADDR*)&client_addr, &client_addr_size);

	while (1)
	{
		readCnt = fread((void *)buf, 1, BUF_SIZE, fp);
		if (readCnt < BUF_SIZE)
		{
			send(clientsocket, (char *)&buf, readCnt, 0);
			break;
		}
		send(clientsocket, (char *)&buf, BUF_SIZE, 0);
	}

	shutdown(clientsocket, SD_SEND); // 출력 스트림 종료
	recv(clientsocket, (char *)buf, BUF_SIZE, 0);
	printf("Message from client: %s \n", buf);

	fclose(fp);
	closesocket(clientsocket);
	closesocket(serversocket);
	WSACleanup();
	return 0;
}

void Handle_Error(const char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

