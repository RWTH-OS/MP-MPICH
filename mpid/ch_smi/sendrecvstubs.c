#include <stdlib.h>
#include <mpi.h>
#include "smicoll.h"
#include "adi3types.h"
#include "smidev.h"
#include "mpiimpl.h"
#include "mpid.h"
#include "mpid_bind.h"
#include "sendrecvstubs.h"


typedef void (send_contig_t) (	void*, int, int, int, int,
								int, MPID_Msgrep_t, int*);
typedef void (recv_contig_t) (	void*, int, int, int, int, 
								MPI_Status*, int*);
typedef void (isend_contig_t) (	void*, int, int, int, int,
								int, MPID_Msgrep_t, MPI_Request, int*);
typedef void (irecv_contig_t) (	void*, int, int, int, int,
								MPI_Request, int*);
typedef void (send_dtype_t) (	MPID_Comm*, void*, int, MPID_Datatype*,
								int, int, int, int, int*);
typedef void (recv_dtype_t) (	MPID_Comm*, void*, int, MPID_Datatype*,
								int, int, int, MPI_Status*, int*);
typedef void (isend_dtype_t) (	MPID_Comm*, void*, int, MPID_Datatype*,
								int, int, int, int, MPI_Request, int*, /* XXX check! */ int);
typedef void (irecv_dtype_t) (	MPID_Comm*, void*, int, MPID_Datatype*,
								int, int, int, MPI_Request, int*);
typedef void (iprobe_t) (	MPID_Comm*, int, int, int, int*, int*,
							MPI_Status*);
typedef void (pack_t) (	void*, int, MPID_Datatype*, void*, int, int*,
						MPID_Msgrep_t, MPID_Msg_pack_t,
						int*);
typedef void (unpack_t) (	void*, int, MPID_Msgrep_t, int*, void*, int,
							MPID_Datatype*, int*, MPID_Comm*, int*);
typedef void (packmsg_t) (	void*, int, MPID_Datatype*, MPID_Comm*, int,
							MPID_Msgrep_t, MPID_Msg_pack_t, void**, int*,
							int*);
typedef void (pack_size_t) (int, MPID_Datatype*, MPID_Msg_pack_t, int*);
typedef int (barrier_t) (MPI_Comm);



static send_contig_t	* send_contig_ptr = NULL;
static recv_contig_t	* recv_contig_ptr = NULL;
static isend_contig_t	* isend_contig_ptr = NULL;
static irecv_contig_t	* irecv_contig_ptr = NULL;
static send_dtype_t		* send_dtype_ptr = NULL;
static recv_dtype_t		* recv_dtype_ptr = NULL;
static isend_dtype_t	* isend_dtype_ptr = NULL;
static irecv_dtype_t	* irecv_dtype_ptr = NULL;
static iprobe_t			* iprobe_ptr = NULL;
static pack_t			* pack_ptr = NULL;
static unpack_t			* unpack_ptr = NULL;
static packmsg_t		* packmsg_ptr = NULL;
static pack_size_t		* pack_size_ptr = NULL;
static barrier_t		* barrier_ptr = NULL;

void MPID_SMI_Init_sendrecv_stubs (void)
{
#if defined MPI_SHARED_LIBS && !defined WIN32
	GET_DLL_FCTNPTR ("MPID_SendContig", send_contig_ptr, send_contig_t *);
	GET_DLL_FCTNPTR ("MPID_RecvContig", recv_contig_ptr, recv_contig_t *);
	GET_DLL_FCTNPTR ("MPID_SendDatatype", send_dtype_ptr, send_dtype_t *);
	GET_DLL_FCTNPTR ("MPID_RecvDatatype", recv_dtype_ptr, recv_dtype_t *);
	GET_DLL_FCTNPTR ("MPID_IsendContig", isend_contig_ptr, isend_contig_t *);
	GET_DLL_FCTNPTR ("MPID_IrecvContig", irecv_contig_ptr, irecv_contig_t *);
	GET_DLL_FCTNPTR ("MPID_IsendDatatype", isend_dtype_ptr, isend_dtype_t *);
	GET_DLL_FCTNPTR ("MPID_IrecvDatatype", irecv_dtype_ptr, irecv_dtype_t *);
	GET_DLL_FCTNPTR ("MPID_Iprobe", iprobe_ptr, iprobe_t *);
	GET_DLL_FCTNPTR ("MPID_Pack", pack_ptr, pack_t *);
	GET_DLL_FCTNPTR ("MPID_Unpack", unpack_ptr, unpack_t *);
	GET_DLL_FCTNPTR ("MPID_PackMessage", packmsg_ptr, packmsg_t *);
	GET_DLL_FCTNPTR ("MPID_Pack_size", pack_size_ptr, pack_size_t *);
	GET_DLL_FCTNPTR ("MPI_Barrier", barrier_ptr, barrier_t *);
#else 
	send_contig_ptr = MPID_SendContig;
	recv_contig_ptr = MPID_RecvContig;
	recv_dtype_ptr = MPID_RecvDatatype;
	isend_contig_ptr = MPID_IsendContig;
	irecv_contig_ptr = MPID_IrecvContig;
	isend_dtype_ptr = MPID_IsendDatatype;
	irecv_dtype_ptr = MPID_IrecvDatatype;
	iprobe_ptr = MPID_Iprobe;
	pack_ptr = MPID_Pack;
	unpack_ptr = MPID_Unpack;
	packmsg_ptr = MPID_PackMessage;
	pack_size_ptr = MPID_Pack_size;
	barrier_ptr = MPI_Barrier;
#endif
}


void MPID_SMIstub_SendContig(buf, len, src_lrank, tag, context_id,
							 dest_grank, msgrep, error_code )
	void			* buf;
	int				len, src_lrank, tag, context_id, 
					dest_grank, * error_code;
	MPID_Msgrep_t	msgrep;
{
	if (send_contig_ptr != NULL)
		(*send_contig_ptr) (buf, len, src_lrank, tag, context_id, 
							dest_grank, msgrep, error_code);
	else 
		MPID_SendContig (buf, len, src_lrank, tag, context_id,
						 dest_grank, msgrep, error_code);

	return;
}



void 
MPID_SMIstub_RecvContig(buf, maxlen, src_lrank, tag, context_id, 
						status, error_code)
	void 		* buf;
	int			maxlen;
	int			src_lrank;
	int			tag;
	int			context_id;
	MPI_Status	* status;
	int			* error_code;
{
	if (recv_contig_ptr != NULL)
	(*recv_contig_ptr) (buf, maxlen, src_lrank, tag, context_id,
						status, error_code);
	else
	MPID_RecvContig (	buf, maxlen, src_lrank, tag, context_id,
						status, error_code);

	return;
}




void 
MPID_SMIstub_SendDatatype(	comm, buf, count, dtype_ptr, src_lrank, tag,
							context_id, dest_grank, error_code )
	MPID_Comm		* comm;
	MPID_Datatype	* dtype_ptr;
	void			* buf;
	int				count, src_lrank, tag, context_id, 
					dest_grank, *error_code;
{
	if (send_dtype_ptr != NULL)
		(*send_dtype_ptr) (	comm, buf, count, dtype_ptr, src_lrank, tag,
							context_id, dest_grank, error_code);
	else
		MPID_SendDatatype (	comm, buf, count, dtype_ptr, src_lrank, tag,
							context_id, dest_grank, error_code);

	return;
}


void 
MPID_SMIstub_RecvDatatype(	comm, buf, count, dtype_ptr, src_lrank, tag,
							context_id, status, error_code )
	MPID_Comm		* comm;
	void 			* buf;
	int				count, src_lrank, tag, context_id, *error_code;
	MPID_Datatype	* dtype_ptr;
	MPI_Status		* status;
{
	if (recv_dtype_ptr != NULL)
		(*recv_dtype_ptr) (	comm, buf, count, dtype_ptr, src_lrank, tag,
							context_id, status, error_code);
	else
		MPID_RecvDatatype (	comm, buf, count, dtype_ptr, src_lrank, tag,
							context_id, status, error_code);

	return;
}


void 
MPID_SMIstub_IsendContig(	comm, buf, len, src_lrank, tag, context_id,
							dest_grank, msgrep, request, error_code )
	MPID_Comm		* comm;
	void			* buf;
	int				len, src_lrank, tag, context_id, 
					dest_grank, *error_code;
	MPID_Msgrep_t	msgrep;
	MPI_Request		request;
{
	if (isend_contig_ptr != NULL)
		(*isend_contig_ptr) (	buf, len, src_lrank, tag, context_id, 
								dest_grank, msgrep, request, error_code);
	else
		MPID_IsendContig (	buf, len, src_lrank, tag, context_id, 
							dest_grank, msgrep, request, error_code);

	return;
}


void 
MPID_SMIstub_IrecvContig(	comm, buf, maxlen, src_lrank, tag, context_id,
							request, error_code)
	MPID_Comm	* comm;
	void		* buf;
	int			maxlen, src_lrank, tag, context_id;
	MPI_Request	request;
	int			* error_code;
{
	if (irecv_contig_ptr != NULL)
		(*irecv_contig_ptr) (	buf, maxlen, src_lrank, tag, context_id, 
								request, error_code);
	else
		MPID_IrecvContig (	buf, maxlen, src_lrank, tag, context_id, 
							request, error_code);

	return;
}


void 
MPID_SMIstub_IsendDatatype (comm, buf, count, dtype_ptr, src_lrank, tag,
							context_id, dest_grank, request, error_code )
	MPID_Comm		* comm;
	MPID_Datatype	* dtype_ptr;
	void			*buf;
	int				count, src_lrank, tag, context_id, 
					dest_grank, *error_code;
	MPI_Request		request;
{
	if (isend_dtype_ptr != NULL)
		(*isend_dtype_ptr) (comm, buf, count, dtype_ptr, src_lrank, tag,
							context_id, dest_grank, request, error_code, /* XXX check! */ 1);
	else
		MPID_IsendDatatype (comm, buf, count, dtype_ptr, src_lrank, tag,
							context_id, dest_grank, request, error_code, 1);

	return;
}


void 
MPID_SMIstub_IrecvDatatype(	comm, buf, count, dtype_ptr, src_lrank, tag,
							context_id, request, error_code )
	MPID_Comm		* comm;
	MPID_Datatype	* dtype_ptr;
	void			* buf;
	int				count, src_lrank, tag, context_id, *error_code;
	MPI_Request		request;
{
	if (irecv_dtype_ptr != NULL)
		(*irecv_dtype_ptr) (comm, buf, count, dtype_ptr, src_lrank, tag,
							context_id, request, error_code);
	else
		MPID_IrecvDatatype (comm, buf, count, dtype_ptr, src_lrank, tag,
							context_id, request, error_code);

	return;
}


void 
MPID_SMIstub_Iprobe (	comm, tag, context_id, src_lrank, found, 
						error_code, status )
	MPID_Comm	* comm;
	int			tag, context_id, src_lrank, *found, *error_code;
	MPI_Status	* status;
{
	if (iprobe_ptr != NULL)
		(*iprobe_ptr) (comm, tag, context_id, src_lrank, found,
					   error_code, status);
	else
		MPID_Iprobe (comm, tag, context_id, src_lrank, found,
					 error_code, status);

	return;
}



void 
MPID_SMIstub_Pack(	src, count, dtype_ptr, dest, maxcount, position,
					comm, partner, msgrep, msgact, error_code )
	void			*src, *dest;
	int				count, maxcount, *position, partner, *error_code;
	MPID_Msgrep_t	msgrep;
	MPID_Datatype	* dtype_ptr;
	MPID_Comm		* comm;
	MPID_Msg_pack_t	msgact;
{
	if (pack_ptr != NULL)
		(*pack_ptr) (	src, count, dtype_ptr, dest, maxcount, position,
						msgrep, msgact, error_code);
	else
		MPID_Pack (	src, count, dtype_ptr, dest, maxcount, position,
					msgrep, msgact, error_code);

	return;
}



void 
MPID_SMIstub_Unpack (	src, maxcount, msgrep, in_position,
						dest, count, dtype_ptr, out_position,
						comm, partner, error_code )
	void			*src, *dest;
	int				maxcount, *in_position, count, *out_position, partner,
					*error_code;
	MPID_Datatype	* dtype_ptr;
	MPID_Comm		* comm;
	MPID_Msgrep_t	msgrep;
{
	if (unpack_ptr != NULL)
		(*unpack_ptr) (	src, maxcount, msgrep, in_position,
						dest, count, dtype_ptr, out_position,
						comm, error_code);
	else
		MPID_Unpack (	src, maxcount, msgrep, in_position,
						dest, count, dtype_ptr, out_position,
						comm, error_code);

	return;
}



void 
MPID_SMIstub_PackMessage (	src, count, dtype_ptr, comm, dest_grank,
							msgrep, msgact, mybuf, mylen, error_code )
	void			*src, **mybuf;
	MPID_Comm		* comm;
	int				count, dest_grank, *mylen, *error_code;
	MPID_Msgrep_t	msgrep;
	MPID_Datatype	* dtype_ptr;
	MPID_Msg_pack_t	msgact;
{
	if (packmsg_ptr != NULL)
		(*packmsg_ptr) (src, count, dtype_ptr, comm, dest_grank, 
						msgrep, msgact, mybuf, mylen, error_code);
	else
		MPID_PackMessage (	src, count, dtype_ptr, comm, dest_grank, 
							msgrep, msgact, mybuf, mylen, error_code);

	return;
}


void 
MPID_SMIstub_Pack_size (count, dtype_ptr, msgact, size )
	int				count, *size;
	MPID_Datatype	* dtype_ptr;
	MPID_Msg_pack_t	msgact;
{
	if (pack_size_ptr != NULL)
		(*pack_size_ptr) (count, dtype_ptr, msgact, size);
	else
		MPID_Pack_size (count, dtype_ptr, msgact, size);

	return;
}

/* maybe we should pack this into another file, or rename this one */
int 
MPID_SMIstub_Barrier (comm)
	MPID_Comm	* comm;
{
	if (barrier_ptr != NULL)
		return (*barrier_ptr) (comm->self);
	else
		return MPI_Barrier (comm->self);
}
















/*
 * Overrides for XEmacs and vim so that we get a uniform tabbing style.
 * XEmacs/vim will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:tw=0:ts=4:wm=0:
 */
