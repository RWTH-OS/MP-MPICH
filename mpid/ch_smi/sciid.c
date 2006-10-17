/* $Id: sciid.c,v 1.3 2001/11/12 14:28:03 joachim Exp $ */
 
/* get SCI id via SCI device */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef WIN32
/* these defines are from sci_io.h */
#define _SCIIOCPARM_MASK   0xff	/* parameters must be < 256 bytes */
#define _SCIIOC_OUT        0x40000000	/* copy out parameters */
#define _SCIIOR(x,y,t)     (_SCIIOC_OUT|((sizeof(t)&_SCIIOCPARM_MASK)<<16)|(x<<8)|y)
#define NC_GET_NODE_ID        _SCIIOR('s',0x24,u_int)
#endif

#ifdef __i386
static char DevicePath[10]="/dev/SCI/\0";
#else
static char DevicePath[10]="/dev/sci\0";
#endif
static int fd;

#define ERROR(where,code) {\
			  fprintf(stderr,"ERROR(%s):%i\n",where,code); \
			  return(code);\
                          }


int main(void)
{
    int i;
    int error;
    int NodeId;
    int DevNo;
    char DeviceName[64];
    
    DevNo = 0;
    do {
	DevNo++;	  
	sprintf( DeviceName, "%s%i", DevicePath, DevNo );
	fd = open( DeviceName, O_RDWR );
    } while ( ( DevNo, 255 ) && ( fd == -1) );
    
    if ( fd == -1 ) {
	sprintf( DeviceName, "%s", DevicePath);
	ERROR(DeviceName, -1);
    }
    
    error = ioctl( fd, NC_GET_NODE_ID, &NodeId );
    if (error == -1)
	ERROR("Could not get Node ID", -1);
    
    printf("%d\n", NodeId); 
    close( fd );

    return( NodeId );
}
