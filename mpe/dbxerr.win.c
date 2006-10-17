#if !defined _WIN32_WINNT 
#define _WIN32_WINNT 0x0400
#endif

#ifdef HAVE_MPICHCONF_H
#include "mpichconf.h"
#else
#include "mpeconf.h"
#endif

#if defined(HAVE_STRING_H) || defined(STDC_HEADERS)
#include <string.h>
#endif
#include "mpi.h"
#include "mpe.h"
#include "mpeexten.h"


#include <winbase.h>
#include <stdio.h>
#include <stdlib.h>

#define DBX_NAME "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug"

/* Prototypes for local routines */
char *MPER_Copy_string ANSI_ARGS(( char * ));
LONG CallDebugger(EXCEPTION_POINTERS *ExceptionInfo);

static char *debugger=0;
static HANDLE hEvent=NULL;

#if (defined(__STDC__) || defined(__cplusplus))
#define MPIR_USE_STDARG
#include <stdarg.h>
#endif

static char *GetDebuggerCommandline() {
    HKEY hKey;
    DWORD sizeNeeded=0,sizeUsed=0,Type;
    char *buf=0,*bufNew,*pos,*newPos;
    char ProcId[10],Event[10];
    SECURITY_ATTRIBUTES Sattr={sizeof(Sattr),0,TRUE};

    LONG res;

    if(RegOpenKey(HKEY_LOCAL_MACHINE,DBX_NAME,&hKey) != ERROR_SUCCESS)
	return 0;
    res = RegQueryValueEx(hKey,"Debugger",0,&Type,(LPBYTE)buf,&sizeNeeded);
    if(res != ERROR_SUCCESS) {
	RegCloseKey(hKey);
	return 0;
    }
    buf = (char*)malloc(sizeNeeded*sizeof(char));
    if(!buf) {
	RegCloseKey(hKey);
	return 0;
    
    }
    res = RegQueryValueEx(hKey,"Debugger",0,&Type,(LPBYTE)buf,&sizeNeeded);
    RegCloseKey(hKey);

    if(res != ERROR_SUCCESS) {
	free(buf);
	return 0;
    }

    sprintf(ProcId,"%u",GetCurrentProcessId());

    if(hEvent==NULL) {
        hEvent = CreateEvent(&Sattr,FALSE,FALSE,0);
        if(hEvent==NULL) {
	    free(buf);
	    return 0;
	}
    } else ResetEvent(hEvent);
#ifdef _WIN64
	sprintf(Event,"%I64u",hEvent);
#else
    sprintf(Event,"%u",hEvent);
#endif
    pos = strstr(buf,"%ld");
    while(pos) {
	sizeNeeded += 7;
	pos = strstr(pos+3,"%ld");
    }
    bufNew = (char*)malloc(sizeNeeded);
    pos = buf;
    newPos = bufNew;
    while(*pos) {
	if(*pos != '%') *newPos++ = *pos++;
	else {
	    if(pos[-2]=='p') {
		strcpy(newPos,ProcId);
		newPos += strlen(ProcId);
	    } else {
		strcpy(newPos,Event);
		newPos += strlen(Event);
	    }
	    pos += 3;
	}
    }
    *newPos = 0;
    free(buf);
    return bufNew;


}


static int AttachDebugger() {
    STARTUPINFO SI;
    PROCESS_INFORMATION PI;
    
    if(IsDebuggerPresent()) 
	return 1;
    if(!debugger) {
	
	debugger = GetDebuggerCommandline();
	if(!debugger) {
	    fprintf(stderr,"No system debugger configured\n");
	    return 0;
	}
    }
 
    memset (&SI,0,sizeof(SI));
    fprintf(stderr,"Staring %s\n",debugger);
    if(!CreateProcess(0,debugger,0,0,TRUE,CREATE_DEFAULT_ERROR_MODE,0,0,&SI,&PI)) {
	fprintf(stderr,"Could not start debugger %s\n Error: %d\n",debugger,GetLastError());
	return 0;
    }

    CloseHandle(PI.hProcess);
    CloseHandle(PI.hThread);
    WaitForSingleObject(hEvent,INFINITE);
    CloseHandle(hEvent);
    hEvent=NULL;
    return 1;
}

/* The exception filter used to start the debugger
   if MPE_Signals_call_debugger() was called */
LONG CallDebugger(EXCEPTION_POINTERS *ExceptionInfo) {
    if(!IsDebuggerPresent() && !AttachDebugger())
        return EXCEPTION_CONTINUE_SEARCH;

    return EXCEPTION_CONTINUE_EXECUTION;
}

void MPE_Start_debugger( ){   
    
    if(AttachDebugger()) {
	DebugBreak();
    }
    
}

/* This is the actual error handler */
#ifdef MPIR_USE_STDARG
void MPE_Errors_to_dbx( MPI_Comm *comm, int * code, ... )
{
  char buf[MPI_MAX_ERROR_STRING];
  int  myid, result_len; 
  char *string, *file;
  int  *line;
  va_list Argp;

  va_start( Argp, code );
  string = va_arg(Argp,char *);
  file   = va_arg(Argp,char *);
  line   = va_arg(Argp,int *);
  va_end( Argp );
#else
void MPE_Errors_to_dbx( comm, code, string, file, line )
MPI_Comm *comm;
int      *code, *line;
char     *string, *file;
{
  char buf[MPI_MAX_ERROR_STRING];
  int  myid, result_len; 
#endif

  if (MPI_COMM_WORLD) MPI_Comm_rank( MPI_COMM_WORLD, &myid );
  else myid = -1;
  MPI_Error_string( *code, buf, &result_len );
  fprintf( stderr, "%d -  File: %s   Line: %d\n", myid, 
		   file, *line );
  fprintf( stderr, "%d - %s : %s\n", myid, 
          string ? string : "<NO ERROR MESSAGE>", buf );

  MPE_Start_debugger();
}

/*@
   MPE_Errors_call_debugger - On an error, print a message and (attempt) to
   start the specified debugger on the program

   Input Parameters:
+  pgm - Name of the program.
.  dbg - Name of the debugger.  If null, use a default (usually dbx)
-  args - arguments to use in generating the debugger.
   This allows things like "'xterm -e dbx pgm pid'", or 
   "'xdbx -geometry +%d+%d pgm pid'".  The list should be null terminated.
   (The '%d %d' format is not yet supported).

    Notes:
    You may need to ignore some signals, depending on the signals that
    the MPICH and underlying communications code is using.  You can
    do this in dbx by adding "ignore signal-name" to your .dbxinit file.
    For example, to ignore 'SIGUSR1', use "'ignore USR1'".

    Currently, there is no Fortran interface for this routine.
@*/
void MPE_Errors_call_debugger( pgm, dbg, args )
char *pgm, *dbg, **args;
{
    MPI_Errhandler err;
    DWORD nbaseargs=0,i;
    DWORD commandlen;
    BOOL freedbg = FALSE;
    if(debugger) {
	free(debugger);
	debugger = 0;
    }

    if(!dbg) {
	dbg = GetDebuggerCommandline();
	if(!dbg) {
	    fprintf(stderr,"No system debugger specified...\n");
	    return;
	}
	freedbg = TRUE;
    }
    commandlen = strlen(dbg)+1;
    if (args) {
	while (args[nbaseargs]) {
	    commandlen += strlen(args[nbaseargs])+1;
	    nbaseargs++;
	}
    }
    

    debugger = (char *)malloc( commandlen +1 );
    strcpy( debugger, dbg );
    for (i=0; i<nbaseargs; i++) {
	strcat(debugger," ");
	strcat(debugger,args[i]);
    }
    if(freedbg) free(dbg);


    MPI_Errhandler_create( (MPI_Handler_function *)MPE_Errors_to_dbx, &err );
    MPI_Errhandler_set( MPI_COMM_WORLD, err );
}

char *MPER_Copy_string( str )
char *str;
{
    char *new;
    new = (char *)malloc( strlen(str) + 1 );
    strcpy( new, str );
    return new;
}

void MPE_Errors_call_xdbx( pgm, display )
char *pgm, *display;
{

    MPE_Errors_call_debugger( pgm, (char *)0, 0 );
}

/* This routine is collective; all processes in MPI_COMM_WORLD must call */
void MPE_Errors_call_dbx_in_xterm( pgm, display )
char *pgm, *display;
{

    MPE_Errors_call_debugger( pgm, (char *)0, 0 );
}

/* This routine is collective; all processes in MPI_COMM_WORLD must call */
void MPE_Errors_call_gdb_in_xterm( pgm, display )
char *pgm, *display;
{

    MPE_Errors_call_debugger( pgm, (char *)0, 0 );
}



/*@
    MPE_Signals_call_debugger - Process-killing signals invoke the MPE
    error handler

    Notes:
    This invokes the MPE error handler which prints some information
    about the signal and then attempts to start the debugger.  Some
    systems will not support calling the debugger.

    You may need to ignore some signals, depending on the signals that
    the MPICH and underlying communications code is using.  You can
    do this in dbx by adding "'ignore signal-name'" to your .dbxinit file.
    For example, to ignore 'SIGUSR1', use "'ignore USR1'".

    Currently, there is no Fortran interface for this routine.
@*/
void MPE_Signals_call_debugger()
{
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)CallDebugger);
}

