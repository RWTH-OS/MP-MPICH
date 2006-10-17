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
* File:         ad_memfs_functions.h                                      *
* Description:                                                            *
* Author(s):    Jan Seidel <Jan.Seidel@fh-bonn-rhein-sieg.de>             *
* Last change:                                                            *
*                                                                         *
* Changelog:                                                              *
**************************************************************************/

#ifndef AD_MEMFS_FUNCTIONS_H
#define AD_MEMFS_FUNCTIONS_H


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "adio.h"
#include "adio_extern.h"
#include "pario_threads.h"
#include "ad_memfs_files.h"
#include "ad_memfs.h"

int open_file(char *filename, int access_mode, int *error, int64_t blocksize, int pos);
void close_file(int fd_sys, int *error);
void read_file_contig(ADIO_File fd, void *buf, int count,
               MPI_Datatype datatype, int file_ptr_type,
               ADIO_Offset offset, ADIO_Status *status, int *error);
void read_file_strided(ADIO_File fd, void *buf, int count,
               MPI_Datatype datatype, int file_ptr_type,
               ADIO_Offset offset, ADIO_Status *status, int *error);

void write_file_contig(ADIO_File fd, void *buf, int count,
               MPI_Datatype datatype, int file_ptr_type,
               ADIO_Offset offset, ADIO_Status *status, int *error);
void write_file_strided(ADIO_File fd, void *buf, int count,
               MPI_Datatype datatype, int file_ptr_type,
               ADIO_Offset offset, ADIO_Status *status, int *error);
void resize_file(ADIO_File fd, ADIO_Offset size, int *error);
void delete_file(char *filename, int *error);
void fcntl_file(ADIO_File fd, int flag, ADIO_Fcntl_t *fcntl_struct, int *error_code);
ADIO_Offset seekind_file(ADIO_File fd, ADIO_Offset offset,
                      int whence, int *error_code);
void setinfo_file(ADIO_File fd, MPI_Info users_info, int *error_code);
int get_exclusive(int fh);
void set_exclusive(int fh, int exclusive);
#endif
