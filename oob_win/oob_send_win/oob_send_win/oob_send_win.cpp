#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define BUF_SIZE 30

#pragma warning (disable:4996)
#pragma comment (lib,"ws2_32.lib")

void Handle_Error(const char * message);

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET serv_socket;
	SOCKADDR_IN send_addr;

	if (argc != 3)
	{
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		Handle_Error("WSAStartup() Errror");
	}

	serv_socket = socket(PF_INET, SOCK_STREAM, 0);
	memset(&send_addr, 0, sizeof(send_addr));
	send_addr.sin_family = AF_INET;
	send_addr.sin_addr.s_addr = inet_addr(argv[1]);
	send_addr.sin_port = htons(atoi(argv[2]));

	if (connect(serv_socket, (SOCKADDR*)&send_addr, sizeof(send_addr)) == SOCKET_ERROR)
	{
		Handle_Error("connect() error!");
	}

	send(serv_socket, "123", 3, 0);
	send(serv_socket, "4", 1, MSG_OOB); // MSG_OOB : Out-of-band data 전송
	send(serv_socket, "567", 3, 0);
	send(serv_socket, "890", 3, MSG_OOB);

	closesocket(serv_socket);
	WSACleanup();
	return 0;
}

void Handle_Error(const char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}