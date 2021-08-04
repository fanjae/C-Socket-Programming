#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 1024
#pragma comment (lib,"ws2_32.lib")
void Handle_Error(const char* message);

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET Server_socket, Client_socket;

	char message[BUF_SIZE];
	int string_length = 0;
	int i;

	SOCKADDR_IN Server_Addr, Client_Addr;
	int Client_Addr_size;

	if (argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		Handle_Error("WSAStartup() Error!");
	}

	Server_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (Server_socket == INVALID_SOCKET)
	{
		Handle_Error("socket() Errror");
	}

	memset(&Server_Addr, 0, sizeof(Server_Addr));
	Server_Addr.sin_family = AF_INET;
	Server_Addr.sin_addr.s_addr = htonl(INADDR_ANY);
	Server_Addr.sin_port = htons(atoi(argv[1]));

	if (bind(Server_socket, (SOCKADDR*)&Server_Addr, sizeof(Server_Addr)) == SOCKET_ERROR)
	{
		Handle_Error("bind() error");
	}

	if (listen(Server_socket, 5) == SOCKET_ERROR)
	{
		Handle_Error("listen() Error");
	}

	Client_Addr_size = sizeof(Client_Addr);

	for (i = 0; i < 5; i++)
	{
		Client_socket = accept(Server_socket, (SOCKADDR*)&Client_Addr, &Client_Addr_size);
		if (Client_socket == -1)
		{
			Handle_Error("Aceept() Error");
		}
		else
		{
			printf("Connected client %d \n", i + 1);
		}

		while ((string_length = recv(Client_socket, message, BUF_SIZE, 0)) != 0)
		{
			send(Client_socket, message, string_length, 0);
		}

		closesocket(Client_socket);
	}
	closesocket(Server_socket);
	WSACleanup();
	return 0;
}
void Handle_Error(const char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

