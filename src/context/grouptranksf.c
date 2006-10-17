/* group_tranks.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_GROUP_TRANSLATE_RANKS = PMPI_GROUP_TRANSLATE_RANKS
EXPORT_MPI_API void MPI_GROUP_TRANSLATE_RANKS ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_group_translate_ranks__ = pmpi_group_translate_ranks__
EXPORT_MPI_API void mpi_group_translate_ranks__ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_group_translate_ranks = pmpi_group_translate_ranks
EXPORT_MPI_API void mpi_group_translate_ranks ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_group_translate_ranks_ = pmpi_group_translate_ranks_
EXPORT_MPI_API void mpi_group_translate_ranks_ ( MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_GROUP_TRANSLATE_RANKS ( MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_GROUP_TRANSLATE_RANKS")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_group_translate_ranks__ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_translate_ranks__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_group_translate_ranks ( MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_translate_ranks")));
#else
EXPORT_MPI_API void mpi_group_translate_ranks_ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint *, MPI_Fint *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_translate_ranks_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_GROUP_TRANSLATE_RANKS  MPI_GROUP_TRANSLATE_RANKS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_group_translate_ranks__  mpi_group_translate_ranks__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_group_translate_ranks  mpi_group_translate_ranks
#else
#pragma _HP_SECONDARY_DEF pmpi_group_translate_ranks_  mpi_group_translate_ranks_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_GROUP_TRANSLATE_RANKS as PMPI_GROUP_TRANSLATE_RANKS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_group_translate_ranks__ as pmpi_group_translate_ranks__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_group_translate_ranks as pmpi_group_translate_ranks
#else
#pragma _CRI duplicate mpi_group_translate_ranks_ as pmpi_group_translate_ranks_
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
#define mpi_group_translate_ranks_ PMPI_GROUP_TRANSLATE_RANKS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_translate_ranks_ pmpi_group_translate_ranks__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_translate_ranks_ pmpi_group_translate_ranks
#else
#define mpi_group_translate_ranks_ pmpi_group_translate_ranks_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_group_translate_ranks_ MPI_GROUP_TRANSLATE_RANKS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_translate_ranks_ mpi_group_translate_ranks__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_translate_ranks_ mpi_group_translate_ranks
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_group_translate_ranks_ ( MPI_Fint *, MPI_Fint *, 
                                            MPI_Fint *, MPI_Fint *, 
                                            MPI_Fint *, MPI_Fint * );

EXPORT_MPI_API void FORTRAN_API mpi_group_translate_ranks_ ( MPI_Fint *group_a, 
     MPI_Fint *n, MPI_Fint *ranks_a, MPI_Fint *group_b, MPI_Fint *ranks_b,
				  MPI_Fint *__ierr )
{

    if (sizeof(MPI_Fint) == sizeof(int))
        *__ierr = MPI_Group_translate_ranks(MPI_Group_f2c(*group_a),*n,
                                            ranks_a,
                                            MPI_Group_f2c(*group_b), 
                                            ranks_b);
    else {
        int *l_ranks_a;
        int *l_ranks_b;
        int i;

	MPIR_FALLOC(l_ranks_a,(int*)MALLOC(sizeof(int)* (int)*n),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Group_translate_ranks");

	MPIR_FALLOC(l_ranks_b,(int*)MALLOC(sizeof(int)* *(n)),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Group_translate_ranks");

        for (i=0; i<(int)*n; i++) {
            l_ranks_a[i] = (int)ranks_a[i];
    } 
        *__ierr = MPI_Group_translate_ranks(MPI_Group_f2c(*group_a),(int)*n,
                                            l_ranks_a,
                                            MPI_Group_f2c(*group_b), 
                                            l_ranks_b);
        for (i=0; i<(int)*n; i++) {
            ranks_b[i] = (MPI_Fint)(l_ranks_b[i]);
        }
	FREE( l_ranks_a );
	FREE( l_ranks_b );
    }
    
}
