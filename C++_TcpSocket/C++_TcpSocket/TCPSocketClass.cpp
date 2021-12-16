#include "TCPSocket.h"

int TCPSocket::Connect(const SocketAddress& inAddress)
{
	int err = connect(mSocket, &inAddress.mSocketAddr, inAddress.GetSize());
	if (err >= 0)
	{
		return NO_ERROR;
	}
	SocketUtil::ReportError("TCPSocket::Connect");
	return -SocketUtil::GetLastError();
}

int TCPSocket::Listen(int inBackLog)
{
	int err = listen(mSocket, inBackLog);
	if (err < 0)
	{
		SocketUtil::ReportError("TCPSocket::Listen");
		return -SocketUtil::GetLastError();
	}
	return NO_ERROR;
}

TCPSocketPtr TCPSocket::Accept(SocketAddress& inFromAddress)
{
	socketlen_t length = inFromAddresss.GetSize();
	SOCKET newSocket = accept(mSocket, &inFromAddress.mSockAddr, &length);

	if (newSocket != INVALID_SOCKET)
	{
		return TCPSocketPtr(new TCPSocket(newSocket));
	}
	else
	{
		SocketUtil::ReportError("TCPSocket::Accept");
		return nullptr;
	}
}


int32_t TCPSocket::Send(const void * inData, size_t inLen)
{
	int bytesSentCount = send(mSocket, static_cast<const char*> (inData), inLen, 0);
	if (bytesSentCount < 0)
	{
		SocketUtil::ReportError("TCPSocket::Send");
		return -SocketUtil::GetLastError();
	}
	return bytesSentCount;
}