#include <wtypes.h>
#include <winbase.h>

char *ad_ntfs_error( int value )
{
	static char err_msg[1024];
	void *lpMsgBuf;

	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,value,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR) &lpMsgBuf,1,NULL);
	
	if(lpMsgBuf) 
		strcat(err_msg,(char*)lpMsgBuf);

	return err_msg;
}