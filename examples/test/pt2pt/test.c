/* Procedures for recording and printing test results */

#include <stdio.h>
#include <string.h>
#include "test.h"

#if defined(__STDC__) || defined (USE_STDARG) || defined (WIN32)
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#define VERBOSE
static int tests_passed = 0;
static int tests_failed = 0;
static char failed_tests[255][81];
static char suite_name[255];
FILE *fileout = NULL;

void Test_Init(char *suite, int rank)
{
    char filename[512];

    sprintf(filename, "%s-%d.out", suite, rank);
    strncpy(suite_name, suite, 255);
    fileout = fopen(filename, "w");
    if (!fileout) {
	fprintf( stderr, "Could not open %s on node %d\n", filename, rank );
	MPI_Abort( MPI_COMM_WORLD, 1 );
    }

    MPI_Errhandler_create( Test_Errors_warn, &TEST_ERRORS_WARN );
}

#if defined(__STDC__) || defined(USE_STDARG) || defined (WIN32)
void Test_Printf(char *format, ...)
{
    va_list arglist;

    va_start(arglist, format);
    (void)vfprintf(fileout, format, arglist);
    va_end(arglist);
}
#else
void Test_Printf(va_alist)
va_dcl
{
    char *format;
    va_list arglist;

    va_start(arglist);
    format = va_arg(arglist, char *);
    (void)vfprintf(fileout, format, arglist);
    fflush(fileout);
    va_end(arglist);
}
#endif

void Test_Message(char *mess)
{
    fprintf(fileout, "[%s]: %s\n", suite_name, mess);
    fflush(fileout);
}

void Test_Failed(char *test)
{
    fprintf(fileout, "[%s]: *** Test '%s' Failed! ***\n", suite_name, test);
    strncpy(failed_tests[tests_failed], test, 81);
    fflush(fileout);
    tests_failed++;
}

void Test_Passed(char *test)
{
#ifdef VERBOSE
    fprintf(fileout, "[%s]: Test '%s' Passed.\n", suite_name, test);
    fflush(fileout);
#endif
    tests_passed++;
}

int Print_Test_Results()
{
	fprintf(stdout, "For test suite '%s':\n", suite_name);
	fprintf(stdout,"Of %d attempted tests, %d passed, %d failed.\n", 
		tests_passed + tests_failed, tests_passed, tests_failed);
	if (tests_failed > 0) {
		int i;

		fprintf(stdout, "*** Tests Failed:\n");
		for (i = 0; i < tests_failed; i++)
			fprintf(stdout, "*** %s\n", failed_tests[i]);
	}
	fflush(stdout);
	return tests_failed;
}


int Summarize_Test_Results()
{
#ifdef VERBOSE
    fprintf(fileout, "For test suite '%s':\n", suite_name);
#else
    if (tests_failed > 0)
#endif
    {
	fprintf(fileout, "Of %d attempted tests, %d passed, %d failed.\n", 
		tests_passed + tests_failed, tests_passed, tests_failed);
    }
    if (tests_failed > 0) {
	int i;

	fprintf(fileout, "*** Tests Failed:\n");
	for (i = 0; i < tests_failed; i++)
	    fprintf(fileout, "*** %s\n", failed_tests[i]);
    }
	fflush(fileout);
    return tests_failed;
}

void Test_Finalize(void)
{
    if (TEST_ERRORS_WARN != MPI_ERRHANDLER_NULL) 
	MPI_Errhandler_free( &TEST_ERRORS_WARN );
    if (fileout) {
	  fflush(fileout);
	  fclose(fileout);
    }

}
void Print_Filecontent(int rank, int dodelete)
{
	char filename[512];
	int ch;
	FILE *fileout = NULL;

	sprintf(filename, "%s-%d.out", suite_name, rank);
	fileout = fopen(filename, "r");
	if (fileout){
	  do {
		ch = fgetc(fileout);
		if (ch>=0) /* avoid output of invalid characters */
			putchar(ch);
	  }
	  while ( feof(fileout) == 0 );
	  fclose(fileout);
	  if (dodelete)
		  /*DeleteFile(filename);*/
		  unlink(filename);
	}
}

void Delete_Outfile(int  rank)
{
	char filename[512];

	sprintf(filename, "%s-%d.out", suite_name, rank);
	unlink(filename);
	
}

#include "mpi.h"
/* Wait for every process to pass through this point.  This test is used
   to make sure that all processes complete, and that a test "passes" because
   it executed, not because it some process failed.  
 */
void Test_Waitforall( void )
{
    int m, one, myrank, n;

    MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
    MPI_Comm_size( MPI_COMM_WORLD, &n );
    one = 1;
    MPI_Allreduce( &one, &m, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD );

    if (m != n) {
	printf( "[%d] Expected %d processes to wait at end, got %d\n", myrank, 
		n, m );
    }
    if (myrank == 0) 
	printf( " No Errors\n" );
fflush(stdout);
}

/*
   Handler prints warning messsage and returns.  Internal.  Not
   a part of the standard.
 */
MPI_Errhandler TEST_ERRORS_WARN = MPI_ERRHANDLER_NULL;

#ifdef USE_STDARG
#ifdef __STDC__
void Test_Errors_warn(  MPI_Comm *comm, int *code, ... )
#else
void Test_Errors_warn(  MPI_Comm *comm, int *code, va_list va_alist )
#endif
{  
  char buf[MPI_MAX_ERROR_STRING];
  int  myid, result_len; 
  char *string;
#ifdef MPIR_DEBUG
  char *file;
  int  *line;
#endif
  static int in_handler = 0;
  va_list Argp;

#ifdef USE_OLDSTYLE_STDARG
  va_start( Argp );
#else
  va_start( Argp, code );
#endif
  string = va_arg(Argp,char *);
#ifdef MPIR_DEBUG
  /* These are only needed for debugging output */
  file   = va_arg(Argp,char *);
  line   = va_arg(Argp,int *);
#endif
  va_end( Argp );
#else
void Test_Errors_warn( MPI_Comm *comm, int *code, char *string, char *file, 
		       int *line )
{
  char buf[MPI_MAX_ERROR_STRING];
  int  myid, result_len; 
  static int in_handler = 0;
#endif

  if (in_handler) return;
  in_handler = 1;

  MPI_Comm_rank( MPI_COMM_WORLD, &myid );
  MPI_Error_string( *code, buf, &result_len );
#ifdef MPIR_DEBUG
  /* Generate this information ONLY when debugging MPIR */
  fprintf( stderr, "%d -  File: %s   Line: %d\n", myid, 
		   file, *line );
#endif
  fprintf( stderr, "%d - %s : %s\n", myid, 
          string ? string : "<NO ERROR MESSAGE>", buf );
  fflush( stderr );
  in_handler = 0;
}
