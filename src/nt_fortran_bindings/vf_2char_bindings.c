#ifdef VISUAL_FORTRAN

/* This file contains the MPI bindings that might take a 
   string as argument. Since VF uses STDCALL namings and
   callee clears stack and additionaly inlines sting length parameters
   we can't just use the normal bindings and ignore the extra parameter
*/


#include "mpiimpl.h"

#if defined(MPI_BUILD_PROFILING)
/* Include mapping from MPI->PMPI */
#include "mpiprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"

#define mpi_allgather_ PMPI_ALLGATHER
#define mpi_allgatherv_ PMPI_ALLGATHERV
#define mpi_allreduce_ PMPI_ALLREDUCE
#define mpi_alltoall_ PMPI_ALLTOALL
#define mpi_alltoallv_ PMPI_ALLTOALLV
#define mpi_gather_ PMPI_GATHER
#define mpi_gatherv_ PMPI_GATHERV
#define mpi_pack_ PMPI_PACK
#define mpi_reduce_scatter_ PMPI_REDUCE_SCATTER
#define mpi_reduce_ PMPI_REDUCE
#define mpi_scan_ PMPI_SCAN
#define mpi_scatter_ PMPI_SCATTER
#define mpi_scatterv_ PMPI_SCATTERV
#define mpi_sendrecv_ PMPI_SENDRECV
#define mpi_unpack_ PMPI_UNPACK
#else
#define mpi_allgather_ MPI_ALLGATHER
#define mpi_allgatherv_ MPI_ALLGATHERV
#define mpi_allreduce_ MPI_ALLREDUCE
#define mpi_alltoall_ MPI_ALLTOALL
#define mpi_alltoallv_ MPI_ALLTOALLV
#define mpi_gather_ MPI_GATHER
#define mpi_gatherv_ MPI_GATHERV
#define mpi_pack_ MPI_PACK
#define mpi_reduce_scatter_ MPI_REDUCE_SCATTER
#define mpi_reduce_ MPI_REDUCE
#define mpi_scan_ MPI_SCAN
#define mpi_scatter_ MPI_SCATTER
#define mpi_scatterv_ MPI_SCATTERV
#define mpi_sendrecv_ MPI_SENDRECV
#define mpi_unpack_ MPI_UNPACK
#endif

void __stdcall mpi_allgather_ ( void *sendbuf,int len1, MPI_Fint *sendcount, MPI_Fint *sendtype,
                    void *recvbuf, int len2, MPI_Fint *recvcount, MPI_Fint *recvtype, 
		    MPI_Fint *comm, MPI_Fint *__ierr )
{
    *__ierr = MPI_Allgather(MPIR_F_PTR(sendbuf), (int)*sendcount,
                            MPI_Type_f2c(*sendtype),
			    MPIR_F_PTR(recvbuf),
                            (int)*recvcount,
                            MPI_Type_f2c(*recvtype),
                            MPI_Comm_f2c(*comm));
}

void __stdcall mpi_allgatherv_ ( void *sendbuf,int len1, MPI_Fint *sendcount,  MPI_Fint *sendtype, 
		       void *recvbuf,int len2, MPI_Fint *recvcounts, MPI_Fint *displs, 
		       MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *__ierr )
{

        *__ierr = MPI_Allgatherv(MPIR_F_PTR(sendbuf), *sendcount,
                                 MPI_Type_f2c(*sendtype),
			         MPIR_F_PTR(recvbuf), recvcounts,
                                 displs, MPI_Type_f2c(*recvtype),
			         MPI_Comm_f2c(*comm));
}

void __stdcall mpi_allreduce_ ( void *sendbuf,int len1, void *recvbuf,int len2, 
		     MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, 
		     MPI_Fint *comm, MPI_Fint *__ierr )
{
    *__ierr = MPI_Allreduce(MPIR_F_PTR(sendbuf),MPIR_F_PTR(recvbuf),
                            (int)*count, MPI_Type_f2c(*datatype),
                            MPI_Op_f2c(*op), MPI_Comm_f2c(*comm) );
}

void __stdcall mpi_alltoall_( void *sendbuf,int len1, MPI_Fint *sendcount, MPI_Fint *sendtype, 
                    void *recvbuf, int len2, MPI_Fint *recvcnt, MPI_Fint *recvtype, 
		    MPI_Fint *comm, MPI_Fint *__ierr )
{
    *__ierr = MPI_Alltoall(MPIR_F_PTR(sendbuf), (int)*sendcount,
                           MPI_Type_f2c(*sendtype), MPIR_F_PTR(recvbuf),
                           (int)*recvcnt, MPI_Type_f2c(*recvtype),
                           MPI_Comm_f2c(*comm) );
}

void __stdcall mpi_alltoallv_ ( void *sendbuf,int len1, MPI_Fint *sendcnts, MPI_Fint *sdispls, MPI_Fint *sendtype, 
                    void *recvbuf, int len2, MPI_Fint *recvcnts, MPI_Fint *rdispls, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *__ierr )
{
    
    
	*__ierr = MPI_Alltoallv(MPIR_F_PTR(sendbuf), sendcnts, 
                                sdispls, MPI_Type_f2c(*sendtype),
			        MPIR_F_PTR(recvbuf), recvcnts, 
                                rdispls, MPI_Type_f2c(*recvtype),
			        MPI_Comm_f2c(*comm) );
}

void __stdcall mpi_gather_ ( void *sendbuf, int len1, MPI_Fint *sendcnt, MPI_Fint *sendtype, 
		   void *recvbuf, int len2, MPI_Fint *recvcount, MPI_Fint *recvtype, 
		   MPI_Fint *root, MPI_Fint *comm, MPI_Fint *__ierr )
{
    *__ierr = MPI_Gather(MPIR_F_PTR(sendbuf), (int)*sendcnt,
                         MPI_Type_f2c(*sendtype), MPIR_F_PTR(recvbuf),
                         (int)*recvcount, MPI_Type_f2c(*recvtype),
                         (int)*root, MPI_Comm_f2c(*comm));
}


void __stdcall mpi_gatherv_ ( void *sendbuf, int len1, MPI_Fint *sendcnt,  MPI_Fint *sendtype, 
                  void *recvbuf, int len2, MPI_Fint *recvcnts, MPI_Fint *displs, MPI_Fint *recvtype, 
                  MPI_Fint *root, MPI_Fint *comm, MPI_Fint *__ierr )
{

 
        *__ierr = MPI_Gatherv(MPIR_F_PTR(sendbuf), *sendcnt,
                              MPI_Type_f2c(*sendtype), MPIR_F_PTR(recvbuf),
                              recvcnts, displs, 
                              MPI_Type_f2c(*recvtype), *root,
                              MPI_Comm_f2c(*comm));
}

void __stdcall mpi_pack_ ( void *inbuf,int len1, MPI_Fint *incount, MPI_Fint *type, 
		 void *outbuf,int len2, MPI_Fint *outcount, MPI_Fint *position, 
		 MPI_Fint *comm, MPI_Fint *__ierr )
{
    int lposition;

    lposition = (int)*position;
    *__ierr = MPI_Pack(MPIR_F_PTR(inbuf), (int)*incount, MPI_Type_f2c(*type),
		       outbuf, (int)*outcount, &lposition,
                       MPI_Comm_f2c(*comm));
    *position = (MPI_Fint)lposition;
}

void __stdcall mpi_reduce_scatter_ ( void *sendbuf,int len1, void *recvbuf,int len2, MPI_Fint *recvcnts, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, 
			   MPI_Fint *__ierr )
{

        *__ierr = MPI_Reduce_scatter(MPIR_F_PTR(sendbuf),
				     MPIR_F_PTR(recvbuf), recvcnts,
                                     MPI_Type_f2c(*datatype), MPI_Op_f2c(*op),
                                     MPI_Comm_f2c(*comm));
}

void __stdcall mpi_reduce_ ( void *sendbuf,int len1, void *recvbuf,int len2, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *__ierr )
{
    *__ierr = MPI_Reduce(MPIR_F_PTR(sendbuf), MPIR_F_PTR(recvbuf), 
                         (int)*count, MPI_Type_f2c(*datatype), 
                         MPI_Op_f2c(*op), (int)*root, 
                         MPI_Comm_f2c(*comm));
}

void __stdcall mpi_scan_ ( void *sendbuf,int len1, void *recvbuf,int len2, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *__ierr )
{
    *__ierr = MPI_Scan(MPIR_F_PTR(sendbuf), MPIR_F_PTR(recvbuf),
                       (int)*count, MPI_Type_f2c(*datatype),
                       MPI_Op_f2c(*op), MPI_Comm_f2c(*comm));
}

void __stdcall mpi_scatter_ ( void *sendbuf,int len1, MPI_Fint *sendcnt, MPI_Fint *sendtype, 
                    void *recvbuf,int len2, MPI_Fint *recvcnt, MPI_Fint *recvtype, 
                    MPI_Fint *root, MPI_Fint *comm, MPI_Fint *__ierr )
{
    *__ierr = MPI_Scatter(MPIR_F_PTR(sendbuf), (int)*sendcnt,
                          MPI_Type_f2c(*sendtype), MPIR_F_PTR(recvbuf),
                          (int)*recvcnt, MPI_Type_f2c(*recvtype), 
                          (int)*root, MPI_Comm_f2c(*comm));
}

void __stdcall mpi_scatterv_ ( void *sendbuf,int len1, MPI_Fint *sendcnts, MPI_Fint *displs, MPI_Fint *sendtype, 
                   void *recvbuf,int len2, MPI_Fint *recvcnt,  MPI_Fint *recvtype, 
                   MPI_Fint *root, MPI_Fint *comm, MPI_Fint *__ierr )
{
    
        *__ierr = MPI_Scatterv(MPIR_F_PTR(sendbuf), sendcnts, displs,
                           MPI_Type_f2c(*sendtype), MPIR_F_PTR(recvbuf),
                           *recvcnt, MPI_Type_f2c(*recvtype),
                           *root, MPI_Comm_f2c(*comm) );
}

void __stdcall mpi_sendrecv_( void *sendbuf,int len1, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *dest, MPI_Fint *sendtag, 
                    void *recvbuf,int len2, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *source, MPI_Fint *recvtag, 
                    MPI_Fint *comm, MPI_Fint *status, MPI_Fint *__ierr )
{
    MPI_Status c_status;

    *__ierr = MPI_Sendrecv(MPIR_F_PTR(sendbuf), (int)*sendcount, 
                           MPI_Type_f2c(*sendtype), (int)*dest, 
                           (int)*sendtag, MPIR_F_PTR(recvbuf), 
                           (int)*recvcount, MPI_Type_f2c(*recvtype),
			   (int)*source, (int)*recvtag,
                           MPI_Comm_f2c(*comm), &c_status);
    MPI_Status_c2f(&c_status, status);
}

void __stdcall mpi_unpack_ ( void *inbuf,int len1, MPI_Fint *insize, MPI_Fint *position, void *outbuf,int len2, MPI_Fint *outcount, MPI_Fint *type, MPI_Fint *comm, 
		   MPI_Fint *__ierr )
{
    int l_position;
    l_position = (int)*position;

    *__ierr = MPI_Unpack(inbuf, (int)*insize, &l_position,
                         MPIR_F_PTR(outbuf), (int)*outcount,
			 MPI_Type_f2c(*type), MPI_Comm_f2c(*comm) );
    *position = (MPI_Fint)l_position;
}

#endif