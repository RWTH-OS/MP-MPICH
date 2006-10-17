/* group_rincl.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"

/* 
   Here we have a tricky situation.  In Fortran, the ranges will be
   an array integer ranges(3,*).  If there are n elements, then this is
   just 3*n integers in the order ranges(1,1),ranges(2,1),ranges(3,1),
   ranges(1,2),... .

   Now, the C binding is for int ranges[][3].  Now, note that the size
   of int [a][b] is a*b*sizeof(int) (NOT a * sizeof(int*)).  Also note
   that int foo[][b] is NOT a valid declaration EXCEPT for an actual 
   parameter to a routine.  What does all of this mean?  It means that
   while ranges[k] is a pointer to a type that consists of an int with
   3 components, it is not an arbitrary pointer; rather, it is computed from
   the layout of the data for an 2-d array in C.  Thus, all we need to do
   is pass the Fortran ranges straight through to C.

   Some compilers may complain about this; if you want to avoid the error
   message, you'll need to copy the "ranges" array into a temporary.
 */


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_GROUP_RANGE_INCL = PMPI_GROUP_RANGE_INCL
EXPORT_MPI_API void MPI_GROUP_RANGE_INCL ( MPI_Fint *, MPI_Fint *, MPI_Fint [][3], MPI_Fint *, MPI_Fint * );
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_group_range_incl__ = pmpi_group_range_incl__
EXPORT_MPI_API void mpi_group_range_incl__ ( MPI_Fint *, MPI_Fint *, MPI_Fint [][3], MPI_Fint *, MPI_Fint * );
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_group_range_incl = pmpi_group_range_incl
EXPORT_MPI_API void mpi_group_range_incl ( MPI_Fint *, MPI_Fint *, MPI_Fint [][3], MPI_Fint *, MPI_Fint * );
#else
#pragma weak mpi_group_range_incl_ = pmpi_group_range_incl_
EXPORT_MPI_API void mpi_group_range_incl_ ( MPI_Fint *, MPI_Fint *, MPI_Fint [][3], MPI_Fint *, MPI_Fint * );
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
EXPORT_MPI_API void MPI_GROUP_RANGE_INCL ( MPI_Fint *, MPI_Fint *,
	MPI_Fint [][3], MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_GROUP_RANGE_INCL")));
#elif defined(FORTRANDOUBLEUNDERSCORE)
EXPORT_MPI_API void mpi_group_range_incl__ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint [][3], MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_range_incl__")));
#elif !defined(FORTRANUNDERSCORE)
EXPORT_MPI_API void mpi_group_range_incl ( MPI_Fint *, MPI_Fint *,
	MPI_Fint [][3], MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_range_incl")));
#else
EXPORT_MPI_API void mpi_group_range_incl_ ( MPI_Fint *, MPI_Fint *,
	MPI_Fint [][3], MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_group_range_incl_")));
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_GROUP_RANGE_INCL  MPI_GROUP_RANGE_INCL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_group_range_incl__  mpi_group_range_incl__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_group_range_incl  mpi_group_range_incl
#else
#pragma _HP_SECONDARY_DEF pmpi_group_range_incl_  mpi_group_range_incl_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_GROUP_RANGE_INCL as PMPI_GROUP_RANGE_INCL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_group_range_incl__ as pmpi_group_range_incl__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_group_range_incl as pmpi_group_range_incl
#else
#pragma _CRI duplicate mpi_group_range_incl_ as pmpi_group_range_incl_
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
#define mpi_group_range_incl_ PMPI_GROUP_RANGE_INCL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_range_incl_ pmpi_group_range_incl__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_range_incl_ pmpi_group_range_incl
#else
#define mpi_group_range_incl_ pmpi_group_range_incl_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_group_range_incl_ MPI_GROUP_RANGE_INCL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_range_incl_ mpi_group_range_incl__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_range_incl_ mpi_group_range_incl
#endif
#endif


/* Prototype to suppress warnings about missing prototypes */
EXPORT_MPI_API void FORTRAN_API mpi_group_range_incl_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, 
                                       MPI_Fint [][3], MPI_Fint *, 
                                       MPI_Fint * ));

EXPORT_MPI_API void FORTRAN_API mpi_group_range_incl_ ( MPI_Fint *group, MPI_Fint *n, MPI_Fint ranges[][3], MPI_Fint *newgroup, MPI_Fint *__ierr )
{
    MPI_Group l_newgroup;
 
    if (sizeof(MPI_Fint) == sizeof(int))
        *__ierr = MPI_Group_range_incl(MPI_Group_f2c(*group), *n,
                                       ranges, &l_newgroup);
    else {
	int *l_ranges;
	int i;
	int j = 0;

        MPIR_FALLOC(l_ranges,(int*)MALLOC(sizeof(int)* ((int)*n * 3)),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Group_range_excl");

        for (i=0; i<*n; i++) {
	    l_ranges[j++] = (int)ranges[i][0];
	    l_ranges[j++] = (int)ranges[i][1];
	    l_ranges[j++] = (int)ranges[i][2];
	}
	
        *__ierr = MPI_Group_range_incl(MPI_Group_f2c(*group), (int)*n,
                                       (int (*)[3])l_ranges,
                                        &l_newgroup);
	FREE( l_ranges );
    }
    *newgroup = MPI_Group_c2f(l_newgroup);
}
