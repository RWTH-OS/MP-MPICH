
#ifndef _IMPI_COMMON_H_
#define _IMPI_COMMON_H_

#include "impi_defs.h"
#include "impi_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMPI_MODULE_NAME "Common"

#ifdef _IMPI_DEBUG_
#define DBG(msg) { printf("### IMPI-%s: %s\n", IMPI_MODULE_NAME, msg); fflush(stdout); }
#define DBG1(msg,a) { printf("### IMPI-%s: ", IMPI_MODULE_NAME); printf(msg,a); printf("\n"); fflush(stdout); }
#define DBG2(msg,a,b) { printf("### IMPI-%s: ", IMPI_MODULE_NAME); printf(msg,a,b); printf("\n"); fflush(stdout); }
#define DBG3(msg,a,b,c) { printf("### IMPI-%s: ", IMPI_MODULE_NAME); printf(msg,a,b,c); printf("\n"); fflush(stdout); }
#define DBG4(msg,a,b,c,d) { printf("### IMPI-%s: ", IMPI_MODULE_NAME); printf(msg,a,b,c,d); printf("\n"); fflush(stdout); }
#else
#define DBG(msg) {}
#define DBG1(msg,a) {}
#define DBG2(msg,a,b) {}
#define DBG3(msg,ab,b,c) {}
#define DBG4(msg,ab,b,c,d) {}
#endif

#endif

