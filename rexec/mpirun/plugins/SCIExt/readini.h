#ifndef __READINI_H__
#define __READINI_H__

#include <stdio.h>
#include <string.h>
#include "SciExt.h"

#define ERR_SCIINFO_OK 0
#define ERR_SCIINFO_NOTFOUND 1
#define ERR_SCIINFO_READERROR 2

#define TOK_NUMADAPTER "NumAdapter="
#define TOK_SCIID "SciId"
#define TOK_SCIRING "SciRing"

#define BUFFSIZE 2048

typedef struct SciInfo_{
    int NumAdapter;
    int SciId[MAXADAPTERS];
    int SciRing[MAXADAPTERS];
} SciInfo_t;


int ReadIniFile(char* szFileName, SciInfo_t *pSciInfo);

#endif /* __READINI_H__ */
