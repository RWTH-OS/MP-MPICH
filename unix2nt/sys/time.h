/* $Id: time.h,v 1.1 2001/11/08 23:05:51 martin Exp $ */

#ifndef _SYS_TIME_H_
#define _SYS_TIME_H_

#include <time.h>
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

/* This function runs always correct. */
int gettimeofday(struct timeval *, void *);

#ifdef __cplusplus
}
#endif

#endif