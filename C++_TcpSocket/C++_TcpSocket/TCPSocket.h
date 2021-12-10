#include "SharedHeader.h"

class TCPSocket
{
public:
	~TCPSocket();
	int Connect(const SocketAdress& inAddress);
	int Bind(const SocketAddress& inToAddress);
	int Listen(int intBackLog = 32);
	shared_ptr<TCPSocket> Accept(SocketAddress& inFromAddress);
	int Send(const void* inData, int inLen);
	int Receive(void * inBuffer, int inLen);
private:
	friend class SocketUtil;
	TCPSocket(SOCKET inSocket) : mSocket(inSocket) { }
	SOCKET mSocket;
};
typedef shared_ptr <TCPSocket> TCPSocketPtr;
