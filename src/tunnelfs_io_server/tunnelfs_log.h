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
* File:         tunnelfs_log.h                                            * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#ifndef _TUNNELFS_LOG_H
#define _TUNNELFS_LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include "pario_threads.h"

/**
 * Rank of tunnelfs server using the log mechanism
 */
extern int tunnelfs_log_rank;
/**
 * printing log messages to stderr, if compiler define switch -DLOGGING is
 * given.
 * @param format Format of log message
 * @param ... List of variables to fill placeholders in format
 */
void tunnelfs_log(const char *format, ...);
/**
 * Convert a number into a string with leading zeros of a given width.
 * @param width Width of the character string.
 * @param num Number to be converted.
 * @return Pointer to character array holding the created string.
 */
char *leadzero(int width, int num);

#ifdef LOGGING

/* needed to fill the log rank with the correct value */
#define LOG_INIT MPI_Comm_rank(MPI_COMM_WORLD, &tunnelfs_log_rank);
/* logging is activated, thus the LOG is mapped to the function call */
#define LOG tunnelfs_log

#else

/* no logging, thus no init required */
#define LOG_INIT
/* logging disabled, thus the log call is mapped to a construct that is never
 * executed an can be thrown out by the compiler in opimization phase */
#define LOG if (0) tunnelfs_log
#endif

#endif
