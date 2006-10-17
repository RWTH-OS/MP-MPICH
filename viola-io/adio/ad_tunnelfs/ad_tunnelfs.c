/**************************************************************************
* TunnelFS                                                                * 
***************************************************************************
*                                                                         * 
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              * 
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
*                                                                         * 
* See COPYRIGHT notice in base directory for details                      * 
**************************************************************************/
/**************************************************************************
* File:         ad_tunnelfs.c                                             * 
* Description:                                                            *
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#include "ad_tunnelfs.h"

/* adioi.h has the ADIOI_Fns_struct define */
#include "adioi.h"

/**
 * Global variable holding the main server for client requests.
 */
int TUNNELFS_GLOBAL_MASTER = -1;

/**
 * ADIO Structure for pointers to the corresponding functions
 */
struct ADIOI_Fns_struct ADIO_TUNNELFS_operations = {
    ADIOI_TUNNELFS_Open,        /* Open */
    ADIOI_TUNNELFS_ReadContig,  /* ReadContig */
    ADIOI_TUNNELFS_WriteContig, /* WriteContig */
    ADIOI_TUNNELFS_ReadStridedColl,     /* ReadStridedColl */
    ADIOI_TUNNELFS_WriteStridedColl,    /* WriteStridedColl */
    ADIOI_GEN_SeekIndividual,   /* SeekIndividual */
    ADIOI_TUNNELFS_Fcntl,       /* Fcntl */
    ADIOI_TUNNELFS_SetInfo,     /* SetInfo */
    ADIOI_TUNNELFS_ReadStrided, /* ReadStrided */
    ADIOI_TUNNELFS_WriteStrided,        /* WriteStrided */
    ADIOI_TUNNELFS_Close,       /* Close */
    ADIOI_TUNNELFS_IreadContig, /* IreadContig */
    ADIOI_TUNNELFS_IwriteContig,        /* IwriteContig */
    ADIOI_TUNNELFS_ReadDone,    /* ReadDone */
    ADIOI_TUNNELFS_WriteDone,   /* WriteDone */
    ADIOI_TUNNELFS_ReadComplete,        /* ReadComplete */
    ADIOI_TUNNELFS_WriteComplete,       /* WriteComplete */
    ADIOI_TUNNELFS_IreadStrided,        /* IreadStrided */
    ADIOI_TUNNELFS_IwriteStrided,       /* IwriteStrided */
    ADIOI_TUNNELFS_Flush,       /* Flush */
    ADIOI_TUNNELFS_Resize,      /* Resize */
    ADIOI_TUNNELFS_Delete       /* Delete */
};
