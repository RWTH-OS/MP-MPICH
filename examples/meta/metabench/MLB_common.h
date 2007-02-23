#ifndef _MBL_COMMON_H_
#define _MBL_COMMON_H_

#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/****************************************/

#define MLB_HIERARCHY_TOLERANCE 0.5
#define MLB_TABLE_BREAK_COUNT 4
#define MLB_TABLE_MAX_COUNT 16

/****************************************/
/**     DO _NOT_ CHANGE:               **/
#define MLB_ABORT_RESIDUAL 0.001
#define MLB_BOUNDARY_VALUE 273.15
#define MLB_INNER_START_VALUE 0.0

/****************************************/

#define MLB_LOCAL_TAG 88
#define MLB_INTER_TAG 99

/****************************************/

typedef enum _MLB_Classes {A,B,C,W,S} MLB_Classes;
extern const unsigned int MLB_Class_size[];

typedef enum _MLB_Op {MLB_MAX, MLB_SUM} MLB_Op;

#define true  1
#define false 0

#endif
