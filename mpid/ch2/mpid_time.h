/* $Id$ */
#ifndef _MPID_TIME_H
#define _MPID_TIME_H

/* returns wall clock time */
#ifndef MPID_Wtime
#define MPID_Wtime(t) MPID_CH_Wtime(t)
extern void MPID_Wtime (double *);
#endif

/* returns resolution of wall clock time */
#ifndef MPID_Wtick
#define MPID_Wtick MPID_CH_Wtick
void MPID_CH_Wtick ( double * );
#endif

#endif
