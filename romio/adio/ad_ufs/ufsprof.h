/* $Id: ufsprof.h 1320 2001-11-16 17:12:01Z joachim $ */

/* 
   This header file converts all ADIOI_UFS_ names into PDIOI_UFS_ names, for use
   in providing a profiling interface 
 */

#if defined(UFS_BUILD_PROFILING)

#define ADIOI_UFS_Close PADIOI_UFS_Close
#define ADIOI_UFS_ReadDone PADIOI_UFS_ReadDone
#define ADIOI_UFS_WriteDone PADIOI_UFS_WriteDone
#define ADIOI_UFS_Fcntl PADIOI_UFS_Fcntl
#define ADIOI_UFS_Flush PADIOI_UFS_Flush
#define ADIOI_UFS_SetInfo PADIOI_UFS_SetInfo
#define ADIOI_UFS_IreadContig PADIOI_UFS_IreadContig
#define ADIOI_UFS_IreadStrided PADIOI_UFS_IreadStrided
#define ADIOI_UFS_IwriteContig PADIOI_UFS_IwriteContig
#define ADIOI_UFS_IwriteStrided PADIOI_UFS_IwriteStrided
#define ADIOI_UFS_Open PADIOI_UFS_Open
#define ADIOI_UFS_ReadStridedColl PADIOI_UFS_ReadStridedColl
#define ADIOI_UFS_ReadContig PADIOI_UFS_ReadContig
#define ADIOI_UFS_ReadStrided PADIOI_UFS_ReadStrided
#define ADIOI_UFS_Resize PADIOI_UFS_Resize
#define ADIOI_UFS_SeekIndividual PADIOI_UFS_SeekIndividual
#define ADIOI_UFS_ReadComplete PADIOI_UFS_ReadComplete
#define ADIOI_UFS_WriteComplete PADIOI_UFS_WriteComplete
#define ADIOI_UFS_WriteStridedColl PADIOI_UFS_WriteStridedColl
#define ADIOI_UFS_WriteContig PADIOI_UFS_WriteContig
#define ADIOI_UFS_WriteStrided PADIOI_UFS_WriteStrided

#endif

