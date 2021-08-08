#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define BUF_SIZE 30
#pragma comment (lib,"ws2_32.lib")
void Handle_Error(const char * message);
int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET acpt_sock, recv_sock;

	SOCKADDR_IN recv_adr;
	SOCKADDR_IN send_adr;
	int send_adr_size, str_length;
	char buf[BUF_SIZE];
	int result;

	fd_set read, except, readCopy, exceptCopy;
	struct timeval timeout;

	if (argc != 3)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		Handle_Error("WSAStartup() error!");
	}

	acpt_sock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&recv_adr, 0, sizeof(recv_adr));
	recv_adr.sin_family = AF_INET;
	recv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	recv_adr.sin_port = htons(atoi(argv[1]));

	if (bind(acpt_sock, (SOCKADDR*)&recv_adr, sizeof(recv_adr)) == SOCKET_ERROR)
	{
		Handle_Error("bind() error");
	}
	if (listen(acpt_sock, 5) == SOCKET_ERROR)
	{
		Handle_Error("listen() error");
	}

	send_adr_size = sizeof(send_adr);
	recv_sock = accept(acpt_sock, (SOCKADDR*)&send_adr, &send_adr_size);

	FD_ZERO(&read);
	FD_ZERO(&except);
	FD_SET(recv_sock, &read);
	FD_SET(recv_sock, &except);

	while (1)
	{
		readCopy = read;
		exceptCopy = except;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		result = select(0, &readCopy, 0, &exceptCopy, &timeout);

		if (result > 0)
		{
			if (FD_ISSET(recv_sock, &exceptCopy))
			{
				str_length = recv(recv_sock, buf, BUF_SIZE - 1, MSG_OOB);
				buf[str_length] = 0;
				printf("Urgent mesage : %s\n", buf);
			}

			if (FD_ISSET(recv_sock, &readCopy))
			{
				str_length = recv(recv_sock, buf, BUF_SIZE - 1, 0);
				if (str_length == 0)
				{
					closesocket(recv_sock);
					break;
				}
				else
				{
					buf[str_length] = 0;
					puts(buf);
				}
			}
		}
	}

	closesocket(acpt_sock);
	WSACleanup();
	return 0;
}
void Handle_Error(const char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

