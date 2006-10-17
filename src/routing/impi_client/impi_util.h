
#ifndef _IMPI_UTIL_H_
#define _IMPI_UTIL_H_

#include "impi_common.h"

char* IMPI_Get_argv2str(int* argc, char** argv[], char* string, char* end);
char* IMPI_Pop_argv2str(int* argc, char** argv[], char* string, char* end);
void  IMPI_Error(char* msg);

#define IMPI_Int2_ntoh IMPI_Int2_hton
short int IMPI_Int2_hton(short int value);
#define IMPI_Int4_ntoh IMPI_Int4_hton
IMPI_Int4 IMPI_Int4_hton(IMPI_Int4 value);
#define IMPI_Int8_ntoh IMPI_Int8_hton
IMPI_Int8 IMPI_Int8_hton(IMPI_Int8 value);

#endif
