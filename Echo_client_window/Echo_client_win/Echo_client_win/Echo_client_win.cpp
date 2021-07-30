#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define BUF_SIZE 1024
#pragma comment (lib,"ws2_32.lib")
void Handle_Error(const char* message);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET Sev_Socket;
	SOCKADDR_IN Server_Addr;
	char message[BUF_SIZE];
	int string_length;

	if (argc != 3)
	{
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		Handle_Error("WSAStartup() error!");
	}

	Sev_Socket = socket(PF_INET, SOCK_STREAM, 0);
	if (Sev_Socket == INVALID_SOCKET)
	{
		Handle_Error("socket() error!");
	}

	memset(&Server_Addr, 0, sizeof(Server_Addr));
	Server_Addr.sin_family = AF_INET;
	Server_Addr.sin_addr.s_addr = inet_addr(argv[1]);
	Server_Addr.sin_port = htons(atoi(argv[2]));

	if (connect(Sev_Socket, (SOCKADDR*)&Server_Addr, sizeof(Server_Addr)) == SOCKET_ERROR)
	{
		Handle_Error("connect() error!");
	}
	else
	{
		puts("connected...........");
	}

	while (1)
	{
		fputs("Input message(Q to quit): ", stdout);
		fgets(message, BUF_SIZE, stdin);

		if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
		{
			break;
		}

		send(Sev_Socket, message, strlen(message), 0);
		string_length = recv(Sev_Socket, message, BUF_SIZE - 1, 0);

		message[string_length] = 0;
		printf("Message from server : %s", message);
	}
	closesocket(Sev_Socket);
	WSACleanup();
	return 0;
}

void Handle_Error(const char * message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}



