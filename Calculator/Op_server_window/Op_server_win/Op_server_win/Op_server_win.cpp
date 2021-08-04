#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>

#pragma comment (lib,"ws2_32.lib")
#define BUF_SIZE 1024
#define OPSZ 4
void Handle_Error(const char* message);

int calculate(int opnum, int opnds[], char oprator);

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET ServSock, ClntSock;
	char opinfo[BUF_SIZE];
	int result, opndCnt, i;
	int recvCnt, recvLen;
	SOCKADDR_IN servAdr, clntAdr;
	int clntAdrSize;
	if (argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		Handle_Error("WSAStartup() error!");
	}

	ServSock = socket(PF_INET, SOCK_STREAM, 0);
	if (ServSock == INVALID_SOCKET)
		Handle_Error("socket() error");

	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(atoi(argv[1]));

	if (bind(ServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
	{
		Handle_Error("bind() error");
	}
	if (listen(ServSock, 5) == SOCKET_ERROR)
	{
		Handle_Error("listen() error");
	}
	clntAdrSize = sizeof(clntAdr);

	for (i = 0; i < 5; i++)
	{
		opndCnt = 0;
		ClntSock = accept(ServSock, (SOCKADDR*)&clntAdr, &clntAdrSize);
		recv(ClntSock, (char *)&opndCnt, 1, 0);

		recvLen = 0;
		while ((opndCnt*OPSZ + 1) > recvLen)
		{
			recvCnt = recv(ClntSock, &opinfo[recvLen], BUF_SIZE - 1, 0);
			recvLen += recvCnt;
		}
		result = calculate(opndCnt, (int *)opinfo, opinfo[recvLen - 1]);
		send(ClntSock, (char *)&result, sizeof(result), 0);
		closesocket(ClntSock);
	}
	closesocket(ServSock);
	WSACleanup();
	return 0;
}

int calculate(int opnum, int opnds[], char op)
{
	int result = opnds[0], i;

	switch (op)
	{
	case '+':
		for (i = 1; i < opnum; i++)	result += opnds[i];
		break;
	case '-':
		for (i = 1; i < opnum; i++) result -= opnds[i];
		break;
	case '*':
		for (i = 1; i < opnum; i++) result *= opnds[i];
		break;
	}
	return result;
}

void Handle_Error(const char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}