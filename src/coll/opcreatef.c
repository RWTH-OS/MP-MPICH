/* opcreate.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpifort.h"
#ifdef WIN32_FORTRAN_STDCALL
#include "mpiops.h"
#endif

#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_OP_CREATE = PMPI_OP_CREATE
#ifdef  FORTRAN_SPECIAL_FUNCTION_PTR
EXPORT_MPI_API void MPI_OP_CREATE ( MPI_User_function **, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
EXPORT_MPI_API void MPI_OP_CREATE ( MPI_User_function *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_op_create__ = pmpi_op_create__
#ifdef  FORTRAN_SPECIAL_FUNCTION_PTR
EXPORT_MPI_API void mpi_op_create__ ( MPI_User_function **, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
EXPORT_MPI_API void mpi_op_create__ ( MPI_User_function *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_op_create = pmpi_op_create
#ifdef  FORTRAN_SPECIAL_FUNCTION_PTR
EXPORT_MPI_API void mpi_op_create ( MPI_User_function **, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
EXPORT_MPI_API void mpi_op_create ( MPI_User_function *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif
#else
#pragma weak mpi_op_create_ = pmpi_op_create_
#ifdef  FORTRAN_SPECIAL_FUNCTION_PTR
EXPORT_MPI_API void mpi_op_create_ ( MPI_User_function **, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#else
EXPORT_MPI_API void mpi_op_create_ ( MPI_User_function *, MPI_Fint *, MPI_Fint *, MPI_Fint * );
#endif
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
#ifdef  FORTRAN_SPECIAL_FUNCTION_PTR
EXPORT_MPI_API void MPI_OP_CREATE ( MPI_User_function **, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_OP_CREATE")));
#else
EXPORT_MPI_API void MPI_OP_CREATE ( MPI_User_function *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_OP_CREATE")));
#endif
#elif defined(FORTRANDOUBLEUNDERSCORE)
#ifdef  FORTRAN_SPECIAL_FUNCTION_PTR
EXPORT_MPI_API void mpi_op_create__ ( MPI_User_function **, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_op_create__")));
#else
EXPORT_MPI_API void mpi_op_create__ ( MPI_User_function *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_op_create__")));
#endif
#elif !defined(FORTRANUNDERSCORE)
#ifdef  FORTRAN_SPECIAL_FUNCTION_PTR
EXPORT_MPI_API void mpi_op_create ( MPI_User_function **, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_op_create")));
#else
EXPORT_MPI_API void mpi_op_create ( MPI_User_function *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_op_create")));
#endif
#else
#ifdef  FORTRAN_SPECIAL_FUNCTION_PTR
EXPORT_MPI_API void mpi_op_create_ ( MPI_User_function **, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_op_create_")));
#else
EXPORT_MPI_API void mpi_op_create_ ( MPI_User_function *, MPI_Fint *,
	MPI_Fint *, MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_op_create_")));
#endif
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_OP_CREATE  MPI_OP_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_op_create__  mpi_op_create__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_op_create  mpi_op_create
#else
#pragma _HP_SECONDARY_DEF pmpi_op_create_  mpi_op_create_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_OP_CREATE as PMPI_OP_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_op_create__ as pmpi_op_create__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_op_create as pmpi_op_create
#else
#pragma _CRI duplicate mpi_op_create_ as pmpi_op_create_
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
#define mpi_op_create_ PMPI_OP_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_op_create_ pmpi_op_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_op_create_ pmpi_op_create
#else
#define mpi_op_create_ pmpi_op_create_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_op_create_ MPI_OP_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_op_create_ mpi_op_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_op_create_ mpi_op_create
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
#ifdef  FORTRAN_SPECIAL_FUNCTION_PTR
EXPORT_MPI_API void FORTRAN_API mpi_op_create_ ANSI_ARGS(( MPI_User_function **, MPI_Fint *, MPI_Fint *, 
				MPI_Fint * ));
#else
EXPORT_MPI_API void FORTRAN_API mpi_op_create_ ANSI_ARGS(( MPI_User_function *, MPI_Fint *, MPI_Fint *,
                                MPI_Fint * ));
#endif

EXPORT_MPI_API void FORTRAN_API mpi_op_create_(
#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
	MPI_User_function **function,
#else
	MPI_User_function *function,
#endif
	MPI_Fint *commute, MPI_Fint *op, MPI_Fint *__ierr)
{

    MPI_Op l_op;

#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
    *__ierr = MPI_Op_create(*function,MPIR_FROM_FLOG((int)*commute),
                            &l_op);
#elif defined(_TWO_WORD_FCD)
    int tmp = *commute;
    *__ierr = MPI_Op_create(*function,MPIR_FROM_FLOG(tmp),&l_op);

#else
    *__ierr = MPI_Op_create(function,MPIR_FROM_FLOG((int)*commute),
                            &l_op);
#endif
#ifdef WIN32_FORTRAN_STDCALL
    {
    struct MPIR_OP *new;
    new = (struct MPIR_OP *)MPIR_ToPointer( l_op );
    new->stdcall = 1;
    }
#endif
    *op = MPI_Op_c2f(l_op);
}
