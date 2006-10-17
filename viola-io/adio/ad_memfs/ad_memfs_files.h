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
* File:         ad_memfs_files.h                                          * 
* Description:                                                            *
* Author(s):    Jan Seidel <Jan.Seidel@fh-bonn-rhein-sieg.de>             * 
* 		Marcel Birkner <Marcel.Birkner@fh-bonn-rhein-sieg.de>     *
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/ 

#ifndef AD_MEMFS_FILES_H
#define AD_MEMFS_FILES_H

/* Protocol Version */
#define MEMFS_VERSION_MAJ 1
#define MEMFS_VERSION_MIN 0

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* Assertions with assert()            			*/
/* Disable Assertions for the compiler: cc -dNDEBUG 	*/
/* #define NDEBUG */ 
#include <assert.h>

#include "ad_memfs.h"

#include <sys/time.h>
#include <stddef.h>


/* Include for memory debugger DMALLOC */
#if 0
#ifdef DMALLOC
#include "dmalloc.h"
#endif
#endif

int memfs_open(char *name, int accessmode, int64_t blocksize, int position);
int memfs_close(int fh);
int memfs_write(int fh, ADIO_Offset filepointer, char *buf, int size);
int memfs_read(int fh, ADIO_Offset filepointer, char *buf, int size);
int memfs_del(char *filename);
int memfs_resize(int fh, ADIO_Offset size);
int memfs_getfilesize(int fh);

int memfs_lock_file(int fh, int *blocks, int size, int num_server);
int memfs_unlock_file(int fh, int *blocks, int size, int num_server);

int memfs_get_exclusive(int fh);
int memfs_set_exclusive(int fh, int exclusive);
int64_t memfs_get_blocksize(int fh);
#endif 

