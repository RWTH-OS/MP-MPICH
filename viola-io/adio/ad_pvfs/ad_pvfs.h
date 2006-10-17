/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   $Id: ad_pvfs.h,v 1.1.1.1 2005/03/10 16:07:12 mhs Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#ifndef AD_PVFS_INCLUDE
#define AD_PVFS_INCLUDE

#ifndef ROMIOCONF_H_INCLUDED
#include "romioconf.h"
#define ROMIOCONG_H_INCLUDED
#endif
#ifdef ROMIO_PVFS_NEEDS_INT64_DEFINITION
typedef long long int int64_t;
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <pvfs.h>
#include "adio.h"
#include "pvfs_proto.h"

#endif
