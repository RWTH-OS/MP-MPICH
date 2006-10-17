
#if !(defined ALREADY_INCLUDED_WINSOCK2_H)
#define ALREADY_INCLUDED_WINSOCK2_H


#include "winsock.h"

#define FD_READ 0x01

#define IN

#define WSAEVENT HANDLE

int
WSAEventSelect (
   	IN SOCKET s,
		IN WSAEVENT hEventObject,
		IN long lNetworkEvent);



#endif
