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
* File:         tunnelfs_log.c                                            * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "tunnelfs_log.h"
#include "ad_tunnelfs_err.h"
#include "ad_tunnelfs_common.h"

/**
 * Determine the length of a character array up to a given length.
 * MPE of mpich 1.x provides its own string.h that does not define this
 * function.
 * @param s String to be tested
 * @param maxlen Maximum of characters to be tested.
 * @return Length of the string if string is shorter than maxlen, else maxlen
 */
size_t strnlen(const char *s, size_t maxlen);
/**
 * Length confined sprintf with variable parameter list.
 * MPE of mpich 1.x provides its own string.h that does not define this
 * function.
 * @param buf Target buffer of print statement
 * @param size Maximum count of characters printed to buf
 * @param fmt Format string for print
 * @param args List of arguments
 * @return error code of printf statement
 */
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);

#ifdef LOGGING
int tunnelfs_log_rank;
extern pthread_t tunnelfs_main;
extern pthread_t tunnelfs_service;
extern pthread_t pario_probe;
#endif

/**
 * Convert a number into a string with leading zeros of a given width.
 * @param width Width of the character string.
 * @param num Number to be converted.
 * @return Pointer to character array holding the created string.
 */
char *leadzero(int width, int num)
{
    char *str;
    int i = 0;
    double log_num = 0;

    if (num > 0)
        log_num = floor(log10(num) + 1);
    else
        log_num = 0;

    if ((width > 0) && (log_num < width))
        str = calloc((width + 1), sizeof(char));
    else
        return NULL;

    for (i = 0; i < width - log_num; i++)
    {
        str[i] = '0';
    }
    sprintf((str + i), "%i", num);

    return str;
}

/**
 * printing log messages to stderr, if compiler define switch -DLOGGING is
 * given.
 * @param format Format of log message
 * @param ... List of variables to fill placeholders in format
 */
void tunnelfs_log(const char *format, ...)
{
#ifdef LOGGING
    va_list vargs;
    char *debug_str = NULL;
    char log_header[20];
    int thread_id;

    pthread_t calling_thread = pthread_self();

    ALLOC(debug_str, TUNNELFS_MAX_STRLEN + 1);
    if (debug_str == NULL)
        return;

    /* main thread logs with [xx.1] */
    if (pthread_equal(calling_thread, tunnelfs_main))
        thread_id = 1;
    /* service thread logs with [xx.2] */
    else if (pthread_equal(calling_thread, tunnelfs_service))
        thread_id = 2;
    /* probe thread logs with [xx.3] */
    else if (pthread_equal(calling_thread, pario_probe))
        thread_id = 3;
    /* all other threads log with [xx.0] */
    else
        thread_id = 0;

    /* fill debug message with values */
    va_start(vargs, format);
    vsnprintf(debug_str, TUNNELFS_MAX_STRLEN, format, vargs);
    va_end(vargs);

    /* print message with header */
    sprintf(log_header, "[%2i.%i] (ioserver):", tunnelfs_log_rank, thread_id);
    fprintf(stderr, "%s %s\n", log_header, debug_str);

    FREE(debug_str);

    return;
#endif
}
