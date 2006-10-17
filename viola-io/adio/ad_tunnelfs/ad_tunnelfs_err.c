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
* File:         tunnelfs_err.c                                            * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "ad_tunnelfs_err.h"

#define FPRN(x) fprintf(stderr, "%s, Line %i: TUNNELFS ERROR %i: %s\n",\
    file, line, err_code, x);

/**
 * Print error message
 * @param err_code Type of error
 * @param file file the error occured in
 * @param line line the error occured in
 * @param do_exit flag indicating a fatal error
 */
void errmsg(int err_code, char *file, int line, int do_exit)
{
    switch (err_code)
    {
    case TUNNELFS_ERR_ALLOC:
        {
            FPRN("Could not allocate memory.");
            break;
        }
    case TUNNELFS_ERR_OUT_OF_BOUNDS:
        {
            FPRN("Index out of bounds.");
            break;
        }
    case TUNNELFS_ERR_NOT_FOUND:
        {
            FPRN("Key not found.");
            break;
        }
    case TUNNELFS_ERR_NULL_POINTER:
        {
            FPRN("Null pointer referenced.");
            break;
        }
    case TUNNELFS_ERR_DUPLICATE:
        {
            FPRN("Duplicate entry found.");
            break;
        }
    case TUNNELFS_ERR_SIZE_IS_ZERO:
        {
            FPRN("Given size parameter is zero (0).");
            break;
        }
    default:
        {
            FPRN("No error description available.");
            break;
        }
    }
    if (do_exit)
        exit(err_code);
}
