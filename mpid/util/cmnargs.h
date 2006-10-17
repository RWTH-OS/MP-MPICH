#ifndef CMNARGS
#define CMNARGS
#ifndef ANSI_ARGS
#if defined(__STDC__) || defined(__cplusplus)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif
#endif

void MPID_ArgSqueeze ANSI_ARGS(( int *, char ** ));
void MPID_ProcessArgs ANSI_ARGS((int *, char *** ));
int MPID_GetIntParameter ANSI_ARGS(( char *, int ));

#endif
