/* $Id$ */

/* Process incoming messges at the target process which have been sent
   by the origin process by a call to the "remote handler call" interface
   (see rhcv.c). 

   Basically, the sent data is written to the local part of the window (for MPI_Put / 
   MPI_Accumulate), or the requested data is sent tot the origin for MPI_Get). */

#include <stdlib.h>

#include "mpi.h"
#include "mpiimpl.h"

#include "mpiops.h"
#include "x86_ops.h"

#include "adi3types.h"
#include "smidev.h"

#include "smidef.h"
#include "maprank.h"
#include "job.h"
#include "mutex.h"
#include "uniqtag.h"
#include "smimem.h"
#include "packdtype.h"
#include "smipackdtype.h"
#include "sside_memcpy.h"
#include "remote_handler.h"
#include "mmu.h"
#include "sendrecvstubs.h"
#include "sside_macros.h"
#include "smistat.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* 
 * Put-function - to be executed at target process.
 */
int MPID_SMI_Do_put_emulation (pkt, from_grank)
	MPID_PKT_T *pkt;
	int			from_grank;	
{
	MPID_PKT_PUT_T  *put_pkt = (MPID_PKT_PUT_T *)pkt;
	MPID_Win		*win;
	MPID_Datatype	*dtype;
	MPID_Msg_pack_t	msgact = MPID_MSG_OK;
	void			*buf_dtype = NULL, *buf_packed_msg = NULL, *data = NULL, *target_addr = NULL;
	size_t			msglen;
	int				winid, in_pos, out_pos, error_code = MPI_SUCCESS;
	int             freepkt_flag = IS_CTRL_MSG;

	MPID_STAT_ENTRY (sside_put_emu_remote);

	win = MPID_GET_WIN_PTR (put_pkt->target_winid);
	if (MPID_TEST_WIN_NOTOK (put_pkt->target_winid, win)) {
		goto error;
	}
	MPID_SMI_Add_job_request(win, put_pkt->lrank);

	if (put_pkt->inline_data) {
		/* Copy data directly from the packet to destination */
		target_addr = (void *)((char *)win->start_address + put_pkt->target_offset*win->disp_unit);
		memcpy (target_addr, (char *)put_pkt + MPID_SMI_PUT_DATA_OFFSET, put_pkt->numofdata);
		/* In this case, it is not a plain ctrl msg, but has data inlined. */
		freepkt_flag = put_pkt->numofdata;
		goto finish;
	}
		
	/* Transfer the data in a blocking manner. */
	target_addr = (void *)((char *)win->start_address + put_pkt->target_offset*win->disp_unit);

	if (put_pkt->kind_dtype == MPID_DTYPE_UNKNOWN) {
		/* receive the datatype */
		ALLOCATE (buf_dtype, void *, put_pkt->size_dtype);
		MPID_SMIstub_RecvContig (buf_dtype, put_pkt->size_dtype, put_pkt->lrank, 
								 put_pkt->tag, win->comm->recv_context, NULL, &error_code);
		if (error_code) 
			goto error;
		/* recreate datatype, we need it here, to know the size of the package to receive */
		dtype = MPID_SMI_Unpack_dtype (buf_dtype, from_grank);
		if (!dtype) 
			goto error;
	} else {
		/* Get the datatype from the local "datatype cache". */
		if (put_pkt->kind_dtype != MPID_DTYPE_CONTIG) {
			dtype = MPID_SMI_Get_known_dtype (put_pkt->kind_dtype, from_grank);
		}
	}
	
	/* calculate message length */
	if (put_pkt->kind_dtype == MPID_DTYPE_CONTIG) {
		msglen = put_pkt->numofdata;
	} else {
		MPID_SMIstub_Pack_size (put_pkt->numofdata, dtype, msgact, (int *)&msglen);
	}

	/* Contiguous data can be received directly into the user buffer. */
	if (put_pkt->kind_dtype == MPID_DTYPE_CONTIG) {
		MPID_SMIstub_RecvContig (target_addr, msglen, put_pkt->lrank, put_pkt->tag, 
								 win->comm->recv_context, NULL, &error_code);
		if (error_code) 
			goto error;
		goto finish;
	}
	
	/* Non-contigous data is received into temporary buffer and unpacked from there. */
	ALLOCATE(buf_packed_msg, void *, msglen);
	MPID_SMIstub_RecvContig (buf_packed_msg, msglen, put_pkt->lrank, put_pkt->tag,
							 win->comm->recv_context, NULL, &error_code);
	if (error_code) 
		goto error;

	in_pos  = 0;
	out_pos = 0;
	MPID_SMIstub_Unpack (buf_packed_msg, msglen, MPID_MSGREP_RECEIVER, &in_pos, 
						 target_addr, put_pkt->numofdata, dtype, &out_pos, win->comm, 
						 put_pkt->lrank, &error_code);
	if (error_code) 
		goto error;

	goto finish;

error:
	SSIDE_ERR_CALL ("REMOTE_HANDLER: do_target_put_emu");
	error_code = -1;

finish:
	MPID_SMI_FreeRecvPkt (pkt, from_grank, freepkt_flag);
	MPID_SMI_Job_request_completed(win, put_pkt->lrank, 0);

	if (buf_packed_msg) 
		FREE (buf_packed_msg);
	if (buf_dtype) 
		FREE (buf_dtype);
	if (data) 
		FREE (data);

	MPID_STAT_EXIT (sside_put_emu_remote);
	return error_code;
}


/*
 * Get - function
 */
int MPID_SMI_Do_get_emulation (pkt, from_grank)
	 MPID_PKT_T	* pkt;
	 int			from_grank;
{
	MPID_Win				*win;
	MPID_Datatype			*dtype;
	MPID_Msg_pack_t	msgact = MPID_MSG_OK;
	size_t					origin_offset, target_offset;
	int						winid, origin_rank, origin_grank, numofdata;
	void					*target_addr;
	int						kind_dtype, size_dtype, do_rmt_put;
	int						retval = 1, sendtag;
	void			       *buf_dtype;
	int				        error_code = MPI_SUCCESS;
	int				        myrank;

	MPID_STAT_ENTRY (sside_get_emu_remote);

	winid 		   = pkt->get_pkt.target_winid;
	origin_offset  = pkt->get_pkt.origin_offset;
	target_offset  = pkt->get_pkt.target_offset;
	numofdata      = pkt->get_pkt.numofdata;
	kind_dtype     = pkt->get_pkt.kind_dtype;
	size_dtype     = pkt->get_pkt.size_dtype;
	origin_rank    = pkt->get_pkt.lrank;
	do_rmt_put     = pkt->get_pkt.do_remote_put;
	sendtag        = pkt->get_pkt.tag;
	MPID_SMI_FreeRecvPkt (pkt, from_grank, IS_CTRL_MSG);

	win = MPID_GET_WIN_PTR (winid);
	if (MPID_TEST_WIN_NOTOK (winid, win)) {
		goto error;
	}

	origin_grank = MPID_WIN_TO_GRANK_FAST (win, origin_rank);
	myrank       = win->devinfo.w_smi.lrank;
	target_addr  = (void *)((char *)win->start_address + target_offset*win->disp_unit);

	if (do_rmt_put && kind_dtype != MPID_DTYPE_UNKNOWN) {
		/* This get is a remote-put request for directly writing the data into
		   a shared memory region at the origin. */
		if (kind_dtype != MPID_DTYPE_CONTIG) {
			dtype = MPID_SMI_Get_known_dtype (kind_dtype, origin_grank);
			if (!dtype) 
				goto error;
		}
		
		if (kind_dtype == MPID_DTYPE_CONTIG) {
			MPID_SMI_Put_contig (target_addr, numofdata, origin_offset, 
								 origin_rank, win, NULL, 0);
		} else {
			MPID_SMI_Put_sametype (target_addr, numofdata, dtype, origin_offset, 
								   origin_rank, win, NULL, 0);
		}

		goto finish;
	}

	MPID_SMI_Add_job_request(win, origin_rank);

	if (kind_dtype == MPID_DTYPE_UNKNOWN) {
		/* receive the datatype */
		ALLOCATE(buf_dtype, void*, size_dtype);
		MPID_SMIstub_RecvContig (buf_dtype, size_dtype, origin_rank, sendtag, 
								 win->comm->recv_context, NULL, &error_code);
		if (error_code) 
			goto error;

		/* recreate datatype, we need it here, to know the size of
			the package to receive */
		dtype = MPID_SMI_Unpack_dtype (buf_dtype, origin_grank);
		if (!dtype) 
			goto error;
	} else 
		if (kind_dtype != MPID_DTYPE_CONTIG) {
			dtype = MPID_SMI_Get_known_dtype (kind_dtype, origin_grank);
		}

	/* send the data to the origin */
	if (kind_dtype == MPID_DTYPE_CONTIG) {
		MPID_SMIstub_SendContig (target_addr, numofdata, myrank, sendtag, 
								 win->comm->send_context, origin_grank, MPID_MSGREP_SENDER, 
								 &error_code);
	} else {
		MPID_SMIstub_SendDatatype (win->comm, target_addr, numofdata, dtype, myrank, sendtag, 
								   win->comm->send_context, origin_grank, &error_code);
	}
	if (!error_code) 
		goto finish;

 error:
	SSIDE_ERR_CALL ("REMOTE_HANDLER: do_target_get_emu");
	retval = 0;

 finish:
	MPID_SMI_Job_request_completed(win, origin_rank, 1);
	MPID_STAT_EXIT (sside_get_emu_remote);
	return retval;
}


/*
 * Accumulate - function
 */
int MPID_SMI_Do_accumulate (pkt, from_grank)
	 MPID_PKT_T	* pkt;
	 int			from_grank;		/* SMI rank */
{
	MPID_Datatype	* dtype;
	MPID_Win		* win;
	MPI_Op			op;
	void			*buf_dtype = NULL, *target_addr = NULL, *buf = NULL, *data;
	size_t			offset, size_dtype, msglen, bufsize;
	int				winid, origin_rank, numofdata, kind_dtype, sendtag;
	int				in_position, out_position;
	int				inline_data, origin_grank, i;
	int				error_code = MPI_SUCCESS;

	MPID_STAT_ENTRY (sside_accu_emu_remote);

	winid 		= pkt->accu_pkt.target_winid;
	offset 		= pkt->accu_pkt.target_offset;
	origin_rank	= pkt->accu_pkt.lrank;
	numofdata 	= pkt->accu_pkt.numofdata;
	sendtag		= pkt->accu_pkt.tag;
	op			= pkt->accu_pkt.op;
	inline_data	= pkt->accu_pkt.inline_data;
	kind_dtype 	= pkt->accu_pkt.kind_dtype;
	size_dtype	= pkt->accu_pkt.size_dtype;

	win = MPID_GET_WIN_PTR (winid);
	if (MPID_TEST_WIN_NOTOK (winid, win)) {
		return 0;
	}

	target_addr = (void *)((char *)win->start_address + offset * win->disp_unit);
	origin_grank = MPID_WIN_TO_GRANK_FAST (win, origin_rank);

	MPID_SMI_Add_job_request(win, origin_rank);

	if (kind_dtype == MPID_DTYPE_UNKNOWN) {
		/* receive the datatype */
		ALLOCATE(buf_dtype, void*, size_dtype);
		MPID_SMIstub_RecvContig (buf_dtype, size_dtype, origin_rank, sendtag, 
								 win->comm->recv_context, NULL, &error_code);
		if (error_code) 
			goto error;
		/* recreate datatype, we need it here, to know the size of
		   the package to receive */
		dtype = MPID_SMI_Unpack_dtype (buf_dtype, origin_grank);
		if (!dtype) 
			goto error;
	} else {
		dtype = MPID_SMI_Get_known_dtype (kind_dtype, origin_grank);
	}
	if (!dtype) 
		goto error;

	/* use the inlined data, or receive it in a separate message */
	if (inline_data) {
		buf = (char *)&pkt->accu_pkt + MPID_SMI_ACCU_DATA_OFFSET;
	} else {
		/* XXX direct accumulate from with in the incoming buffer 
		   (a la direct-reduce) would speed up things here! */
		bufsize = (kind_dtype == MPID_DTYPE_CONTIG) ? numofdata : dtype->extent*numofdata;
		ALLOCATE(buf, void *, bufsize);

		MPID_SMIstub_RecvDatatype (win->comm, buf, numofdata, dtype, origin_rank, sendtag, 
								   win->comm->recv_context, NULL, &error_code);
		if (error_code) 
			goto error;
	}

	/* now we can accumulate */
	MPID_SMI_LOCK (&win->devinfo.w_smi.accu_lock);
	for (i = 0; i < numofdata; i++) {
		MPID_SMI_Walk_thru_dtype_and_accu (dtype, (char *)target_addr + i*dtype->extent,
										   (char *)buf + i * dtype->extent, op);
	}
	MPID_SMI_UNLOCK (&win->devinfo.w_smi.accu_lock);

	/* job is done */
	goto finish;

 error:
	/* do some error handling */
	SSIDE_ERR_CALL ("REMOTE_HANDLER: do_target_accu_emu");

 finish:
	MPID_SMI_FreeRecvPkt (pkt, from_grank, IS_CTRL_MSG);
	MPID_SMI_Job_request_completed(win, origin_rank, 0);
	if (buf && !inline_data)
		FREE (buf);
	if (buf_dtype) 
		FREE (buf_dtype);
	
	MPID_STAT_EXIT (sside_accu_emu_remote);
	return 1;
}


/*
 * this function is needed by rhcv
 */
int MPID_SMI_Walk_thru_dtype_and_accu (dtype, target_addr, data, op)
	 MPID_Datatype	* dtype;
	 void			* target_addr;
	 void			* data;
	 MPI_Op			op;
{
	int i, j;

	switch (dtype->dte_type) {
		/* the nodes to ignore */
	case MPIR_UB: case MPIR_LB: 
	case MPIR_PACKED: case MPIR_LOGICAL: 
		break;
	/* complex types - require a recursive call */
	case MPIR_CONTIG:
		for (i=0; i<dtype->count; i++) {
			MPID_SMI_Walk_thru_dtype_and_accu (dtype->old_type, 
											   (char *)target_addr + i * dtype->old_type->extent, 
											   (char *)data + i * dtype->old_type->extent, op);
		}
		break;
	case MPIR_VECTOR:
	case MPIR_HVECTOR:
		for (i=0; i<dtype->count; i++) {
			for (j=0; j<dtype->blocklen; j++) {
				MPID_SMI_Walk_thru_dtype_and_accu (dtype->old_type,
												   (char *)target_addr + i*dtype->stride + 
												   j*dtype->old_type->extent,
												   (char *)data + i*dtype->stride +
												   j*dtype->old_type->extent, op);
			}
		}
		break;
	case MPIR_INDEXED:
	case MPIR_HINDEXED:
		for (i=0; i<dtype->count; i++) {
			for (j=0; j<dtype->blocklens[i]; j++) {
				MPID_SMI_Walk_thru_dtype_and_accu (dtype->old_type,
												   (char *)target_addr + dtype->indices[i] +
												   j*dtype->old_type->extent,
												   (char *)data + dtype->indices[i] +
												   j*dtype->old_type->extent, op);
			}
		}
		break;
	case MPIR_STRUCT:
		for (i=0; i<dtype->count; i++) {
			for (j=0; j<dtype->blocklens[i]; j++) {
				MPID_SMI_Walk_thru_dtype_and_accu (dtype->old_types[i],
												   (char *)target_addr + dtype->indices[i] +
												   j*dtype->old_types[i]->extent,
												   (char *)data + dtype->indices[i] +
												   j*dtype->old_types[i]->extent, op);
			}
		}
		break;
	/* the basic types - can be processed imediatly */
	/* int types */
	case MPIR_INT:
	case MPIR_FORT_INT:
		DO_ACCU (int, target_addr, data, op); 
		break;
	case MPIR_LONG:
		DO_ACCU (long, target_addr, data, op);
		break;
	case MPIR_SHORT:
		DO_ACCU (short, target_addr, data, op);
		break;
	case MPIR_CHAR: case MPIR_BYTE:
		DO_ACCU (char, target_addr, data, op);
		break;
	case MPIR_UCHAR:
		DO_ACCU (unsigned char, target_addr, data, op);
		break;
	case MPIR_USHORT:
		DO_ACCU (unsigned short, target_addr, data, op);
		break;
	case MPIR_ULONG:
		DO_ACCU (unsigned long, target_addr, data, op);
		break;
	case MPIR_UINT:
		DO_ACCU (unsigned, target_addr, data, op);
		break;
	case MPIR_LONGLONGINT:
		DO_ACCU (longlong_t , target_addr, data, op);
		break;
	/* float types */
	case MPIR_FLOAT:
		DO_ACCU_FLOAT (float, target_addr, data, op);
		break;
	case MPIR_DOUBLE:
		DO_ACCU_FLOAT (double, target_addr, data, op);
		break;
	case MPIR_LONGDOUBLE:
		DO_ACCU_FLOAT (long double, target_addr, data, op);
		break;
	/* fortran complex types */
	case MPIR_COMPLEX:
		DO_ACCU_COMPLEX (s_complex, target_addr, data, op);
		break;
	case MPIR_DOUBLE_COMPLEX:
		DO_ACCU_COMPLEX (d_complex, target_addr, data, op);
		break;
	default:
		break;
	}
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
