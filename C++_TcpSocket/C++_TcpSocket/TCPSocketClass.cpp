#include "TCPSocket.h"

int TCPSocket::Connect(const SocketAddress& inAddress)
{
	int err = connect(mSocket, &inAddress.mSocketAddr, inAddress.GetSize());
	if (err >= 0)
	{
		return 0;
	}
	SocketUtil::ReportError("TCPSocket::Connect");
	return -SocketUtil::GetLastError();
}