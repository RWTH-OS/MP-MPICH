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

void ADIOI_MEMFS_Close(ADIO_File fd, int *error_code)
{
    UNLOCK_MPI();

#ifdef MEMFS_TIME
    double t1, t2;
    t1 = gettime();
#endif

    /* communicate with MEMFS main thread */
    thread_comm(MEMFS_CLOSE, fd, NULL, error_code);

#ifdef MEMFS_TIME
    t2 = gettime();
    settime(CLOSE_TIME, t2-t1);
    settime(TOTAL_TIME, t2-t1);
#endif

    /* MEMFS_Shutdown(); */

    LOCK_MPI();
}
