#ifndef MEMFS_TIME_H
#define MEMFS_TIME_H


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "ad_memfs.h"

double gettime(void);

void settime(int function, double value);

int timeInitialized();

void initTimeMeasurement();

void printTimeMeasurement(int rank);

#define READ_TIME              0 
#define READ_MAIN              1
#define READ_SETUP             2
#define LOCAL_READ             3
#define REMOTE_READ            4
#define READ_SETLOCK           5
#define READ_REMOVELOCK        6
#define READ_SERVICE           7

#define WRITE_TIME             8
#define WRITE_MAIN             9
#define WRITE_SETUP            10  
#define FILESYSTEM_WRITE       11
#define REMOTE_WRITE           12
#define WRITE_GETERROR         13 
#define WRITE_SETLOCK	       14
#define WRITE_REMOVELOCK       15
#define WRITE_SERVICE          16

#define OPEN_TIME              17
#define CLOSE_TIME             18

#define REFERENCE              19	
#define FS_LOCK                20

#define TOTAL_TIME             21

#endif
