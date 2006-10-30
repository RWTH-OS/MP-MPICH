/* Header for testing procedures */

#ifndef _INCLUDED_TEST_H_
#define _INCLUDED_TEST_H_

#if defined(NEEDS_STDLIB_PROTOTYPES)
#include "protofix.h"
#endif

#ifdef _DEBUG
#define DBM(m) printf(m);
#else
#define DBM(m)
#endif

void Test_Init (char *);
void Test_Init_No_File (void);
void Test_Message (char *);
void Test_Failed (char *);
void Test_Passed (char *);
int Summarize_Test_Results (void);
int Test_Global_Summary (void);
void Test_Finalize (void);
void Print_Filecontent(char *, int);
void Test_Waitforall (void);

#endif
