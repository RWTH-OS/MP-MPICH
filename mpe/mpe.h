/* MPE_Graphics should not be included here in case the system does not
   support the graphics features. */

#ifndef _MPE_INCLUDE
#define _MPE_INCLUDE

#ifndef ANSI_ARGS
#if defined(__STDC__) || defined(__cplusplus)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(MPE_GRAPHICS) || defined(WIN32)
    #ifdef WIN32
	#include "mpe_win_graphics.h"
    #else
	#include "mpe_graphics.h"
    #endif /*WIN32*/
#endif /*MPE_GRAPHICS*/
#include "mpe_log.h"

#ifdef WIN32
#define MPE_SUCCESS        ERROR_SUCCESS	    /* successful operation */
#define MPE_ERR_NOXCONNECT ERROR_CONNECTION_REFUSED /* could not connect to X server */
#define MPE_ERR_BAD_ARGS   ERROR_INVALID_PARAMETER  /* graphics handle invalid */
#define MPE_ERR_LOW_MEM    ERROR_OUTOFMEMORY        /* out of memory (malloc() failed) */
#else
#define MPE_SUCCESS        0		/* successful operation */
#define MPE_ERR_NOXCONNECT 1		/* could not connect to X server */
#define MPE_ERR_BAD_ARGS   2            /* graphics handle invalid */
#define MPE_ERR_LOW_MEM    3	        /* out of memory (malloc() failed) */
#endif

#if defined(FORTRANNOUNDERSCORE)

#define mpe_initlog_          mpe_initlog
#define mpe_startlog_         mpe_startlog
#define mpe_stoplog_          mpe_stoplog
#define mpe_describe_state_   mpe_describe_state
#define mpe_describe_event_   mpe_describe_event
#define mpe_log_event_        mpe_log_event
#define mpe_finishlog_        mpe_finishlog

#endif

IMPORT_MPI_API void MPE_Seq_begin ANSI_ARGS(( MPI_Comm, int ));
IMPORT_MPI_API void MPE_Seq_end   ANSI_ARGS(( MPI_Comm, int ));

IMPORT_MPI_API int MPE_DelTag     ANSI_ARGS(( MPI_Comm, int, void *, void * ));
IMPORT_MPI_API int MPE_GetTags    ANSI_ARGS(( MPI_Comm, int, MPI_Comm *, int * ));
IMPORT_MPI_API int MPE_ReturnTags ANSI_ARGS(( MPI_Comm, int, int ));
IMPORT_MPI_API int MPE_TagsEnd    ANSI_ARGS(( void ));

IMPORT_MPI_API void MPE_IO_Stdout_to_file ANSI_ARGS(( char *, int ));

IMPORT_MPI_API void MPE_GetHostName       ANSI_ARGS(( char *, int ));

IMPORT_MPI_API void MPE_Start_debugger ANSI_ARGS(( void ));
#if (defined(__STDC__) || defined(__cplusplus))
IMPORT_MPI_API void MPE_Errors_to_dbx ANSI_ARGS(( MPI_Comm *, int *, ... ));
#else
IMPORT_MPI_API void MPE_Errors_to_dbx ANSI_ARGS(( MPI_Comm *, int *, char *, char *, int * ));
#endif
IMPORT_MPI_API void MPE_Errors_call_debugger ANSI_ARGS(( char *, char *, char ** ));
IMPORT_MPI_API void MPE_Errors_call_xdbx     ANSI_ARGS(( char *, char * ));
IMPORT_MPI_API void MPE_Errors_call_dbx_in_xterm ANSI_ARGS(( char *, char * ));
IMPORT_MPI_API void MPE_Signals_call_debugger ANSI_ARGS(( void ));

IMPORT_MPI_API int MPE_Decomp1d ANSI_ARGS(( int, int, int, int *, int * ));

IMPORT_MPI_API void MPE_Comm_global_rank ANSI_ARGS(( MPI_Comm, int, int * ));

#if defined(__cplusplus)
}
#endif

#endif
