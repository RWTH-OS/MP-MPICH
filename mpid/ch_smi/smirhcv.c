/* $Id$
 *
 * MPID_SMI_Rhcv		- remote handler call (vector version)
 *
 * input parameters:
 *	rank			rank of process to invoke handler on
 *	comm			communicator that rank is relative to
 *	id				id of handler
 *	vector			vector of elements containing info to be delivered 
 *					to the remote handler
 *	count			number of elements in vector
 *
 * output parameters:
 *	local_flag		the location specified by this pointer is set when the 
 *					operation has completed
 *
 * return value:
 *	error code
 */

#include <stdlib.h>

#include "smi.h"
#include "mpi.h"

#include "mpiimpl.h"
#include "adi3types.h"
#include "smidev.h"
#include "req.h"

#include "maprank.h"
#include "job.h"
#include "mutex.h"
#include "uniqtag.h"
#include "packdtype.h"
#include "smipackdtype.h"
#include "smimem.h"
#include "mmu.h"
#include "sendrecvstubs.h"
#include "remote_handler.h"
#include "sside_macros.h"
#include "smistat.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif


static int send_put_pkt (	int, MPID_Win *, int, int, size_t, int, int,
							size_t, int, void *, size_t);
static int send_get_pkt (	int, MPID_Win *, int, int, size_t, int, int,
							size_t, int, int, int, size_t, size_t);
static int send_accu_pkt (	int, MPID_Win *, int, int, size_t, int, int,
							size_t, int, void *, size_t, MPI_Op);

static int do_origin_put_emu (	MPID_Win *, int, MPID_Hid_Put_emulation_t *,
								void *, MPID_Datatype *, MPID_Datatype *,
								volatile int *);
static int do_origin_get_emu (	MPID_Win *, int, MPID_Hid_Get_emulation_t *,
								void *, MPID_Datatype *, MPID_Datatype *,
								volatile int *);
static int do_origin_accu_emu (	MPID_Win *, int, MPID_Hid_Accumulate_t *,
								void *, MPID_Datatype *, MPID_Datatype *,
								volatile int *);


/*
 * Remote Handler Call with vector argument
 */
int  MPID_SMI_Rhcv (rank, win, id, vector, count, local_flag)
	 int 				rank;
	 MPID_Win 			* win;
	 MPID_Handler_id 	id;
	 const struct iovec 	vector[];
	 int 				count;
	 volatile int 		* local_flag;
{
	void			*hinfo, *buffer;
	MPID_Datatype	*dtype1, *dtype2;

	if (local_flag)
		*local_flag = 0;
	
	if (count < 2 || vector == NULL) 
		return MPI_ERR_ARG;

	hinfo = vector[0].iov_base;
	buffer = vector[1].iov_base;
	if (count >= 3)
		dtype1 = (MPID_Datatype *) vector[2].iov_base;
	else
		dtype1 = NULL;
	if (count >= 4)
		dtype2 = (MPID_Datatype *) vector[3].iov_base;
	else
		dtype2 = NULL;

	switch (id) {
	case MPID_Hid_Put_emulation:
		return do_origin_put_emu (win, rank, (MPID_Hid_Put_emulation_t *)hinfo,
								 buffer, dtype1, dtype2, local_flag) 
			? MPI_SUCCESS : MPI_ERR_INTERN;
		break;
	case MPID_Hid_Get_emulation:
		return do_origin_get_emu (win, rank,(MPID_Hid_Get_emulation_t *)hinfo,
								 buffer, dtype1, dtype2, local_flag) 
			? MPI_SUCCESS : MPI_ERR_INTERN;
		break;
	case MPID_Hid_Accumulate:
		return do_origin_accu_emu (win, rank, (MPID_Hid_Accumulate_t *)hinfo,
								  buffer, dtype1, dtype2, local_flag) 
			? MPI_SUCCESS : MPI_ERR_INTERN;
		break;
	}
	
	return MPI_ERR_UNSUPPORTED_OPERATION;	
}


/* 
 * Put emulation
 */
static int do_origin_put_emu (win, target_rank, hinfo, data, target_dtype, 
							  origin_dtype, localflag)
	MPID_Win					* win;
	int							target_rank;
	MPID_Hid_Put_emulation_t	* hinfo;
	void						* data;
	MPID_Datatype				* target_dtype,
								* origin_dtype;
	volatile int				* localflag;
{
	MPID_Msg_pack_t	msgact = MPID_MSG_OK;
	void 		   *buf_dtype, *buf, *mybuf, *target_addr;
	size_t			datasize;
	int             target_offset, target_count, origin_count;
	int             kind_dtype, dtypes_equal;
	int				psize = 0;
	int				error_code = MPI_SUCCESS;
	int				to_self, len, mylen, in_pos, out_pos;
	int				sendtag, mygrank, myrank, target_grank;

	MPID_STAT_ENTRY (sside_put_emulate);

	myrank = win->devinfo.w_smi.lrank;
	mygrank = MPID_WIN_TO_GRANK_FAST (win, myrank);
	to_self = (myrank == target_rank);
	target_grank = MPID_WIN_TO_GRANK_FAST (win, target_rank);

	target_offset	= hinfo->target_offset;
	target_count 	= hinfo->target_count;
	origin_count	= hinfo->origin_count;
	kind_dtype	    = hinfo->kind_dtype;
	dtypes_equal	= hinfo->dtypes_equal;
	origin_dtype	= hinfo->dtypes_equal ? target_dtype : origin_dtype;

	if (!to_self) {
		MPID_SMI_Add_target_job(win, target_rank);
		sendtag = MPID_SMI_Get_uniq_tag();
	}
	
	datasize = 0;
	if (!to_self && kind_dtype == MPID_DTYPE_CONTIG && dtypes_equal
		&& target_count == origin_count
		&& target_count <= MPID_SMI_PUT_DATA_SIZE) {
		/* We inline the data and thus send immeadiatly */

		datasize = hinfo->target_count; /* XXX what about the datatype? */
		if (!send_put_pkt (	target_grank, win, myrank, win->devinfo.w_smi.frames[target_rank].winid, 
							hinfo->target_offset, hinfo->target_count, hinfo->kind_dtype, 0, 
							sendtag, data, datasize))
			goto error;

		/* job is done - go home */
		if (localflag)
			*localflag = 1;

		goto finish;
	}
		
	/* create dtype representation to send lateron */
	if (kind_dtype == MPID_DTYPE_UNKNOWN && !to_self) {
		if (!MPID_SMI_Pack_dtype (target_dtype, target_grank, &buf_dtype, (size_t *)&psize))
			goto error;
	}

	/* send put-announce ctrl packet to remote handler */
	if (!to_self)
		if (!send_put_pkt (	target_grank, win, myrank, win->devinfo.w_smi.frames[target_rank].winid, 
							target_offset, target_count, kind_dtype, psize, 
							sendtag, data, datasize))
			goto error;
	
	/* send dtype if neccessary */
	if (kind_dtype == MPID_DTYPE_UNKNOWN && !to_self) {
		MPID_SMIstub_SendContig (buf_dtype, psize, myrank, sendtag, win->comm->send_context,
								 target_grank, MPID_MSGREP_SENDER, &error_code);
		if (error_code) 
			goto error;
	}	

	if (to_self) {
		/* maybe this stuff should be moved to another place */
		target_addr = (char *)win->start_address + win->disp_unit * target_offset;
		MPID_SMIstub_PackMessage (data, origin_count, origin_dtype, win->comm, mygrank, 
								  MPID_MSGREP_SENDER, MPID_MSG_OK, &mybuf, &mylen, &error_code);
		if (error_code) 
			goto error;
		in_pos = 0;	out_pos = 0;
		MPID_SMIstub_Unpack (mybuf, mylen, MPID_MSGREP_RECEIVER, &in_pos, target_addr, 
							 target_count, target_dtype,  &out_pos, win->comm, 
							 myrank, &error_code);
		FREE (mybuf);
		if (error_code) 
			goto error;

		goto finish;
	}
	
	/* send data to remote host */
	if (kind_dtype == MPID_DTYPE_CONTIG) {
		if (MPID_SMI_cfg.SSIDED_NONBLOCKING) {
			MPI_Request req = 0; MPIR_SHANDLE *sh = (MPIR_SHANDLE *)req;
			
			MPID_Send_alloc(sh);
			MPID_Request_init (req, MPIR_SEND);
			MPID_SMIstub_IsendContig (win->comm, data, origin_count, myrank, sendtag, win->comm->send_context,
									  target_grank, MPID_MSGREP_SENDER, req, &error_code);
			MPID_FIFO_push(win->devinfo.w_smi.putaccu_req_fifo, req);
		} else
			MPID_SMIstub_SendContig (data, origin_count, myrank, sendtag, win->comm->send_context,
									 target_grank, MPID_MSGREP_SENDER, &error_code);
	} else {
		/* we don't want to use MPID_SendDatatype here, because we
		   don't know wether the remote process already has got the
		   job lock, so we need to pack the data first */ 
		/* XXX is this still a problem?? */
		MPID_SMIstub_PackMessage (data, origin_count, origin_dtype, win->comm, target_grank, 
								  MPID_MSGREP_SENDER, msgact, &buf, &len, &error_code);
		if (error_code) 
			goto error;
		if (MPID_SMI_cfg.SSIDED_NONBLOCKING) {
			MPI_Request req = 0; MPIR_SHANDLE *sh = (MPIR_SHANDLE *)req;
			
			MPID_Send_alloc(sh);
			MPID_Request_init (req, MPIR_SEND);
			MPID_SMIstub_IsendContig (win->comm, buf, len, myrank, sendtag, win->comm->send_context,
									  target_grank, MPID_MSGREP_SENDER, req, &error_code);
			MPID_FIFO_push(win->devinfo.w_smi.putaccu_req_fifo, req);
		} else
			MPID_SMIstub_SendContig (buf, len, myrank, sendtag, win->comm->send_context,
									 target_grank, MPID_MSGREP_SENDER, &error_code);
		FREE (buf);
	}
	if (error_code) 
		goto error;

	/* job is done */
	goto finish;

 error:
	SSIDE_ERR_CALL("RHCV: do_origin_put_emu");
	MPID_SMI_Remove_target_job (win, target_rank);
	MPID_STAT_EXIT (sside_put_emulate);
	return 0;

 finish:
	/* set local flag, because the job is done locally */
	if (localflag)
		*localflag = 1;
	MPID_STAT_EXIT (sside_put_emulate);
	return 1;
}


/*
 * Get - emulation
 */
static int do_origin_get_emu (win, target_rank, hinfo, buffer, target_dtype, 
							  origin_dtype, localflag)
	MPID_Win					* win;
	int							target_rank;
	MPID_Hid_Get_emulation_t	* hinfo;
	void						* buffer;
	MPID_Datatype				* target_dtype,
								* origin_dtype;
	volatile int				* localflag;
{
	MPID_Msg_pack_t			msgact = MPID_MSG_OK;
	size_t					target_offset;
	int						kind_dtype, dtypes_equal;
	int						do_remote_put;
	void 					*buf_dtype, *mybuf, *target_addr;
	int						target_count, origin_count;
	volatile int			*remote_flag;
	int						psize = 0;
	int						sendtag, myrank, mygrank, target_grank;
	int						error_code = MPI_SUCCESS;
	int					    min_count, from_self;
	int						in_pos, out_pos, mylen;

	MPID_STAT_ENTRY (sside_get_emulate);

	MPID_SMI_Add_target_job(win, target_rank);

	target_offset	= hinfo->target_offset;
	target_count 	= hinfo->target_count;
	origin_count	= hinfo->origin_count;
	kind_dtype	    = hinfo->kind_dtype;
	dtypes_equal	= hinfo->dtypes_equal;
	do_remote_put	= hinfo->do_remote_put;
	origin_dtype    = hinfo->dtypes_equal ? target_dtype : origin_dtype;

	myrank    = win->devinfo.w_smi.lrank;
	mygrank   = MPID_WIN_TO_GRANK_FAST (win, myrank);
	from_self = (myrank == target_rank);

	target_grank = MPID_WIN_TO_GRANK_FAST (win, target_rank);
	if (kind_dtype == MPID_DTYPE_UNKNOWN && !from_self) {
		if (!MPID_SMI_Pack_dtype (target_dtype, target_grank, 
								  &buf_dtype, (size_t *)&psize))
			goto error;
	}

	if (!from_self)
		sendtag = MPID_SMI_Get_uniq_tag();
	
	/* Create and send packet to remote handler. We need more infos
	   than for a put_pkt, because a remote put needs to be handled as well */
	if (do_remote_put) {
		min_count = target_count < origin_count ? target_count : origin_count;
	} else {
		min_count = target_count;
	}
	
	/* If a remote_put is performed the datatypes need to be equivalent. 
	   This has to be checked by the calling function */
	if (!from_self)
		send_get_pkt (target_grank, win, myrank, win->devinfo.w_smi.frames[target_rank].winid, 
					  target_offset, min_count, kind_dtype, psize, sendtag,
					  do_remote_put, 0, 0, /* notfication is now done via job counters! */
					  (char *)buffer - (char *)win->start_address);
	
	/* send datatype if neccessary */
	if (kind_dtype == MPID_DTYPE_UNKNOWN && !from_self) {
		MPID_SMIstub_SendContig (buf_dtype, psize, myrank, sendtag, win->comm->send_context,
								 target_grank, MPID_MSGREP_SENDER, &error_code);
		if (error_code) goto error;
	}

	if (from_self) {
		/* maybe this stuff should be moved to another place */
		target_addr = (char *)win->start_address + win->disp_unit * target_offset;
		MPID_SMIstub_PackMessage (target_addr, target_count, target_dtype, win->comm, 
								  mygrank, MPID_MSGREP_SENDER, MPID_MSG_OK, &mybuf, &mylen, &error_code);
		if (error_code) 
			goto error;
		in_pos = 0; out_pos = 0;
		MPID_SMIstub_Unpack (mybuf, mylen, MPID_MSGREP_RECEIVER, &in_pos, buffer, origin_count, 
							 origin_dtype, &out_pos, win->comm, myrank, &error_code);
		FREE (mybuf);
		if (error_code) 
			goto error;

		MPID_SMI_Remove_target_job (win, target_rank);
	} else 
		/* finally receive the data */
		if (!do_remote_put) {
			if (kind_dtype == MPID_DTYPE_CONTIG) {
				if (MPID_SMI_cfg.SSIDED_NONBLOCKING) {
					MPI_Request req = 0; MPIR_RHANDLE *rh = (MPIR_RHANDLE *)req;
					
					MPID_Recv_alloc(rh);
					MPID_Request_init (req, MPIR_RECV);
					MPID_SMIstub_IrecvContig (win->comm, buffer, origin_count, target_rank, sendtag, 
											  win->comm->recv_context, req, &error_code);
					MPID_FIFO_push(win->devinfo.w_smi.get_req_fifo, req);
				} else {
					MPID_SMIstub_RecvContig (buffer, origin_count, target_rank, sendtag, 
											 win->comm->recv_context, NULL, &error_code);
					MPID_SMI_Remove_target_job (win, target_rank);
				}
				if (error_code) 
					goto error;
			} else {
				/* here we can do a RecvDatatype, because it is garantied, 
				   that both procs do have got a job lock */
				if (MPID_SMI_cfg.SSIDED_NONBLOCKING) {
					MPI_Request req = 0; MPIR_RHANDLE *rh = (MPIR_RHANDLE *)req;
					
					MPID_Recv_alloc(rh);
					MPID_Request_init (req, MPIR_RECV);
					MPID_SMIstub_IrecvDatatype (win->comm, buffer, origin_count, origin_dtype, 
												target_rank, sendtag, win->comm->recv_context, 
												req, &error_code);
					MPID_FIFO_push(win->devinfo.w_smi.get_req_fifo, req);
				} else {
					MPID_SMIstub_RecvDatatype (win->comm, buffer, origin_count, origin_dtype, 
											   target_rank, sendtag, win->comm->recv_context, 
											   NULL, &error_code);
					MPID_SMI_Remove_target_job (win, target_rank);
				}

				if (error_code) 
					goto error;
			}
		}
	
	goto finish;

 error:
	SSIDE_ERR_CALL("RHCV: do_origin_get_emu_bg");
	MPID_SMI_Remove_target_job (win, target_rank);
	MPID_STAT_EXIT (sside_get_emulate);
	return 0;

 finish:
	/* set local flag, because the job is done locally */
	if (localflag) 
		*localflag = 1;

	MPID_STAT_EXIT (sside_get_emulate);
	return 1;
}



/* 
 * Accumulate - emulation
 */
static int do_origin_accu_emu (win, target_rank, hinfo, data, target_dtype, 
							  origin_dtype, localflag)
	MPID_Win				*win;
	int						target_rank;
	MPID_Hid_Accumulate_t	*hinfo;
	void					*data;
	MPID_Datatype			*target_dtype,
							*origin_dtype;
	volatile int			*localflag;
{
	void 	*buf_dtype, *mybuf, *buffer, *target_addr;
	int     target_offset, target_count, origin_count;
	int     kind_dtype, dtypes_equal;
	int		sendtag, mygrank, myrank, target_grank, to_self;
	int		in_pos, out_pos;
	int		mylen, i;
	int		error_code = MPI_SUCCESS;
	size_t	datasize, psize = 0;


	MPID_STAT_ENTRY (sside_accu_emulate);

	if (!target_dtype) 
		return 0;

	target_offset	= hinfo->target_offset;
	target_count 	= hinfo->target_count;
	origin_count	= hinfo->origin_count;
	kind_dtype	    = hinfo->kind_dtype;
	dtypes_equal	= hinfo->dtypes_equal;
	origin_dtype    = hinfo->dtypes_equal ? target_dtype : origin_dtype;

	myrank = win->devinfo.w_smi.lrank;
	to_self = (myrank == target_rank);
	mygrank = MPID_WIN_TO_GRANK_FAST (win, myrank);
	target_grank = MPID_WIN_TO_GRANK_FAST (win, target_rank);

	/* Possibly delay the operation (and gather it with others) */
 	if (0 && /* XXX delayed does not yet work for accumulate! -> FIX ME */
		!to_self && target_dtype->is_contig && dtypes_equal 
		&& kind_dtype != MPID_DTYPE_UNKNOWN 
		&& target_count == origin_count 
		&& target_dtype->extent*target_count <= MPID_SMI_ACCU_DATA_SIZE
		&& target_dtype->extent*target_count < MPID_SMI_cfg.SSIDED_DELAY) {

		MPID_SMI_Delayed_ta_t *os_ta = (MPID_SMI_Delayed_ta_t *)
			MPID_SBalloc(MPID_SMI_os_ta_allocator);

		os_ta->target_lrank  = target_rank;
		os_ta->ta_type       = OS_TA_ACCU;
		os_ta->ta_accu_op    = hinfo->op;
		os_ta->origin_addr   = data;
		os_ta->target_offset = target_offset;
		os_ta->contig_size   = target_dtype->extent*target_count;

		MPID_SMI_Os_delayed_store (win, os_ta);
		
		goto finish;
	}

	MPID_SMI_Add_target_job (win, target_rank);

	if (!to_self) {
		sendtag = MPID_SMI_Get_uniq_tag ();
	}

	/* Can we inline the data and send it together with the put ctrl pkt? */
	datasize = 0;
 	if (!to_self && target_dtype->is_contig && dtypes_equal
		&& kind_dtype != MPID_DTYPE_UNKNOWN
		&& target_count == origin_count
		&& target_dtype->extent * target_count <= MPID_SMI_ACCU_DATA_SIZE) {

		datasize = target_dtype->extent * target_count;
		if (!send_accu_pkt (target_grank, win, myrank, win->devinfo.w_smi.frames[target_rank].winid, 
							target_offset, target_count, kind_dtype, 0, 
							sendtag, data, datasize, hinfo->op))
			goto error;

		goto finish;
	}
	
	if (kind_dtype == MPID_DTYPE_UNKNOWN && !to_self) {
		if (!MPID_SMI_Pack_dtype (target_dtype, target_grank, &buf_dtype, (size_t *)&psize))
			goto error;
	}

	/* create and send put-announce packet to remote handler, 
	   and send dtype if neccessary */
	if (!to_self) {
		if (!send_accu_pkt (target_grank, win, myrank, win->devinfo.w_smi.frames[target_rank].winid, 
							target_offset, target_count, kind_dtype, psize, 
							sendtag, data, datasize, hinfo->op))
			goto error;
	}
	if (kind_dtype == MPID_DTYPE_UNKNOWN && !to_self) {
		MPID_SMIstub_SendContig (buf_dtype, psize, myrank, sendtag, win->comm->send_context,
								 target_grank, MPID_MSGREP_SENDER, &error_code);
		if (error_code) 
			goto error;
	}
	
	if (to_self) {
		/* maybe this stuff should be moved to another place */
		if (!dtypes_equal) {
			MPID_SMIstub_PackMessage (data, origin_count, origin_dtype, win->comm, mygrank, 
									  MPID_MSGREP_SENDER, MPID_MSG_OK, &mybuf, &mylen, &error_code);
			if (error_code) 
				goto error;
			in_pos = 0; out_pos = 0;
			ALLOCATE (buffer, void *, origin_count*origin_dtype->extent);
			MPID_SMIstub_Unpack (mybuf, mylen, MPID_MSGREP_RECEIVER, &in_pos, buffer, 
								 target_count, target_dtype, &out_pos, win->comm, 
								 myrank, &error_code);
			FREE (mybuf);
			if (error_code) 
				goto error;
		} else {
			buffer = data;
		}
		target_addr = (char *)win->start_address + win->disp_unit * target_offset;
		for (i = 0; i < target_count; i++) 
			MPID_SMI_Walk_thru_dtype_and_accu (target_dtype, (char *)target_addr + i*target_dtype->extent,
											   (char *)buffer + i*target_dtype->extent, hinfo->op);
		
		if (!dtypes_equal) 
			FREE (buffer);

		MPID_SMI_Remove_target_job (win, target_rank);	
		goto finish;
	}

	/* send data to remote host */
	if (MPID_SMI_cfg.SSIDED_NONBLOCKING) {
		MPI_Request req = 0; MPIR_SHANDLE *sh = (MPIR_SHANDLE *)req;

		MPID_Send_alloc(sh);
		MPID_Request_init ((MPI_Request)req, MPIR_SEND);
		MPID_SMIstub_IsendDatatype (win->comm, data, origin_count, origin_dtype, myrank, sendtag, 
									win->comm->send_context, target_grank, req, &error_code);
		MPID_FIFO_push(win->devinfo.w_smi.putaccu_req_fifo, req);
	} else
		MPID_SMIstub_SendDatatype (win->comm, data, origin_count, origin_dtype, myrank, sendtag, 
								   win->comm->send_context, target_grank, &error_code);
	if (!error_code) 
		goto finish;

 error:
	SSIDE_ERR_CALL("RHCV: do_origin_accu_emu_bg");
	MPID_SMI_Remove_target_job (win, target_rank);	
	return 0;

 finish:
	/* set local flag, because the job is done locally */
	if (localflag)
		*localflag = 1;
	MPID_STAT_EXIT (sside_accu_emulate);
	return 1;
}


/*
 * low-level request-send functions
 */
static int send_put_pkt (	target_grank, win, myrank, winid, target_offset,
							numofdata, kind_dtype, size_dtype, sendtag, 
							data, datasize)
	 int			target_grank;
	 MPID_Win		* win;
	 int			myrank, winid;
	 size_t			target_offset, size_dtype;
	 int			numofdata, kind_dtype, sendtag;
	 void			* data;
	 size_t			datasize;
{
	MPID_PKT_PUT_T		* pkt;
	MPID_SMI_CTRLPKT_T	pkt_desc;
	int					target_drank;
	int					pkt_size;

	pkt_size = datasize ? MPID_SMI_PUT_PKT_SIZE + datasize :  sizeof (MPID_PKT_PUT_T);
	ALLOCATE (pkt, MPID_PKT_PUT_T *, pkt_size);

	target_drank = MPID_GRANK_TO_DRANK_FAST (target_grank);
	MPID_GETSENDPKT (pkt_desc, pkt_size, pkt, 0, NULL, target_drank, 0);

	pkt->mode 			= MPID_PKT_PUT;
	pkt->context_id		= win->comm->send_context;
	pkt->lrank 			= myrank;
	pkt->tag 			= sendtag;
	pkt->target_offset 	= target_offset;
	pkt->numofdata 		= numofdata;
	pkt->target_winid 	= winid;
	if (datasize > 0) {
		pkt->inline_data 	= 1;
		memcpy (((char *)pkt) + MPID_SMI_PUT_DATA_OFFSET, data, datasize);
	} else {
		pkt->inline_data = 0;
		pkt->kind_dtype  = kind_dtype;
		pkt->size_dtype  = size_dtype;
	}

	while (MPID_SMI_SendControl (&pkt_desc) != MPI_SUCCESS)
		;
	FREE(pkt);

	/* Signal the worker thread of the receivers, that a packet was sent 
	   this is configurable as it is not always wanted (costs some time). */
	if (MPID_SMI_cfg.SSIDED_SIGNAL)
		SMI_Signal_send (target_drank|SMI_SIGNAL_ANY);
	
	return 1;
}


static int send_get_pkt (target_grank, win, origin_lrank, target_winid,
						target_offset, target_count, kind_dtype, size_dtype,
						sendtag, do_remote_put, flag_id, flag_offset,
						origin_offset)
	 int target_grank, origin_lrank, target_winid, target_count;
	 int kind_dtype, sendtag, do_remote_put, flag_id;
	 size_t	target_offset, size_dtype, flag_offset, origin_offset;
	 MPID_Win *win;
{
	MPID_PKT_GET_T		pkt;
	MPID_SMI_CTRLPKT_T	pkt_desc;
	int					target_drank;

	target_drank = MPID_GRANK_TO_DRANK_FAST (target_grank);
	MPID_GETSENDPKT (pkt_desc, sizeof (MPID_PKT_GET_T), &pkt, 0, NULL, target_drank, 0);
	
	pkt.mode 			= MPID_PKT_GET;
	pkt.context_id	 	= win->comm->send_context;
	pkt.lrank 			= origin_lrank;
	pkt.tag 			= sendtag;
	pkt.target_offset 	= target_offset;
	pkt.numofdata 		= target_count;
	pkt.target_winid 	= target_winid;
	pkt.do_remote_put	= do_remote_put;
	pkt.kind_dtype		= kind_dtype;
	pkt.size_dtype		= size_dtype;
	pkt.flag_id			= flag_id;
	pkt.flag_offset		= flag_offset;
	pkt.origin_offset	= origin_offset;

	while (MPID_SMI_SendControl (&pkt_desc) != MPI_SUCCESS)
		;

	/* Signal the worker thread of the receivers, that a packet was sent 
	   this is configurable as it is not always wanted (costs some time). */
	if (MPID_SMI_cfg.SSIDED_SIGNAL)
		SMI_Signal_send (target_drank|SMI_SIGNAL_ANY);

	return 1;
}


static int send_accu_pkt (	target_grank, win, myrank, winid, target_offset,
							numofdata, kind_dtype, size_dtype, sendtag, 
							data, datasize, op)
	int				target_grank;
	MPID_Win		* win;
	int				myrank, winid;
	size_t			target_offset, size_dtype;
	int				numofdata, kind_dtype, sendtag;
	void			* data;
	size_t			datasize;
	MPI_Op			op;
{
	MPID_PKT_ACCU_T	   *pkt;
	MPID_SMI_CTRLPKT_T	pkt_desc;
	int					target_drank;
	int 				pkt_size;

	pkt_size = datasize ? MPID_SMI_ACCU_PKT_SIZE + datasize : sizeof (MPID_PKT_ACCU_T);
	ALLOCATE(pkt, MPID_PKT_ACCU_T *, pkt_size);

	target_drank = MPID_GRANK_TO_DRANK_FAST (target_grank);
	MPID_GETSENDPKT (pkt_desc, pkt_size, pkt, 0, NULL, target_drank, 0);

	pkt->mode 			= MPID_PKT_ACCU;
	pkt->context_id 	= win->comm->send_context;
	pkt->lrank 			= myrank;
	pkt->tag 			= sendtag;
	pkt->target_offset 	= target_offset;
	pkt->numofdata 		= numofdata;
	pkt->op				= op;
	pkt->target_winid 	= winid;
	pkt->kind_dtype		= kind_dtype;
	pkt->size_dtype		= size_dtype;
	if (datasize > 0) {
		pkt->inline_data 	= 1;
		pkt->datasize		= datasize;
		memcpy (((char *)pkt) + MPID_SMI_ACCU_DATA_OFFSET, data, datasize);
	} else {
		pkt->inline_data 	= 0;
	}

	while (MPID_SMI_SendControl (&pkt_desc) != MPI_SUCCESS)
		;
	FREE (pkt);
	
	/* Signal the worker thread of the receivers, that a packet was sent 
	   this is configurable as it is not always wanted (costs some time). */
	if (MPID_SMI_cfg.SSIDED_SIGNAL)
		SMI_Signal_send (target_drank|SMI_SIGNAL_ANY);
	
	return 1;
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
