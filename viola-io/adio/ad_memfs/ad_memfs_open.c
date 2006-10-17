/**************************************************************************
* MEMFS                                                                   * 
***************************************************************************
*                                                                         * 
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              * 
*                                                                         * 
* See COPYRIGHT notice in base directory for details                      * 
**************************************************************************/
/**************************************************************************
* File:         ad_memfs_open.c                                           * 
* Description:                                                            *
* Author(s):    Jan Seidel <Jan.Seidel@fh-bonn-rhein-sieg.de>             * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/ 


#include "ad_memfs.h"
#include "adioi.h"

void ADIOI_MEMFS_Open(ADIO_File fd, int *error_code)
{
    UNLOCK_MPI();
 
    /* MEMFS_Init(1); */

#ifdef MEMFS_TIME
    double t1, t2;
    t1 = gettime();
#endif

    /* communicate with MEMFS main thread */
    thread_comm(MEMFS_OPEN, fd, NULL, error_code);

#ifdef MEMFS_TIME
    t2 = gettime();
    settime(OPEN_TIME, t2-t1);
    settime(TOTAL_TIME, t2-t1);
#endif

    LOCK_MPI();

}

