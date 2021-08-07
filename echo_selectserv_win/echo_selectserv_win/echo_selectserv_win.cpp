#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>

#pragma comment (lib,"ws2_32.lib")
#define BUF_SIZE 1024
void Handle_Error(const char * message);

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET serv_sock, clnt_sock;
	SOCKADDR_IN serv_adr, clnt_adr;
	TIMEVAL timeout; // 
	fd_set reads, cpyReads; // 윈도우의 fd_set은 Linux의 fd_set과 같은 비트의 배열 형태가 아님에 유의.

	int adr_size;
	int str_length, fdNum, i;

	char buf[BUF_SIZE];

	if (argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		Handle_Error("WSAStartup() error!");
	}

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if (bind(serv_sock, (SOCKADDR*)&serv_adr, sizeof(serv_adr)) == SOCKET_ERROR)
	{
		Handle_Error("bind() error");
	}
	if (listen(serv_sock, 5) == SOCKET_ERROR)
	{
		Handle_Error("listen() error");
	}

	FD_ZERO(&reads);
	FD_SET(serv_sock, &reads);

	while (1)
	{
		cpyReads = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;

		if ((fdNum = select(0, &cpyReads, 0, 0, &timeout)) == SOCKET_ERROR)
		{
			break;
		}
		if (fdNum == 0)
		{
			continue;
		}

		for (i = 0; i < reads.fd_count; i++)
		{
			if (FD_ISSET(reads.fd_array[i], &cpyReads))
			{
				if (reads.fd_array[i] == serv_sock) // connection request!
				{
					adr_size = sizeof(clnt_adr);
					clnt_sock = accept(serv_sock, (SOCKADDR*)&clnt_adr, &adr_size);
					FD_SET(clnt_sock, &reads);
					printf("connection client : %d \n", clnt_sock);
				}
				else // read message
				{
					str_length = recv(reads.fd_array[i], buf, BUF_SIZE - 1, 0);
					if (str_length == 0)
					{
						FD_CLR(reads.fd_array[i], &reads);
						closesocket(cpyReads.fd_array[i]);
						printf("closed client: %d\n", cpyReads.fd_array[i]);
					}
					else
					{
						send(reads.fd_array[i], buf, str_length, 0);
					}
				}
			}
		}
	}

	closesocket(serv_sock);
	WSACleanup();

	return 0;
}
					
void Handle_Error(const char * message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}


