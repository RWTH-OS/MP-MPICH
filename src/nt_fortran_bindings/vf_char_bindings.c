
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

#define mpi_address_ __stdcall PMPI_ADDRESS
#define mpi_allgather_ __stdcall PMPI_ALLGATHER
#define mpi_allgatherv_ __stdcall PMPI_ALLGATHERV
#define mpi_alltoall_ __stdcall PMPI_ALLTOALL
#define mpi_alltoallv_ __stdcall PMPI_ALLTOALLV
#define mpi_bcast_ __stdcall PMPI_BCAST
#define mpi_bsend_init_ __stdcall PMPI_BSEND_INIT
#define mpi_bsend_ __stdcall PMPI_BSEND
#define mpi_buffer_attach_ __stdcall PMPI_BUFFER_ATTACH
#define mpi_buffer_detach_ __stdcall PMPI_BUFFER_DETACH
#define mpi_recv_init_ __stdcall PMPI_RECV_INIT
#define mpi_send_init_ __stdcall PMPI_SEND_INIT
#define mpi_gather_ __stdcall PMPI_GATHER
#define mpi_gatherv_ __stdcall PMPI_GATHERV
#define mpi_ibsend_ __stdcall PMPI_IBSEND
#define mpi_irecv_ __stdcall PMPI_IRECV
#define mpi_irsend_ __stdcall PMPI_IRSEND
#define mpi_isend_ __stdcall PMPI_ISEND
#define mpi_issend_ __stdcall PMPI_ISSEND
#define mpi_recv_ __stdcall PMPI_RECV
#define mpi_rsend_init_ __stdcall PMPI_RSEND_INIT
#define mpi_rsend_ __stdcall PMPI_RSEND
#define mpi_scatter_ __stdcall PMPI_SCATTER
#define mpi_scatterv_ __stdcall PMPI_SCATTERV
#define mpi_send_ __stdcall PMPI_SEND
#define mpi_sendrecv_ __stdcall PMPI_SENDRECV
#define mpi_sendrecv_replace_ __stdcall PMPI_SENDRECV_REPLACE
#define mpi_ssend_init_ __stdcall PMPI_SSEND_INIT
#define mpi_ssend_ __stdcall PMPI_SSEND
#else
#define mpi_address_ __stdcall MPI_ADDRESS
#define mpi_allgather_ __stdcall MPI_ALLGATHER
#define mpi_allgatherv_ __stdcall MPI_ALLGATHERV
#define mpi_alltoall_ __stdcall MPI_ALLTOALL
#define mpi_alltoallv_ __stdcall MPI_ALLTOALLV
#define mpi_bcast_ __stdcall MPI_BCAST
#define mpi_bsend_init_ __stdcall MPI_BSEND_INIT
#define mpi_bsend_ __stdcall MPI_BSEND
#define mpi_buffer_attach_ __stdcall MPI_BUFFER_ATTACH
#define mpi_buffer_detach_ __stdcall MPI_BUFFER_DETACH
#define mpi_recv_init_ __stdcall MPI_RECV_INIT
#define mpi_send_init_ __stdcall MPI_SEND_INIT
#define mpi_gather_ __stdcall MPI_GATHER
#define mpi_gatherv_ __stdcall MPI_GATHERV
#define mpi_ibsend_ __stdcall MPI_IBSEND
#define mpi_irecv_ __stdcall MPI_IRECV
#define mpi_irsend_ __stdcall MPI_IRSEND
#define mpi_isend_ __stdcall MPI_ISEND
#define mpi_issend_ __stdcall MPI_ISSEND
#define mpi_recv_ __stdcall MPI_RECV
#define mpi_rsend_init_ __stdcall MPI_RSEND_INIT
#define mpi_rsend_ __stdcall MPI_RSEND
#define mpi_scatter_ __stdcall MPI_SCATTER
#define mpi_scatterv_ __stdcall MPI_SCATTERV
#define mpi_send_ __stdcall MPI_SEND
#define mpi_sendrecv_ __stdcall MPI_SENDRECV
#define mpi_sendrecv_replace_ __stdcall MPI_SENDRECV_REPLACE
#define mpi_ssend_init_ __stdcall MPI_SSEND_INIT
#define mpi_ssend_ __stdcall MPI_SSEND
#endif

#define PAGE_READ_PRIV (PAGE_READONLY|PAGE_READWRITE|PAGE_WRITECOPY|\
PAGE_EXECUTE_READ|PAGE_EXECUTE_READWRITE|PAGE_EXECUTE_WRITECOPY)     

#define PAGE_WRITE_PRIV (PAGE_READWRITE|PAGE_WRITECOPY|\
PAGE_EXECUTE_READWRITE|PAGE_EXECUTE_WRITECOPY)     

#undef VF_TYPE_ERROR
#define VF_TYPE_ERROR(func,arg,val) *__ierr=MPIR_ERROR(MPIR_COMM_WORLD,\
MPIR_Err_setmsg(MPI_ERR_ARG,MPIR_ERR_ARG_NAMED,func,"Invalid type","Arg %s invalid value %d",arg,val),\
func)


#define VF_TYPE_ERROR1(func) VF_TYPE_ERROR(func,"sendtype",MPI_Type_f2c(*sendtype))
#define VF_TYPE_ERROR2(func) VF_TYPE_ERROR(func,"recvtype",MPI_Type_f2c(*recvtype))

#define BUF_TXT "Can't determine which of sendbuf/recvbuf is a character sting"
#define VF_BUF_ERROR(func) *__ierr=MPIR_ERROR(MPIR_COMM_WORLD,MPIR_Err_setmsg(MPI_ERR_ARG,0,func,0,BUF_TXT),func)

WINBASEAPI
DWORD
WINAPI
MPIR_VirtualQuery(
    IN LPCVOID lpAddress,
    OUT PMEMORY_BASIC_INFORMATION lpBuffer,
    IN DWORD dwLength
    );

void mpi_address_( void *location,int str_len, MPI_Fint *address, MPI_Fint *__ierr )
{
    MPI_Aint a, b;

    *__ierr = MPI_Address( location, &a );
    if (*__ierr != MPI_SUCCESS) return;
    
    b = a - (MPI_Aint)MPIR_F_MPI_BOTTOM;
    *address = (int)( b );
    if (((MPI_Aint)*address) - b != 0) {
	*__ierr = MPIR_ERROR( MPIR_COMM_WORLD,     
      MPIR_ERRCLASS_TO_CODE(MPI_ERR_ARG,MPIR_ERR_FORTRAN_ADDRESS_RANGE),
			      "MPI_ADDRESS" );
    }
}

/* The version of MPI_ALLGATHER with only one string parameter. */
void mpi_allgather_ ( void *sendbuf,int len, MPI_Fint *sendcount, MPI_Fint *sendtype,
                    void *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *__ierr )
{

    MEMORY_BASIC_INFORMATION mem_info;
    BOOL first,second;
    /* This is quite complicated. We know that we got one string. This might either be
       sendbuf or recvbuf. So we are not sure it the string length is behind
       the first or the second buffer. All we can do is to guess.
       We try to solve the problem by checking if slen is a valid pointer
       If it is, most likely recvbuf will be the string and the parameters are shifted
       Please note that we can not be sure if we guessed right. If the length of the string
       is a valid address (which is unlikely, but possible) we are doomed.
    */
    MPIR_VirtualQuery((LPCVOID)len,&mem_info,sizeof(mem_info));
    first = (mem_info.State != MEM_COMMIT || !(mem_info.Protect & PAGE_READ_PRIV));
    MPIR_VirtualQuery((LPCVOID)recvbuf,&mem_info,sizeof(mem_info));
    second = (mem_info.State != MEM_COMMIT || !(mem_info.Protect & PAGE_WRITE_PRIV));

    if((first && second) || (!first && !second)) {
	/* oops, both possible positions of len are invalid or valid addresses.
	This is not good, so we give up guessing and return an error*/
	VF_BUF_ERROR("MPI_ALLGATHER" );
	return;
    }

    if(first) {
	if(MPI_Type_f2c(*sendtype) != MPI_CHAR) {
	    VF_TYPE_ERROR1("MPI_ALLGATHER");
	    return;
	}
	*__ierr = MPI_Allgather(MPIR_F_PTR(sendbuf), (int)*sendcount,
	                        MPI_Type_f2c(*sendtype),
	    			MPIR_F_PTR(recvbuf),
				(int)*recvcount,
				MPI_Type_f2c(*recvtype),
				MPI_Comm_f2c(*comm));
    } else {
	/* Now we have a problem. The length parameter is behind the second,
	   not behind the first buffer. So we have to shift the values...
	   It looks like this: 
	   local name     real value
	   strlen     --> sendcount,
	   sendcount  --> sendtype
	   sendtype   --> recvbuf
	   recvbuf    --> strlen
	*/
	if(MPI_Type_f2c(*recvtype) != MPI_CHAR) {
	    VF_TYPE_ERROR2("MPI_ALLGATHER");
	    return;
	}
	*__ierr = MPI_Allgather(MPIR_F_PTR(sendbuf), 
				(int)*(MPI_Fint*)len, /*sendcount*/
	                        MPI_Type_f2c(*sendcount),/*sendtype*/
	    			MPIR_F_PTR((void*)sendtype), /*recvbuf*/
				(int)*recvcount,
				MPI_Type_f2c(*recvtype),
				MPI_Comm_f2c(*comm));
    }

}

void mpi_allgatherv_ ( void *sendbuf, int len,MPI_Fint *sendcount,  MPI_Fint *sendtype, 
		       void *recvbuf, MPI_Fint *recvcounts, MPI_Fint *displs, 
		       MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *__ierr )
{

    MEMORY_BASIC_INFORMATION mem_info;
    BOOL first,second;
    /* This is quite complicated. We know that we got one string. This might either be
       sendbuf or recvbuf. So we are not sure it the string length is behind
       the first or the second buffer. All we can do is to guess.
       We try to solve the problem by checking if strlen is a valid pointer
       If it is, most likely recvbuf will be the string and the parameters are shifted
       Please note that we can not be sure if we guessed right. If the length of the string
       is a valid address (which is unlikely, but possible) we are doomed.
    */
    MPIR_VirtualQuery((LPCVOID)len,&mem_info,sizeof(mem_info));
    first = (mem_info.State != MEM_COMMIT || !(mem_info.Protect & PAGE_READ_PRIV));
    MPIR_VirtualQuery((LPCVOID)recvbuf,&mem_info,sizeof(mem_info));
    second = (mem_info.State != MEM_COMMIT || !(mem_info.Protect & PAGE_WRITE_PRIV));

    if((first && second) || (!first && !second)) {
	/* oops, both possible positions of strlen are invlid or valid addresses.
	This is not good, so we give up guessing and return an error*/
	VF_BUF_ERROR("MPI_ALLGATHERV" );
	return;
    }

    if(first) {
	if(MPI_Type_f2c(*sendtype) != MPI_CHAR) {
	    VF_TYPE_ERROR1("MPI_ALLGATHERV");
	    return;
	}

	        *__ierr = MPI_Allgatherv(MPIR_F_PTR(sendbuf), *sendcount,
                                 MPI_Type_f2c(*sendtype),
			         MPIR_F_PTR(recvbuf), recvcounts,
                                 displs, MPI_Type_f2c(*recvtype),
			         MPI_Comm_f2c(*comm));

    } else {
 	if(MPI_Type_f2c(*recvtype) != MPI_CHAR) {
	    VF_TYPE_ERROR2("MPI_ALLGATHERV");
	    return;
	}
	*__ierr = MPI_Allgatherv(MPIR_F_PTR(sendbuf), 
				 (int)*(MPI_Fint*)len, /*sendcount*/
	                         MPI_Type_f2c(*sendcount),/*sendtype*/
	    			 MPIR_F_PTR((void*)sendtype), /*recvbuf*/
				 recvcounts,displs,
				 MPI_Type_f2c(*recvtype),
				 MPI_Comm_f2c(*comm));

    }
}

void mpi_alltoall_( void *sendbuf, int len, MPI_Fint *sendcount, MPI_Fint *sendtype, 
                  void *recvbuf, MPI_Fint *recvcnt, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *__ierr )
{
    MEMORY_BASIC_INFORMATION mem_info;
    BOOL first,second;
    MPIR_VirtualQuery((LPCVOID)len,&mem_info,sizeof(mem_info));
    first = (mem_info.State != MEM_COMMIT || !(mem_info.Protect & PAGE_READ_PRIV));
    MPIR_VirtualQuery((LPCVOID)recvbuf,&mem_info,sizeof(mem_info));
    second = (mem_info.State != MEM_COMMIT || !(mem_info.Protect & PAGE_WRITE_PRIV));

    if((first && second) || (!first && !second)) {
	VF_BUF_ERROR("MPI_ALLTOALL" );
	return;
    }

    if(first) {
	if(MPI_Type_f2c(*sendtype) != MPI_CHAR) {
	    VF_TYPE_ERROR1("MPI_ALLTOALL");
	    return;
	}


        *__ierr = MPI_Alltoall(MPIR_F_PTR(sendbuf), (int)*sendcount,
	                       MPI_Type_f2c(*sendtype), MPIR_F_PTR(recvbuf),
	                       (int)*recvcnt, MPI_Type_f2c(*recvtype),
		               MPI_Comm_f2c(*comm) );
    } else {
	if(MPI_Type_f2c(*recvtype) != MPI_CHAR) {
	    VF_TYPE_ERROR2("MPI_ALLTOALL");
	    return;
	}
	*__ierr = MPI_Alltoall(MPIR_F_PTR(sendbuf), 
			       (int)*(MPI_Fint*)len, /*sendcount*/
	                       MPI_Type_f2c(*sendcount),/*sendtype*/
	    		       MPIR_F_PTR((void*)sendtype), /*recvbuf*/
	                       (int)*recvcnt, MPI_Type_f2c(*recvtype),
		               MPI_Comm_f2c(*comm) );

    }
}

void mpi_alltoallv_ ( void *sendbuf,int len, MPI_Fint *sendcnts, MPI_Fint *sdispls, MPI_Fint *sendtype, 
                    void *recvbuf, MPI_Fint *recvcnts, MPI_Fint *rdispls, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *__ierr )
{
    
    MEMORY_BASIC_INFORMATION mem_info;
    BOOL first,second;
    MPIR_VirtualQuery((LPCVOID)len,&mem_info,sizeof(mem_info));
    first = (mem_info.State != MEM_COMMIT || !(mem_info.Protect & PAGE_READ_PRIV));
    MPIR_VirtualQuery((LPCVOID)recvbuf,&mem_info,sizeof(mem_info));
    second = (mem_info.State != MEM_COMMIT || !(mem_info.Protect & PAGE_WRITE_PRIV));

    if((first && second) || (!first && !second)) {
	VF_BUF_ERROR("MPI_ALLTOALLV" );
	return;
    }

    if(first) {
	if(MPI_Type_f2c(*sendtype) != MPI_CHAR) {
	    VF_TYPE_ERROR1("MPI_ALLTOALLV");
	    return;
	}
    
	*__ierr = MPI_Alltoallv(MPIR_F_PTR(sendbuf), sendcnts, 
                                sdispls, MPI_Type_f2c(*sendtype),
			        MPIR_F_PTR(recvbuf), recvcnts, 
                                rdispls, MPI_Type_f2c(*recvtype),
			        MPI_Comm_f2c(*comm) );
    } else {
	if(MPI_Type_f2c(*recvtype) != MPI_CHAR) {
	    VF_TYPE_ERROR2("MPI_ALLTOALLV");
	    return;
	}

	*__ierr = MPI_Alltoallv(MPIR_F_PTR(sendbuf), 
				(MPI_Fint*)len,		/*sendcnts*/
				sendcnts,		/*sdispls*/
                                MPI_Type_f2c(*sdispls), /*sendtype*/
				MPIR_F_PTR(sendtype),   /*sendbuf*/
			        recvcnts, rdispls, MPI_Type_f2c(*recvtype),
			        MPI_Comm_f2c(*comm) );
    }
}


void mpi_bcast_ ( void *buffer,int len, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *__ierr )
{
   

    *__ierr = MPI_Bcast(MPIR_F_PTR(buffer), (int)*count, 
                        MPI_Type_f2c(*datatype), (int)*root,
                        MPI_Comm_f2c(*comm));
}

void mpi_bsend_init_( void *buf,int len, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *__ierr )
{
    MPI_Request lrequest;
    *__ierr = MPI_Bsend_init(MPIR_F_PTR(buf),(int)*count,
                             MPI_Type_f2c(*datatype),
                             (int)*dest,
                             (int)*tag,MPI_Comm_f2c(*comm), 
                             &lrequest);
    *request = MPI_Request_c2f(lrequest);
}

void mpi_bsend_( void *buf, int len,MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *__ierr )
{
    *__ierr = MPI_Bsend(MPIR_F_PTR(buf),(int)*count,MPI_Type_f2c(*datatype),
                    (int)*dest,(int)*tag,MPI_Comm_f2c(*comm) );
}

void mpi_buffer_attach_( void *buffer,int len, MPI_Fint *size, MPI_Fint *__ierr )
{
    *__ierr = MPI_Buffer_attach(buffer,(int)*size);
}


void mpi_buffer_detach_( void **buffer,int len, MPI_Fint *size, MPI_Fint *__ierr )
{
  void *tmp = (void *)buffer;
  int lsize;

  *__ierr = MPI_Buffer_detach(&tmp,&lsize);
  *size = (MPI_Fint)lsize;
}

void mpi_recv_init_( void *buf,int len, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *__ierr )
{
    MPI_Request lrequest;
    *__ierr = MPI_Recv_init(MPIR_F_PTR(buf),(int)*count,
                            MPI_Type_f2c(*datatype),(int)*source,(int)*tag,
			    MPI_Comm_f2c(*comm),&lrequest);
    *request = MPI_Request_c2f(lrequest);
}


void mpi_send_init_( void *buf, int len, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *__ierr )
{
    MPI_Request lrequest;
    *__ierr = MPI_Send_init(MPIR_F_PTR(buf),(int)*count, 
                            MPI_Type_f2c(*datatype),(int)*dest,(int)*tag,
                            MPI_Comm_f2c(*comm),&lrequest);
    *request = MPI_Request_c2f( lrequest );
}


void mpi_gather_ ( void *sendbuf,int len, MPI_Fint *sendcnt, MPI_Fint *sendtype, 
		   void *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, 
		   MPI_Fint *root, MPI_Fint *comm, MPI_Fint *__ierr )
{

    MEMORY_BASIC_INFORMATION mem_info;
    BOOL first,second;
    MPIR_VirtualQuery((LPCVOID)len,&mem_info,sizeof(mem_info));
    first = (mem_info.State != MEM_COMMIT || !(mem_info.Protect & PAGE_READ_PRIV));
    MPIR_VirtualQuery((LPCVOID)recvbuf,&mem_info,sizeof(mem_info));
    second = (mem_info.State != MEM_COMMIT || !(mem_info.Protect & PAGE_WRITE_PRIV));

    if((first && second) || (!first && !second)) {
	VF_BUF_ERROR("MPI_GATHER" );
	return;
    }

    if(first) {
	if(MPI_Type_f2c(*sendtype) != MPI_CHAR) {
	    VF_TYPE_ERROR1("MPI_GATHER");
	    return;
	}
	*__ierr = MPI_Gather(MPIR_F_PTR(sendbuf), (int)*sendcnt,
                     MPI_Type_f2c(*sendtype), MPIR_F_PTR(recvbuf),
                     (int)*recvcount, MPI_Type_f2c(*recvtype),
                     (int)*root, MPI_Comm_f2c(*comm));
    } else {
	if(MPI_Type_f2c(*recvtype) != MPI_CHAR) {
	    VF_TYPE_ERROR2("MPI_GATHER");
	    return;
	}
	*__ierr = MPI_Gather(MPIR_F_PTR(sendbuf), 
		    *(int*)len,MPI_Type_f2c(*sendcnt),
                     MPIR_F_PTR(sendtype),(int)*recvcount, 
		     MPI_Type_f2c(*recvtype),
                     (int)*root, MPI_Comm_f2c(*comm));

    }
}

void mpi_gatherv_ ( void *sendbuf, int len, MPI_Fint *sendcnt,  MPI_Fint *sendtype, 
                  void *recvbuf, MPI_Fint *recvcnts, MPI_Fint *displs, MPI_Fint *recvtype, 
                  MPI_Fint *root, MPI_Fint *comm, MPI_Fint *__ierr )
{

    MEMORY_BASIC_INFORMATION mem_info;
    BOOL first,second;
    MPIR_VirtualQuery((LPCVOID)len,&mem_info,sizeof(mem_info));
    first = (mem_info.State != MEM_COMMIT || !(mem_info.Protect & PAGE_READ_PRIV));
    MPIR_VirtualQuery((LPCVOID)recvbuf,&mem_info,sizeof(mem_info));
    second = (mem_info.State != MEM_COMMIT || !(mem_info.Protect & PAGE_WRITE_PRIV));

    if((first && second) || (!first && !second)) {
	VF_BUF_ERROR("MPI_GATHERV" );
	return;
    }

    if(first) {
	if(MPI_Type_f2c(*sendtype) != MPI_CHAR) {
	    VF_TYPE_ERROR1("MPI_GATHERV");
	    return;
	}

        *__ierr = MPI_Gatherv(MPIR_F_PTR(sendbuf), *sendcnt,
                              MPI_Type_f2c(*sendtype), MPIR_F_PTR(recvbuf),
                              recvcnts, displs, 
                              MPI_Type_f2c(*recvtype), *root,
                              MPI_Comm_f2c(*comm));
    } else {
	if(MPI_Type_f2c(*recvtype) != MPI_CHAR) {
	    VF_TYPE_ERROR2("MPI_GATHERV");
	    return;
	}
       *__ierr = MPI_Gatherv(MPIR_F_PTR(sendbuf), 
			  *(int*)len,MPI_Type_f2c(*sendcnt),
                          MPIR_F_PTR(sendtype),
                          recvcnts, displs, 
                          MPI_Type_f2c(*recvtype), *root,
                          MPI_Comm_f2c(*comm));
    }
}

void mpi_ibsend_( void *buf,int len, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *__ierr )
{
    MPI_Request lrequest;
    *__ierr = MPI_Ibsend(MPIR_F_PTR(buf),(int)*count,MPI_Type_f2c(*datatype),
                         (int)*dest,(int)*tag,MPI_Comm_f2c(*comm),
                         &lrequest);
    *request = MPI_Request_c2f(lrequest);
}

void mpi_irecv_( void *buf,int len, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *__ierr )
{
    MPI_Request lrequest;
    *__ierr = MPI_Irecv(MPIR_F_PTR(buf),(int)*count,MPI_Type_f2c(*datatype),
			(int)*source,(int)*tag,
                        MPI_Comm_f2c(*comm),&lrequest);
    *request = MPI_Request_c2f(lrequest);
}

void mpi_irsend_( void *buf,int len, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *__ierr )
{
    MPI_Request lrequest;
    *__ierr = MPI_Irsend(MPIR_F_PTR(buf),(int)*count,MPI_Type_f2c(*datatype),
                     (int)*dest,(int)*tag,
                     MPI_Comm_f2c(*comm),&lrequest);
    *request = MPI_Request_c2f(lrequest);
}

void mpi_isend_( void *buf,int len, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *__ierr )
{
    MPI_Request lrequest;
    *__ierr = MPI_Isend(MPIR_F_PTR(buf),(int)*count,MPI_Type_f2c(*datatype),
                        (int)*dest,
                        (int)*tag,MPI_Comm_f2c(*comm),
			&lrequest);
    *request = MPI_Request_c2f(lrequest);
}


void mpi_issend_( void *buf,int len, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *__ierr )
{
    MPI_Request lrequest;
    *__ierr = MPI_Issend(MPIR_F_PTR(buf),(int)*count,MPI_Type_f2c(*datatype),
                         (int)*dest, (int)*tag,
                         MPI_Comm_f2c(*comm),
			 &lrequest);
    *request = MPI_Request_c2f(lrequest);
}

void mpi_recv_( void *buf,int len, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *__ierr )
{
    MPI_Status c_status;

    *__ierr = MPI_Recv(MPIR_F_PTR(buf), (int)*count,MPI_Type_f2c(*datatype),
                       (int)*source, (int)*tag,
		       MPI_Comm_f2c(*comm), &c_status);
    MPI_Status_c2f(&c_status, status);
}

void mpi_rsend_init_( void *buf,int len, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *__ierr )
{
    MPI_Request lrequest;
    *__ierr = MPI_Rsend_init(MPIR_F_PTR(buf), (int)*count, 
                             MPI_Type_f2c(*datatype), (int)*dest,
                             (int)*tag,
			     MPI_Comm_f2c(*comm), &lrequest);
    *request = MPI_Request_c2f(lrequest);
}

void mpi_rsend_( void *buf,int len, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *__ierr )
{
    *__ierr = MPI_Rsend(MPIR_F_PTR(buf), (int)*count,MPI_Type_f2c(*datatype),
			(int)*dest, (int)*tag, MPI_Comm_f2c(*comm));
}

void mpi_scatter_ ( void *sendbuf,int len, MPI_Fint *sendcnt, MPI_Fint *sendtype, 
                    void *recvbuf, MPI_Fint *recvcnt, MPI_Fint *recvtype, 
                    MPI_Fint *root, MPI_Fint *comm, MPI_Fint *__ierr )
{
    MEMORY_BASIC_INFORMATION mem_info;
    BOOL first,second;
    MPIR_VirtualQuery((LPCVOID)len,&mem_info,sizeof(mem_info));
    first = (mem_info.State != MEM_COMMIT || !(mem_info.Protect & PAGE_READ_PRIV));
    MPIR_VirtualQuery((LPCVOID)recvbuf,&mem_info,sizeof(mem_info));
    second = (mem_info.State != MEM_COMMIT || !(mem_info.Protect & PAGE_WRITE_PRIV));

    if((first && second) || (!first && !second)) {
	VF_BUF_ERROR("MPI_SCATTER" );
	return;
    }

    if(first) {
	if(MPI_Type_f2c(*sendtype) != MPI_CHAR) {
	    VF_TYPE_ERROR1("MPI_SCATTER" );
	    return;
	}

    *__ierr = MPI_Scatter(MPIR_F_PTR(sendbuf), (int)*sendcnt,
                          MPI_Type_f2c(*sendtype), MPIR_F_PTR(recvbuf),
                          (int)*recvcnt, MPI_Type_f2c(*recvtype), 
                          (int)*root, MPI_Comm_f2c(*comm));
    } else {
	if(MPI_Type_f2c(*recvtype) != MPI_CHAR) {
	    VF_TYPE_ERROR2("MPI_SCATTER" );
	    return;
	}
	*__ierr = MPI_Scatter(MPIR_F_PTR(sendbuf),
			    *(int*)len,
			    MPI_Type_f2c(*sendcnt),
			    MPIR_F_PTR(sendtype),
			    (int)*recvcnt, MPI_Type_f2c(*recvtype), 
			    (int)*root, MPI_Comm_f2c(*comm));
    }
}

void mpi_scatterv_ ( void *sendbuf,int len, MPI_Fint *sendcnts, MPI_Fint *displs, MPI_Fint *sendtype, 
                   void *recvbuf, MPI_Fint *recvcnt,  MPI_Fint *recvtype, 
                   MPI_Fint *root, MPI_Fint *comm, MPI_Fint *__ierr )
{
    MEMORY_BASIC_INFORMATION mem_info;
    BOOL first,second;
    MPIR_VirtualQuery((LPCVOID)len,&mem_info,sizeof(mem_info));
    first = (mem_info.State != MEM_COMMIT || !(mem_info.Protect & PAGE_READ_PRIV));
    MPIR_VirtualQuery((LPCVOID)recvbuf,&mem_info,sizeof(mem_info));
    second = (mem_info.State != MEM_COMMIT || !(mem_info.Protect & PAGE_WRITE_PRIV));

    if((first && second) || (!first && !second)) {
	VF_BUF_ERROR("MPI_SCATTERV" );
	return;
    }

    if(first) {
	if(MPI_Type_f2c(*sendtype) != MPI_CHAR) {
	    VF_TYPE_ERROR1("MPI_SCATTERV");
	    return;
	}
    
        *__ierr = MPI_Scatterv(MPIR_F_PTR(sendbuf), sendcnts, displs,
                           MPI_Type_f2c(*sendtype), MPIR_F_PTR(recvbuf),
                           *recvcnt, MPI_Type_f2c(*recvtype),
                           *root, MPI_Comm_f2c(*comm) );
    } else {
	if(MPI_Type_f2c(*recvtype) != MPI_CHAR) {
	    VF_TYPE_ERROR2("MPI_SCATTERV");
	    return;
	}
	*__ierr = MPI_Scatterv(MPIR_F_PTR(sendbuf), 
			   (MPI_Fint*)len,
			    sendcnts, MPI_Type_f2c(*displs),
                           MPIR_F_PTR(sendtype),
                           *recvcnt, MPI_Type_f2c(*recvtype),
                           *root, MPI_Comm_f2c(*comm) );
    
    }
}

void mpi_send_( void *buf,int len, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *__ierr )
{
    *__ierr = MPI_Send(MPIR_F_PTR(buf), (int)*count, MPI_Type_f2c(*datatype),
                       (int)*dest, (int)*tag, MPI_Comm_f2c(*comm));
}

void mpi_sendrecv_( void *sendbuf,int len, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *dest, MPI_Fint *sendtag, 
                  void *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *source, MPI_Fint *recvtag, 
                  MPI_Fint *comm, MPI_Fint *status, MPI_Fint *__ierr )
{
    MPI_Status c_status;
    MEMORY_BASIC_INFORMATION mem_info;
    BOOL first,second;
    MPIR_VirtualQuery((LPCVOID)len,&mem_info,sizeof(mem_info));
    first = (mem_info.State != MEM_COMMIT || !(mem_info.Protect & PAGE_READ_PRIV));
    MPIR_VirtualQuery((LPCVOID)recvbuf,&mem_info,sizeof(mem_info));
    second = (mem_info.State != MEM_COMMIT || !(mem_info.Protect & PAGE_WRITE_PRIV));

    if((first && second) || (!first && !second)) {
	VF_BUF_ERROR("MPI_SENDRECV" );
	return;
    }

    if(first) {
	if(MPI_Type_f2c(*sendtype) != MPI_CHAR) {
	    VF_TYPE_ERROR1("MPI_SENDRECV");	    
	    return;
	}
    

	*__ierr = MPI_Sendrecv(MPIR_F_PTR(sendbuf), (int)*sendcount, 
			       MPI_Type_f2c(*sendtype), (int)*dest, 
			       (int)*sendtag, MPIR_F_PTR(recvbuf), 
			       (int)*recvcount, MPI_Type_f2c(*recvtype),
			       (int)*source, (int)*recvtag,
			       MPI_Comm_f2c(*comm), &c_status);
    } else {
	if(MPI_Type_f2c(*recvtype) != MPI_CHAR) {
	    VF_TYPE_ERROR2("MPI_SENDRECV");
	    return;
	}
	*__ierr = MPI_Sendrecv(MPIR_F_PTR(sendbuf), 
				  *(int*)len,
				    MPI_Type_f2c(*sendcount), 
				   (int)*sendtype, (int)*dest, 
				   MPIR_F_PTR(sendtag), 
				   (int)*recvcount, MPI_Type_f2c(*recvtype),
				   (int)*source, (int)*recvtag,
				   MPI_Comm_f2c(*comm), &c_status);
    }
    MPI_Status_c2f(&c_status, status);
}

void mpi_sendrecv_replace_( void *buf,int len, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *sendtag, 
     MPI_Fint *source, MPI_Fint *recvtag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *__ierr )
{
    MPI_Status c_status;

    *__ierr = MPI_Sendrecv_replace(MPIR_F_PTR(buf), (int)*count,
			     MPI_Type_f2c(*datatype), (int)*dest, 
                             (int)*sendtag, (int)*source, (int)*recvtag,
				   MPI_Comm_f2c(*comm), &c_status );
    MPI_Status_c2f(&c_status, status);
}

void mpi_ssend_init_( void *buf,int len, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *__ierr )
{
    MPI_Request lrequest;
    *__ierr = MPI_Ssend_init(MPIR_F_PTR(buf), (int)*count, 
                             MPI_Type_f2c(*datatype), (int)*dest, (int)*tag,
			     MPI_Comm_f2c(*comm), &lrequest);
    *request = MPI_Request_c2f(lrequest);
}

void mpi_ssend_( void *buf,int len, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *__ierr )
{
    *__ierr = MPI_Ssend(MPIR_F_PTR(buf), (int)*count, 
                        MPI_Type_f2c(*datatype), (int)*dest, (int)*tag,
                        MPI_Comm_f2c(*comm));
}


 
#endif

