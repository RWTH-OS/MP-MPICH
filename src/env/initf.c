/*
 *  $Id$
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"
#include "mpimem.h"

#ifndef MPID_NO_FORTRAN
/* Skip over the entire file if Fortran isn't available */

#ifdef MPI_CRAY
/* Cray requires special code for sending strings to/from Fortran */
#include <fortran.h>
#endif


#ifdef WIN32
#ifdef ABSOFT
#define MPIPRIV _Cmpipriv
#elif defined(NAS_FORTRAN) || defined(LAHEY) || defined (G77)
#define MPIPRIV mpipriv_
#elif defined(SALFORD)
#define MPIPRIV MPIPRIV_
#endif

extern MPIPRIV;

#ifdef FORTRANCAPS
#define mpir_init_bottom_   MPIR_INIT_BOTTOM
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpir_init_bottom_   mpir_init_bottom__
#elif !defined(FORTRANUNDERSCORE)
#define mpir_init_bottom_   mpir_init_bottom
#endif

/* Prototypes for Fortran interface functions */
void FORTRAN_API mpir_init_bottom_ ( void* );
#endif

 

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_INIT = PMPI_INIT
EXPORT_MPI_API void MPI_INIT ( MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_init__ = pmpi_init__
EXPORT_MPI_API void mpi_init__ ( MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_init = pmpi_init
EXPORT_MPI_API void mpi_init ( MPI_Fint * );
#else
#pragma weak mpi_init_ = pmpi_init_
EXPORT_MPI_API void mpi_init_ ( MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API int MPI_INIT( MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_INIT")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API int mpi_init__( MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_init__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API int mpi_init( MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_init")));
#else
EXPORT_MPI_API int mpi_init_( MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_init_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_INIT  MPI_INIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_init__  mpi_init__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_init  mpi_init
#else
#pragma _HP_SECONDARY_DEF pmpi_init_  mpi_init_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_INIT as PMPI_INIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_init__ as pmpi_init__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_init as pmpi_init
#else
#pragma _CRI duplicate mpi_init_ as pmpi_init_
#endif

/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#include "mpiprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif

#ifdef FORTRANCAPS
#define mpi_init_ PMPI_INIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_init_ pmpi_init__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_init_ pmpi_init
#else
#define mpi_init_ pmpi_init_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_init_ MPI_INIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_init_ mpi_init__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_init_ mpi_init
#endif
#endif


/* Get the correct versions of the argument programs */
#ifdef FORTRANCAPS
#define mpir_iargc_ MPIR_IARGC
#define mpir_getarg_ MPIR_GETARG
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpir_iargc_ mpir_iargc__
#define mpir_getarg_ mpir_getarg__
#elif !defined(FORTRANUNDERSCORE)
#define mpir_iargc_ mpir_iargc
#define mpir_getarg_ mpir_getarg
#endif


/* #define DEBUG(a) {a}  */
#define DEBUG(a)
/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_init_ ( MPI_Fint * );
MPI_Fint mpir_iargc_ (void);

#ifdef MPI_CRAY
void mpir_getarg_ ( MPI_Fint *, _fcd );
#else
void FORTRAN_API mpir_getarg_ ( MPI_Fint *, char *, MPI_Fint );
#endif



EXPORT_MPI_API void FORTRAN_API mpi_init_( MPI_Fint *ierr )
{
#ifdef WIN32
    extern void mpir_init_bottom_win32(void*);
    mpir_init_bottom_win32(&MPIPRIV);
    *ierr = MPI_Init( 0, 0 );
#else
    int  Argc;
    MPI_Fint i, argsize = 1024;
    char **Argv, *p;
    int  ArgcSave;           /* Save the argument count */
    char **ArgvSave,         /* Save the pointer to the argument vector */
 	 **ArgvValSave;      /* Save the ENTRIES in the argument vector */
#ifdef MPI_CRAY
    _fcd tmparg;
#endif
/* Recover the args with the Fortran routines iargc_ and getarg_ */
    ArgcSave	= Argc = mpir_iargc_() + 1;
    ArgvSave	= Argv = (char **)MALLOC( Argc * sizeof(char *) );    
    ArgvValSave	= (char **)MALLOC( Argc * sizeof(char *) );
    if (!Argv || !ArgvValSave) {
	*ierr = MPIR_ERROR( (struct MPIR_COMMUNICATOR *)0, MPI_ERR_EXHAUSTED, 
			    "MPI_INIT" );
	return;
    }
    for (i=0; i<Argc; i++) {
	ArgvValSave[i] = Argv[i] = (char *)MALLOC( argsize + 1 );
	if (!Argv[i]) {
	    *ierr = MPIR_ERROR( (struct MPIR_COMMUNICATOR *)0, 
				MPI_ERR_EXHAUSTED, 
				"MPI_INIT" );
	    return;
        }
#ifdef MPI_CRAY
	tmparg = _cptofcd( Argv[i], argsize );
	mpir_getarg_( &i, tmparg );
#else
	mpir_getarg_( &i, Argv[i], argsize );
#endif
	DEBUG(MPID_trvalid( "after getarg" ); fflush(stderr););
	/* Trim trailing blanks */
	p = Argv[i] + argsize - 1;
	while (p >= Argv[i]) {
	    if (*p != ' ' && *p) {
		p[1] = '\0';
		break;
	    }
	    p--;
	}
    }

    DEBUG(for (i=0; i<ArgcSave; i++) {
	PRINTF( "[%d] argv[%d] = |%s|\n", MPIR_tid, i, ArgvSave[i] );
    });
    *ierr = MPI_Init( &Argc, &Argv );
    
    /* Recover space */
    DEBUG(PRINTF("[%d] About to recover space\n", MPIR_tid ););
    DEBUG(MPID_trdump(stdout););
    for (i=0; i<ArgcSave; i++) {
	DEBUG(PRINTF("[%d] About to recover ArgvSave[%d]=|%s|\n",\
		     MPIR_tid,i,ArgvSave[i] ););
	FREE( ArgvValSave[i] );
    }
    DEBUG(PRINTF("[%d] About to recover ArgvSave\n", MPIR_tid ););
    FREE( ArgvSave );
    FREE( ArgvValSave );
#endif
}

#endif
