#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h> // IP_MULTICAST_TTL 선언

#pragma comment (lib,"ws2_32.lib")
#pragma warning (disable:4996)
#define TTL 64
#define BUF_SIZE 30

void Handle_Error(const char *message);

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET sendsock;
	SOCKADDR_IN mul_addr;
	int timeLive = TTL;
	FILE *fp;
	char buf[BUF_SIZE];

	if (argc != 3)
	{
		printf("Usage : %s <GroupIP> <PORT>\n", argv[0]);
		exit(1);
	}
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		Handle_Error("WSAStartup() error!");
	}

	sendsock = socket(PF_INET, SOCK_DGRAM, 0);
	memset(&mul_addr, 0, sizeof(mul_addr));
	mul_addr.sin_family = AF_INET;
	mul_addr.sin_addr.s_addr = inet_addr(argv[1]);
	mul_addr.sin_port = htons(atoi(argv[2]));

	setsockopt(sendsock, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&timeLive, sizeof(timeLive));

	if ((fp = fopen("news.txt", "r")) == NULL)
	{
		Handle_Error("fopen() error");
	}
	while (!feof(fp))
	{
		fgets(buf, BUF_SIZE, fp);
		sendto(sendsock, buf, strlen(buf), 0, (SOCKADDR*)&mul_addr, sizeof(mul_addr));
		Sleep(2000);
	}
	closesocket(sendsock);
	WSACleanup();
	return 0;
}

void Handle_Error(const char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}