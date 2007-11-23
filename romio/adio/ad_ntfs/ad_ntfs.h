/* 
 *   $Id: ad_ntfs.h 922 2001-05-31 14:10:48Z karsten $    
 *
 */

#ifndef AD_NTFS_INCLUDE
#define AD_NTFS_INCLUDE

#include <wtypes.h>
#include <winbase.h>
#include <sys/types.h>
#include <fcntl.h>
#include "adio.h"

#ifndef HasOverlappedIoCompleted
#define HasOverlappedIoCompleted(lpOverlapped) ((lpOverlapped)->Internal != STATUS_PENDING)
#endif


int ADIOI_NTFS_aio(ADIO_File fd, int wr, ADIO_Request request,int blocking, DWORD *error_code);
char *ad_ntfs_error(int value );

OVERLAPPED *GetOverlappedStruct(void);
void FreeOverlappedStruct(OVERLAPPED *);

#define MAX_BUFFERED_OVERS 100



#endif
