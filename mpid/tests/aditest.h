#ifndef ADITEST
#define ADITEST

#ifndef ANSI_ARGS
#if defined(__STDC__) || defined(__cplusplus)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif
#endif

int CheckData ANSI_ARGS(( char *, char *, int ));
int CheckDataS ANSI_ARGS(( short *, short *, int, char * ));
int CheckStatus ANSI_ARGS(( MPI_Status *, int, int, int ));
void SetupArgs ANSI_ARGS(( int, char **, int *, int *, int * ));
void SetupTests ANSI_ARGS(( int, char **, int *, int *, int *, char **, 
			    char ** ));
void SetupTestsS ANSI_ARGS(( int, char **, int *, int *, int *, short **, 
			     short ** ));
void EndTests ANSI_ARGS(( void *, void * ));
#endif
