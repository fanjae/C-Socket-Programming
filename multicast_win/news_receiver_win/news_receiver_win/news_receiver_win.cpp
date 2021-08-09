#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include <WS2tcpip.h> // for struct ip_mreq

#pragma comment (lib,"ws2_32.lib")
#pragma warning (disable:4996)
#define BUF_SIZE 30
void Handle_Error(const char *message);

int main(int argc, char * argv[])
{
	WSADATA wsaData;
	SOCKET recvsock;
	SOCKADDR_IN addr;
	struct ip_mreq join_adr;
	char buf[BUF_SIZE];
	int str_length;

	if (argc != 3)
	{
		printf("Usage : %s <GroupIP> <PORT>\n", argv[0]);
		exit(1);
	}
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		Handle_Error("WSAStartup() error!");
	}

	recvsock = socket(PF_INET, SOCK_DGRAM, 0);
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(atoi(argv[2]));
	if (bind(recvsock, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		Handle_Error("bind() error");
	}

	join_adr.imr_multiaddr.s_addr = inet_addr(argv[1]);
	join_adr.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(recvsock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&join_adr, sizeof(join_adr)) == SOCKET_ERROR)
	{
		Handle_Error("setsocket() error");
	}

	while (1)
	{
		str_length = recvfrom(recvsock, buf, BUF_SIZE - 1, 0, NULL, 0);
		if (str_length < 0)
		{
			break;
		}
		buf[str_length] = 0;
		fputs(buf, stdout);
	}
	closesocket(recvsock);
	WSACleanup();
	return 0;
}
void Handle_Error(const char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}