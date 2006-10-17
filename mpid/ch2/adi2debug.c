/*
 *  $Id$
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */


#include <string.h>

#include "mpid.h"
#include "mpiddev.h"
#include "chpackflow.h"

/* 
   Unfortunately, stderr is not a guarenteed to be a compile-time
   constant in ANSI C, so we can't initialize MPID_DEBUG_FILE with
   stderr.  Instead, we set it to null, and check for null.  Note
   that stdout is used in chinit.c 
 */
FILE *MPID_DEBUG_FILE = 0;
FILE *MPID_TRACE_FILE = 0;
int MPID_DebugFlag = 0;

/* the packets are printed by each device as each device may have it's own
   packet layout */

void MPID_Print_Send_Handle( shandle )
MPIR_SHANDLE *shandle;
{
    FPRINTF( stdout, "[%d]* dmpi_send_contents:\n\
* totallen    = %d\n\
* recv_handle = %p\n", MPID_MyWorldRank, 
		 shandle->bytes_as_contig, 
		 shandle->recv_handle );
}

void MPID_SetDebugFile( name )
char *name;
{
    char filename[1024];
    
    if (strcmp( name, "-" ) == 0) {
	MPID_DEBUG_FILE = stdout;
	return;
    }
    if (strchr( name, '%' )) {
	SPRINTF( filename, name, MPID_MyWorldRank );
	MPID_DEBUG_FILE = fopen( filename, "w" );
    }
    else
	MPID_DEBUG_FILE = fopen( name, "w" );

    if (!MPID_DEBUG_FILE) MPID_DEBUG_FILE = stdout;
}

void MPID_Set_tracefile( name )
char *name;
{
    char filename[1024];

    if (strcmp( name, "-" ) == 0) {
	MPID_TRACE_FILE = stdout;
	return;
    }
    if (strchr( name, '%' )) {
	SPRINTF( filename, name, MPID_MyWorldRank );
	MPID_TRACE_FILE = fopen( filename, "w" );
    }
    else
	MPID_TRACE_FILE = fopen( name, "w" );

    /* Is this the correct thing to do? */
    if (!MPID_TRACE_FILE)
	MPID_TRACE_FILE = stdout;
}

void MPID_SetSpaceDebugFlag( flag )
int flag;
{
/*      DebugSpace = flag; */
#ifdef CHAMELEON_COMM   /* #CHAMELEON_START# */
/* This file may be used to generate non-Chameleon versions */
    if (flag) {
	/* Check the validity of the malloc arena on every use of 
	   trmalloc/free */
	trDebugLevel( 1 );
    }
#endif                  /* #CHAMELEON_END# */
}
void MPID_SetDebugFlag( f )
int f;
{
    MPID_DebugFlag = f;
}

/*
   Data about messages
 */
static int DebugMsgFlag = 0;
void MPID_SetMsgDebugFlag( f )
int f;
{
    DebugMsgFlag = f;
}
int MPID_GetMsgDebugFlag()
{
    return DebugMsgFlag;
}
void MPID_PrintMsgDebug()
{
}

/*
 * Print information about a request
 */
void MPID_Print_rhandle( fp, rhandle )
FILE *fp;
MPIR_RHANDLE *rhandle;
{
    FPRINTF( fp, "rhandle at %p\n\
\tcookie     \t= %lx\n\
\tis_complete\t= %d\n\
\tbuf        \t= %p\n", 
	     rhandle, 
#ifdef MPIR_HAS_COOKIES
	     rhandle->cookie, 
#else
	     0,
#endif
	     rhandle->is_complete, 
	     rhandle->buf );
}
void MPID_Print_shandle( fp, shandle )
FILE *fp;
MPIR_SHANDLE *shandle;
{
    FPRINTF( fp, "shandle at %p\n\
\tcookie     \t= %lx\n\
\tis_complete\t= %d\n\
\tstart      \t= %p\n\
\tbytes_as_contig\t= %d\n\
", 
	     shandle, 
#ifdef MPIR_HAS_COOKIES
	     shandle->cookie, 
#else
	     0,
#endif
	     shandle->is_complete, 
	     shandle->start,
	     shandle->bytes_as_contig
 );
}
