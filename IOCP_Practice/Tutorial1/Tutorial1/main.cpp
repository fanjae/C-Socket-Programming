#include "IOCompletionPort.h"

const UINT16 SERVER_PORT = 11021;
const UINT16 MAX_CLIENT = 100;

int main()
{
	IOCompletionPort = ioCompletionPort;

	// 소켓 초기화
	ioCompletionPort.InitSocket();
