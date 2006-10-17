/* mpe_counter.c */
/* Fortran interface file for sun4 */
#ifndef DEBUG_ALL
#define DEBUG_ALL
#endif
#include <stdio.h>
#include "mpeconf.h"
#include "mpe.h"


#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpe_counter_create_ PMPE_COUNTER_CREATE
#define mpe_counter_free_ PMPE_COUNTER_FREE
#define mpe_counter_nxtval_ PMPE_COUNTER_NXTVAL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpe_counter_create_ pmpe_counter_create__
#define mpe_counter_free_ pmpe_counter_free__
#define mpe_counter_nxtval_ pmpe_counter_nxtval__
#elif !defined(FORTRANUNDERSCORE)
#define mpe_counter_create_ pmpe_counter_create
#define mpe_counter_free_ pmpe_counter_free
#define mpe_counter_nxtval_ pmpe_counter_nxtval
#else
#define mpe_counter_create_ pmpe_counter_create_
#define mpe_counter_free_ pmpe_counter_free_
#define mpe_counter_nxtval_ pmpe_counter_nxtval_
#endif
#else
#ifdef FORTRANCAPS
#define mpe_counter_create_ MPE_COUNTER_CREATE
#define mpe_counter_free_ MPE_COUNTER_FREE
#define mpe_counter_nxtval_ MPE_COUNTER_NXTVAL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpe_counter_create_ mpe_counter_create__
#define mpe_counter_free_ mpe_counter_free__
#define mpe_counter_nxtval_ mpe_counter_nxtval__
#elif !defined(FORTRANUNDERSCORE)
#define mpe_counter_create_ mpe_counter_create
#define mpe_counter_free_ mpe_counter_free
#define mpe_counter_nxtval_ mpe_counter_nxtval
#endif
#endif

 int  FORTRAN_API mpe_counter_create_( oldcomm, smaller_comm, counter_comm)
MPI_Comm  *oldcomm,  *smaller_comm,  *counter_comm;
{
return MPE_Counter_create(*oldcomm,
	(MPI_Comm* )*((int*)smaller_comm),
	(MPI_Comm* )*((int*)counter_comm));
}
 int  FORTRAN_API mpe_counter_free_(counter_comm, smaller_comm)
MPI_Comm *counter_comm;
MPI_Comm *smaller_comm;
{
return MPE_Counter_free(*counter_comm,*smaller_comm);
}
 int  FORTRAN_API mpe_counter_nxtval_(counter_comm, value)
MPI_Comm *counter_comm;
int *value;
{
return MPE_Counter_nxtval(*counter_comm,value);
}
