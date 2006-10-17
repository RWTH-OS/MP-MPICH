/**************************************************************************
* TunnelFS Server                                                         * 
***************************************************************************
*                                                                         * 
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              * 
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
*                                                                         * 
* See COPYRIGHT notice in base directory for details                      * 
**************************************************************************/
/**************************************************************************
* File:         tunnelfs_err.h                                            * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#ifndef AD_TUNNELFS_ERR_H
#define AD_TUNNELFS_ERR_H

#include <stdio.h>

/**
 * Fatal Error
 */
#define ERR(x)  errmsg(x, __FILE__, __LINE__, 1)

/**
 * Non-Fatal Error
 */
#define WARN(x) errmsg(x, __FILE__, __LINE__, 0);

/**
 * Allocate memory with precautions
 */
#define ALLOC(x,y) if (x != NULL) free(x); \
                   x = malloc(y); \
                   if (x == NULL) ERR(TUNNELFS_ERR_ALLOC);
/**
 * Free memory with precautions
 */
#define FREE(x) if (x != NULL) free(x); x = NULL;

/**
 * Do a string copy with forced null termination 
 */
#define STRNCPY(dest, src, length)  strncpy(dest, src, length); \
                                    (dest)[length] = '\0';

/**
 * Error while allocation memory
 */
#define TUNNELFS_ERR_ALLOC          1

/**
 * Accessing an array out of bounds
 */
#define TUNNELFS_ERR_OUT_OF_BOUNDS  2

/**
 * No entry found in a search
 */
#define TUNNELFS_ERR_NOT_FOUND      3

/**
 * Pointer not sufficiently allocated
 */
#define TUNNELFS_ERR_NULL_POINTER   4

/**
 * Duplicate entry
 */
#define TUNNELFS_ERR_DUPLICATE      5

/**
 * Size parameter is zero where it should be positive
 */
#define TUNNELFS_ERR_SIZE_IS_ZERO   6

/**
 * Print error message
 * @param err_code Type of error
 * @param file file the error occured in
 * @param line line the error occured in
 * @param do_exit flag indicating a fatal error
 */
void errmsg(int err_code, char *file, int line, int do_exit);

#endif
