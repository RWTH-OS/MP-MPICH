#ifndef MPID_Wtime

#ifndef ANSI_ARGS
#if defined(__STDC__) || defined(__cplusplus)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif
#endif

#define MPID_Wtime(a) MPID_CH_Wtime(a)

#define MPID_Wtick MPID_CH_Wtick

void MPID_CH_Wtick ANSI_ARGS(( double * ));
void MPID_CH_Wtime ANSI_ARGS((double*));
#endif
