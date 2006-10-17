#ifndef __SOCKET_EXCEPTION__
#define __SOCKET_EXCEPTION__
#include <stdio.h>

#ifdef _MSC_VER				
#pragma warning( disable :  4786 )
#endif

class socketException
{ public:
socketException(const char* T=0) {
    error = WSAGetLastError(); //wsock2func
    if(T) {
	txt=new char[strlen(T)+30];
	sprintf(txt,"SocketExeption: %d\n%s",error,T);
    }
}

socketException(const socketException &other) {
	   txt=new char[strlen(other.txt)+1];
	   strcpy(txt,other.txt);
	   error=other.error;
}

DWORD GetError() {return error;}
~socketException(void) { if(txt) delete txt;txt=0;}
operator const char*(void) {return txt;}
private:
    char* txt;
    DWORD error;
};

#endif
