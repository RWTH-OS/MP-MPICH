/***************************************************************************
                          environment.cc  -  description
                             -------------------
    begin                : Don Aug 15 2002
    copyright            : (C) 2002 by silke
    email                : silke@oskar
 ***************************************************************************/

#define _DEBUG_EXTERN_REC
#include "mydebug.h"

#include "nt2unix.h"
#include <stdio.h>

extern char** environ;


/* adapted from http://www.sct.gu.edu.au/~anthony/info/C/C.algorithms */
/* environ[-2] downto environ[-2-(argc-1)] contains pointers to the   */
/* corresponding argv[i] in the opposit order                         */
/* argc itself is located at environ[-2-(argc)], so it has to be      */
/* searched this strange way */
static int argc() {
  int             ac = 1;
  while ((int)environ[ -2 - ac ] != ac) ac++;
  return (ac);
}

WINBASEAPI LPSTR WINAPI GetCommandLine (VOID)
{
    DSECTION("GetCommandLine");
    static LPSTR Retval = NULL;
    int i = 0;
    int size = 0;
    int off = 0;
    int count;
    
    DSECTENTRYPOINT;
    
    
    if (Retval == NULL) {
	count = argc();
	for(i=0; i<count; i++) {
	    size += strlen(environ[-2-(count-i-1)]) + 1;
	}
	size++;
	Retval = (char*) malloc(sizeof(char) * size);
	for(i=0; i<count; i++) {
	    sprintf(Retval+off,"%s ",environ[-2-(count-i-1)]);
	    off += strlen(environ[-2-(count-i-1)]) + 1;
	}
	Retval[size-1] = '\0';
    }
    
    DSECTLEAVE
	return (Retval);
}


WINBASEAPI LPSTR WINAPI GetEnvironmentStrings(VOID
)
{
    DSECTION("GetEnvironmentStrings");
    static LPSTR Retval = NULL;
    int i;
    int size = 0;
    int off =0;
    
    DSECTENTRYPOINT;
    
    if (Retval == NULL) {
	
	for (i=0; environ[i] != NULL; i++) {
	    size += strlen(environ[i]) + 1; 
	}
	size++;
	
	Retval = (char*) malloc(sizeof(char) * size);
	
	for (i=0; environ[i] != NULL; i++) {
	    strcpy(Retval+off, environ[i]);
	    off += strlen(environ[i]) + 1;
	}
	Retval[size-1] = '\0';
    }
    
    DSECTLEAVE
	return(Retval);
}

WINBASEAPI VOID WINAPI FreeEnvironmentStrings( LPSTR EnvStrings )
{
  free(EnvStrings);
}

BOOL SetEnvironmentVariable(LPCTSTR lpName,  // address of environment variable name 
			    LPCTSTR lpValue  // address of new value for variable 
)
{
    DSECTION("SetEnvironmentVariable");
    const char* Value = lpValue;
    char* Name_Value;
    int Ret;
    
    DSECTENTRYPOINT;
    
    Name_Value = strcat (lpName, Value);
    
    Ret = putenv (Name_Value);
    
    DSECTLEAVE
	return(Ret?0:1);
}

DWORD GetEnvironmentVariable(LPCTSTR lpName,  // address of environment variable name 
			     LPTSTR lpBuffer, // address of buffer for variable value 
			     DWORD nSize      // size of buffer, in characters 
)
{
    DSECTION("GetEnvironmentVariable");
    
    DSECTENTRYPOINT;
    
    if (!lpName || !lpBuffer || sizeof(getenv (lpName)) > nSize) {
	DSECTLEAVE
	    return 0;
    }
    
    lpBuffer = getenv (lpName);

    DSECTLEAVE
	return sizeof (lpBuffer);
}
