/* $Id: utildefs.h,v 1.7 2004/03/09 10:04:28 joachim Exp $ */

#ifndef _MPID_UTILDEFS_H
#define _MPID_UTILDEFS_H

/* some common definitions */
#include "mpichconf.h"

#include <stdlib.h>
#include <stdio.h>

#include <../ch2/mpid.h>

/* checked memory allocation */
#define MPID_ALLOCATE(ptr,type,size) \
    if ((ptr = (type) malloc (size)) == NULL) { \
        fprintf(stderr,"[%d] MP-MPICH ERROR (%s:%d) : out of local memory\n",\
                MPID_MyWorldRank,__FILE__,__LINE__); \
        fflush(stderr); \
    	exit (MPI_ERR_INTERN); \
    } 

#define MPID_ZALLOCATE(ptr,type,size) \
    if ((ptr = (type) malloc (size)) == NULL) { \
        fprintf(stderr,"[%d] MP-MPICH ERROR (%s:%d) : out of local memory\n",\
                MPID_MyWorldRank,__FILE__,__LINE__); \
        fflush(stderr); \
    	exit (MPI_ERR_INTERN); \
    } ; memset (ptr, 0, size);


#define MPID_UTIL_THREADSAFE        1
#define MPID_UTIL_MALLOCED_DATA     1<<1
#define MPID_UTIL_IGNORE_DUPLICATES 1<<2

#endif
