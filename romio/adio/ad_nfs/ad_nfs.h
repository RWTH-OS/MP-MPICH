/* 
 *   $Id: ad_nfs.h 2177 2003-05-12 17:36:53Z joachim $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#ifndef AD_NFS_INCLUDE
#define AD_NFS_INCLUDE

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "adio.h"

#ifndef NO_AIO
#ifdef AIO_SUN
#include <sys/asynch.h>
#else
#include <aio.h>
#endif
#endif

#ifdef MPIO_BUILD_PROFILING
/* Include mapping from ADIOI_PVFS->PADIOI_PVFS */
#define NFS_BUILD_PROFILING
#include "nfsprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif

int ADIOI_NFS_aio(ADIO_File fd, void *buf, int len, ADIO_Offset offset,
                  int wr, void *handle);

#endif
