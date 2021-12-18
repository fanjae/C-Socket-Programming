#include "RoboCatShared.h"

string SocketAddress::ToString() const
{
#if _WIN32
	const sockaddr_in* s = GetAsSockAddrIn();
	char destinationBuffer[128];
	InetNtop(s->sin_family, const_char<in_addr *>(&s->sin_addr), destinationBuffer, sizeof(destinationBuffer));
	return StringUtils::Sprintf("%s:%d", destinationBuffer, ntohs(s->sin_port));
#else
	return string("not implemented on mac for now");
#endif
}