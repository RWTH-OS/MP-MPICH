/* errcreate.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"


#if defined(MPI_BUILD_PROFILING) || defined(HAVE_WEAK_SYMBOLS)

#if defined(HAVE_WEAK_SYMBOLS)
#if defined(HAVE_PRAGMA_WEAK)
#if defined(FORTRANCAPS)
#pragma weak MPI_ERRHANDLER_CREATE = PMPI_ERRHANDLER_CREATE
#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
EXPORT_MPI_API void MPI_ERR_HANDLER_CREATE ( MPI_Handler_function **, MPI_Fint *, MPI_Fint * );
#else
EXPORT_MPI_API void MPI_ERRHANDLER_CREATE ( MPI_Handler_function *, MPI_Fint *, MPI_Fint * );
#endif
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma weak mpi_errhandler_create__ = pmpi_errhandler_create__
#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
EXPORT_MPI_API void mpi_errhandler_create__ ( MPI_Handler_function **, MPI_Fint *, MPI_Fint * );
#else
EXPORT_MPI_API void mpi_errhandler_create__ ( MPI_Handler_function *, MPI_Fint *, MPI_Fint * );
#endif
#elif !defined(FORTRANUNDERSCORE)
#pragma weak mpi_errhandler_create = pmpi_errhandler_create
#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
EXPORT_MPI_API void mpi_errhandler_create ( MPI_Handler_function **, MPI_Fint *, MPI_Fint * );
#else
EXPORT_MPI_API void mpi_errhandler_create ( MPI_Handler_function *, MPI_Fint *, MPI_Fint * );
#endif
#else
#pragma weak mpi_errhandler_create_ = pmpi_errhandler_create_
#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
EXPORT_MPI_API void mpi_errhandler_create_ ( MPI_Handler_function **, MPI_Fint *, MPI_Fint * );
#else
EXPORT_MPI_API void mpi_errhandler_create_ ( MPI_Handler_function *, MPI_Fint *, MPI_Fint * );
#endif
#endif

#elif defined(HAVE_ATTRIBUTE_WEAK)
#if defined(FORTRANCAPS)
#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
EXPORT_MPI_API void MPI_ERR_HANDLER_CREATE ( MPI_Handler_function **, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_ERR_HANDLER_CREATE")));
#else
EXPORT_MPI_API void MPI_ERRHANDLER_CREATE ( MPI_Handler_function *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("PMPI_ERRHANDLER_CREATE")));
#endif
#elif defined(FORTRANDOUBLEUNDERSCORE)
#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
EXPORT_MPI_API void mpi_errhandler_create__ ( MPI_Handler_function **, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_errhandler_create__")));
#else
EXPORT_MPI_API void mpi_errhandler_create__ ( MPI_Handler_function *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_errhandler_create__")));
#endif
#elif !defined(FORTRANUNDERSCORE)
#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
EXPORT_MPI_API void mpi_errhandler_create ( MPI_Handler_function **, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_errhandler_create")));
#else
EXPORT_MPI_API void mpi_errhandler_create ( MPI_Handler_function *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_errhandler_create")));
#endif
#else
#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
EXPORT_MPI_API void mpi_errhandler_create_ ( MPI_Handler_function **, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_errhandler_create_")));
#else
EXPORT_MPI_API void mpi_errhandler_create_ ( MPI_Handler_function *, MPI_Fint *,
	MPI_Fint * ) __attribute__ ((weak, alias ("pmpi_errhandler_create_")));
#endif
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(FORTRANCAPS)
#pragma _HP_SECONDARY_DEF PMPI_ERRHANDLER_CREATE  MPI_ERRHANDLER_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_errhandler_create__  mpi_errhandler_create__
#elif !defined(FORTRANUNDERSCORE)
#pragma _HP_SECONDARY_DEF pmpi_errhandler_create  mpi_errhandler_create
#else
#pragma _HP_SECONDARY_DEF pmpi_errhandler_create_  mpi_errhandler_create_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(FORTRANCAPS)
#pragma _CRI duplicate MPI_ERRHANDLER_CREATE as PMPI_ERRHANDLER_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#pragma _CRI duplicate mpi_errhandler_create__ as pmpi_errhandler_create__
#elif !defined(FORTRANUNDERSCORE)
#pragma _CRI duplicate mpi_errhandler_create as pmpi_errhandler_create
#else
#pragma _CRI duplicate mpi_errhandler_create_ as pmpi_errhandler_create_
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
#define mpi_errhandler_create_ PMPI_ERRHANDLER_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_errhandler_create_ pmpi_errhandler_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_errhandler_create_ pmpi_errhandler_create
#else
#define mpi_errhandler_create_ pmpi_errhandler_create_
#endif

#else

#ifdef FORTRANCAPS
#define mpi_errhandler_create_ MPI_ERRHANDLER_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_errhandler_create_ mpi_errhandler_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_errhandler_create_ mpi_errhandler_create
#endif
#endif


#ifdef _WIN32
/* We have to take care of the different calling conventions
   on Windows platforms. So in order to support STDCALL compilers
   we create a wrapper that is called by MPI and in turn calls the 
   real handler.
*/
#ifdef VISUAL_FORTRAN
typedef void (FORTRAN_API *fortran_handler)(MPI_Fint*,MPI_Fint*,char*,int err_len,char*,int file_len,MPI_Fint*);
#else
typedef void (*fortran_handler)(MPI_Fint*,MPI_Fint*,char*,char*,MPI_Fint*,int err_len,int file_len);
#endif
void fortran_dummy_handler(comm, code, string, file, line )
MPI_Comm *comm;
int      *code, *line;
char     *string, *file;
{
    struct MPIR_COMMUNICATOR *comm_ptr;
    struct MPIR_Errhandler *errhand;
    comm_ptr = MPIR_GET_COMM_PTR(*comm);
    errhand = MPIR_ToPointer( comm_ptr->error_handler);
    if(!string) string="<NO ERROR MESSAGE>";
#ifdef VISUAL_FORTRAN
    ((fortran_handler)errhand->fortran_routine)(comm,code,string,strlen(string),file,strlen(file),line);
#else
    ((fortran_handler)errhand->fortran_routine)(comm,code,string,file,line,strlen(string),strlen(file));
#endif

}
#endif


/* Prototype to suppress warnings about missing prototypes */
#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
EXPORT_MPI_API void FORTRAN_API mpi_errhandler_create_ ANSI_ARGS(( MPI_Handler_function **, 
					MPI_Fint *, MPI_Fint * ));
#else
EXPORT_MPI_API void FORTRAN_API mpi_errhandler_create_ ANSI_ARGS(( MPI_Handler_function *, 
					MPI_Fint *, MPI_Fint * ));
#endif

EXPORT_MPI_API void FORTRAN_API mpi_errhandler_create_(
#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
	MPI_Handler_function **function,
#else
	MPI_Handler_function *function,
#endif
	MPI_Fint *errhandler, MPI_Fint *__ierr)
{

    MPI_Errhandler l_errhandler;

#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
    *__ierr = MPI_Errhandler_create( *function, &l_errhandler );
#else
    *__ierr = MPI_Errhandler_create( function, &l_errhandler );
#endif
#ifdef _WIN32
    {
	    struct MPIR_Errhandler *errhand;
	    errhand = MPIR_ToPointer(l_errhandler);
	    errhand->fortran_routine = errhand->routine;
	    errhand->routine = fortran_dummy_handler;
    }
#endif

    *errhandler = MPI_Errhandler_c2f(l_errhandler);
}
