#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>

#define BUF_SIZE 30
#pragma warning (disable:4996)
#pragma comment (lib,"ws2_32.lib")
void Handle_error(const char* message);

int main(int argc, char * argv[])
{
	WSADATA wsaData;
	SOCKET serv_socket;
	FILE *fp;

	char buf[BUF_SIZE];
	int readCnt;
	SOCKADDR_IN serv_addr;

	if (argc != 3)
	{
		printf("Usage: %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		Handle_error("WSAStartup() Error!");
	}

	fp = fopen("receive.dat", "wb");
	serv_socket = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	connect(serv_socket, (SOCKADDR*)&serv_addr, sizeof(serv_addr));

	while ((readCnt = recv(serv_socket, buf, BUF_SIZE, 0)) != 0)
	{
		fwrite((void *)buf, 1, readCnt, fp);
	}

	puts("Received file data");
	send(serv_socket, "Thank You", 10, 0);
	fclose(fp);
	closesocket(serv_socket);
	WSACleanup();
	return 0;
}

void Handle_error(const char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
