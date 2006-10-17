/**************************************************************************
* MEMFS                                                                   * 
***************************************************************************
*                                                                         * 
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              * 
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
*                                                                         * 
* See COPYRIGHT notice in base directory for details                      * 
**************************************************************************/
/**************************************************************************
* File:         ad_memfs.c                                                * 
* Description:                                                            *
* Author(s):    Jan Seidel <Jan.Seidel@fh-bonn-rhein-sieg.de>             *
		Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/

#include "ad_memfs.h"

/* adioi.h has the ADIOI_Fns_struct define */
#include "adioi.h"

struct ADIOI_Fns_struct ADIO_MEMFS_operations = {
    ADIOI_MEMFS_Open, /* Open */
    ADIOI_MEMFS_ReadContig, /* ReadContig */
    ADIOI_MEMFS_WriteContig, /* WriteContig */
    ADIOI_MEMFS_ReadStridedColl, /* ReadStridedColl */
    ADIOI_MEMFS_WriteStridedColl, /* WriteStridedColl */
    ADIOI_MEMFS_SeekIndividual, /* SeekIndividual */
    ADIOI_MEMFS_Fcntl, /* Fcntl */
    ADIOI_MEMFS_SetInfo, /* SetInfo */
    ADIOI_MEMFS_ReadStrided, /* ReadStrided */
    ADIOI_MEMFS_WriteStrided, /* WriteStrided */
    ADIOI_MEMFS_Close, /* Close */
    ADIOI_MEMFS_IreadContig, /* IreadContig */
    ADIOI_MEMFS_IwriteContig, /* IwriteContig */
    ADIOI_MEMFS_ReadDone, /* ReadDone */
    ADIOI_MEMFS_WriteDone, /* WriteDone */
    ADIOI_MEMFS_ReadComplete, /* ReadComplete */
    ADIOI_MEMFS_WriteComplete, /* WriteComplete */
    ADIOI_MEMFS_IreadStrided, /* IreadStrided */
    ADIOI_MEMFS_IwriteStrided, /* IwriteStrided */
    ADIOI_MEMFS_Flush, /* Flush */
    ADIOI_MEMFS_Resize, /* Resize */
    ADIOI_MEMFS_Delete, /* Delete */
};
