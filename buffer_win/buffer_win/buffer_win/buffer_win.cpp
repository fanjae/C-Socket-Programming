#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment (lib,"ws2_32.lib")
#pragma warning (disable:4996)

void Handle_Error(const char* message);
void ShowSocketBufSize(SOCKET sock);

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET serv_sock;
	int sendbuffer, recvbuffer, state;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		Handle_Error("WSAStartup() Error");
	}

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	ShowSocketBufSize(serv_sock);

	sendbuffer = 1024 * 3;
	recvbuffer = 1024 * 3;
	state = setsockopt(serv_sock, SOL_SOCKET, SO_SNDBUF, (char *)&sendbuffer, sizeof(sendbuffer)); // 입력 버퍼 크기 변경
	if (state == SOCKET_ERROR)
	{
		Handle_Error("setsockopt error!");
	}

	state = setsockopt(serv_sock, SOL_SOCKET, SO_RCVBUF, (char *)&recvbuffer, sizeof(recvbuffer)); // 출력 버퍼 크기 변경
	if (state == SOCKET_ERROR)
	{
		Handle_Error("setsockopt error!");
	}

	ShowSocketBufSize(serv_sock);
	closesocket(serv_sock);
	WSACleanup();
	return 0;
}

void ShowSocketBufSize(SOCKET sock)
{
	int send_buffer, recv_buffer, state, len;

	len = sizeof(send_buffer);
	state = getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&send_buffer, &len);
	if (state == SOCKET_ERROR)
	{
		Handle_Error("getsockopt() error");
	}

	len = sizeof(recv_buffer);
	state = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&recv_buffer, &len);
	if (state == SOCKET_ERROR)
	{
		Handle_Error("getsockopt() error");
	}
	printf("Input buffer size : %d\n", recv_buffer);
	printf("Output buffer size : %d\n", send_buffer);
}

void Handle_Error(const char * message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}