/* $Id: nfsprof.h,v 1.2 2001/07/17 15:57:30 stef Exp $ */

/* 
   This header file converts all ADIOI_NFS_ names into PDIOI_NFS_ names, for use
   in providing a profiling interface 
 */

#if defined(NFS_BUILD_PROFILING)

#define ADIOI_NFS_Close PADIOI_NFS_Close
#define ADIOI_NFS_ReadDone PADIOI_NFS_ReadDone
#define ADIOI_NFS_WriteDone PADIOI_NFS_WriteDone
#define ADIOI_NFS_Fcntl PADIOI_NFS_Fcntl
#define ADIOI_NFS_Flush PADIOI_NFS_Flush
#define ADIOI_NFS_SetInfo PADIOI_NFS_SetInfo
#define ADIOI_NFS_IreadContig PADIOI_NFS_IreadContig
#define ADIOI_NFS_IreadStrided PADIOI_NFS_IreadStrided
#define ADIOI_NFS_IwriteContig PADIOI_NFS_IwriteContig
#define ADIOI_NFS_IwriteStrided PADIOI_NFS_IwriteStrided
#define ADIOI_NFS_Open PADIOI_NFS_Open
#define ADIOI_NFS_ReadStridedColl PADIOI_NFS_ReadStridedColl
#define ADIOI_NFS_ReadContig PADIOI_NFS_ReadContig
#define ADIOI_NFS_ReadStrided PADIOI_NFS_ReadStrided
#define ADIOI_NFS_Resize PADIOI_NFS_Resize
#define ADIOI_NFS_SeekIndividual PADIOI_NFS_SeekIndividual
#define ADIOI_NFS_ReadComplete PADIOI_NFS_ReadComplete
#define ADIOI_NFS_WriteComplete PADIOI_NFS_WriteComplete
#define ADIOI_NFS_WriteStridedColl PADIOI_NFS_WriteStridedColl
#define ADIOI_NFS_WriteContig PADIOI_NFS_WriteContig
#define ADIOI_NFS_WriteStrided PADIOI_NFS_WriteStrided
#define ADIOI_NFS_Get_shared_fp PADIOI_NFS_Get_shared_fp
#define ADIOI_NFS_Set_shared_fp PADIOI_NFS_Set_shared_fp

#endif

