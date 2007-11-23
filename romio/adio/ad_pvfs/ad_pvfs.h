/* 
 *   $Id: ad_pvfs.h 2177 2003-05-12 17:36:53Z joachim $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#ifndef AD_PVFS_INCLUDE
#define AD_PVFS_INCLUDE

#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <pvfs.h>
#include "adio.h"
#include "pvfs_proto.h"

#ifdef MPIO_BUILD_PROFILING
/* Include mapping from ADIOI_PVFS->PADIOI_PVFS */
#define PVFS_BUILD_PROFILING
#include "pvfsprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif

#endif
