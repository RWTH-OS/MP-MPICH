/* $Id: pvfsprof.h,v 1.2 2001/07/17 15:57:31 stef Exp $ */

/* 
   This header file converts all ADIOI_PVFS_ names into PDIOI_PVFS_ names, for use
   in providing a profiling interface 
 */

#if defined(PVFS_BUILD_PROFILING)

#define ADIOI_PVFS_Close PADIOI_PVFS_Close
#define ADIOI_PVFS_ReadDone PADIOI_PVFS_ReadDone
#define ADIOI_PVFS_WriteDone PADIOI_PVFS_WriteDone
#define ADIOI_PVFS_Fcntl PADIOI_PVFS_Fcntl
#define ADIOI_PVFS_Flush PADIOI_PVFS_Flush
#define ADIOI_PVFS_SetInfo PADIOI_PVFS_SetInfo
#define ADIOI_PVFS_IreadContig PADIOI_PVFS_IreadContig
#define ADIOI_PVFS_IreadStrided PADIOI_PVFS_IreadStrided
#define ADIOI_PVFS_IwriteContig PADIOI_PVFS_IwriteContig
#define ADIOI_PVFS_IwriteStrided PADIOI_PVFS_IwriteStrided
#define ADIOI_PVFS_Open PADIOI_PVFS_Open
#define ADIOI_PVFS_ReadStridedColl PADIOI_PVFS_ReadStridedColl
#define ADIOI_PVFS_ReadContig PADIOI_PVFS_ReadContig
#define ADIOI_PVFS_ReadStrided PADIOI_PVFS_ReadStrided
#define ADIOI_PVFS_Resize PADIOI_PVFS_Resize
#define ADIOI_PVFS_SeekIndividual PADIOI_PVFS_SeekIndividual
#define ADIOI_PVFS_ReadComplete PADIOI_PVFS_ReadComplete
#define ADIOI_PVFS_WriteComplete PADIOI_PVFS_WriteComplete
#define ADIOI_PVFS_WriteStridedColl PADIOI_PVFS_WriteStridedColl
#define ADIOI_PVFS_WriteContig PADIOI_PVFS_WriteContig
#define ADIOI_PVFS_WriteStrided PADIOI_PVFS_WriteStrided

#endif

