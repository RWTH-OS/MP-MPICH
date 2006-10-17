/* $Id$ */

/* Handle delayed gahered one-sided transactions. */


#include "adi3types.h"

#include "smidef.h"
#include "smidev.h"
#include "smistat.h"
#include "smiostypes.h"
#include "smicoll.h"
#include "sside_macros.h"
#include "mpiops.h"
#include "job.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

struct os_vec {
	ulong  os_offset; 	/* offset of data in window */
	size_t os_len;    	/* length of data  */
};


/* Store a transaction in the delayed-ta-queue. This is fairly easy. */
int  MPID_SMI_Os_delayed_store (MPID_Win *win, MPID_SMI_Delayed_ta_t *os_ta)
{
	MPID_STAT_ENTRY (sside_delay_store);

    MPID_tree_insert (win->devinfo.w_smi.delayed_ta[os_ta->ta_type][os_ta->target_lrank], os_ta);

	if (win->devinfo.w_smi.ta_count[os_ta->ta_type][os_ta->target_lrank] == 0)
		win->devinfo.w_smi.ta_proc_count[os_ta->ta_type]++;
    win->devinfo.w_smi.ta_count[os_ta->ta_type][os_ta->target_lrank]++;
    win->devinfo.w_smi.ta_total_len[os_ta->ta_type][os_ta->target_lrank] += os_ta->contig_size;

    /* If this is an accumulate, check for the accumulate operation. If it differs
       from the previous ones, we need to flush the delayed ta's because all delayed
       accumulates must use the same operation. */
    if (os_ta->ta_type == OS_TA_ACCU) {
		if (win->devinfo.w_smi.ta_accu_op != 0 && 
			win->devinfo.w_smi.ta_accu_op != os_ta->ta_accu_op) 
			MPID_SMI_Os_delayed_flush (win, -1);
		win->devinfo.w_smi.ta_accu_op = os_ta->ta_accu_op;
    }

	MPID_STAT_EXIT (sside_delay_store);
    return MPI_SUCCESS;
}


/* Send out stored transactions from the delayed-ta-queue of window 'win'.
   Only process transaction for 'to_lrank' target process, or all transactions
   if 'to_lrank == -1'.

   This is usually done in a synchronization point. We need to loop over 
   all processes in the group of this window, retrieve all stored transactions
   of a certain type for a single process, generate the related message and 
   proceed. Finally, need to wait for the completion of all messages. */
int MPID_SMI_Os_delayed_flush (MPID_Win *win, int to_lrank)
{
    MPID_PKT_ONESIDED_T prepkt;
    MPID_SMI_CTRLPKT_T pkt_desc;
    MPID_SMI_Delayed_ta_t *os_ta;
    MPI_Request *msg_reqs, *get_reqs;
    MPI_Status *msg_status;
    struct os_vec *os_desc;
    os_ta_t ta_type;
	ulong buf_offset, buf_len, buf_pos;
    int proc, lower_proc = 0, upper_proc = win->comm->np;
    int nbr_get_ta = 0, nbr_os_ta = 0, ta_cnt = 0, get_ta_cnt = 0, ta;
	int req, *nbr_get_blocks, nbr_get_ta_complete, is_complete;
    char **msg_bufs, **get_bufs;
	char ***origin_addr;
	struct os_vec **get_vec;

	MPID_STAT_ENTRY (sside_delay_flush);

	if (MPID_SMI_cfg.SSIDED_DELAY == 0) {
		MPID_STAT_EXIT (sside_delay_flush);
		return MPI_SUCCESS;
	}

    if (to_lrank >= 0) {
		lower_proc = to_lrank;
		upper_proc = to_lrank + 1;
    }

    /* Check the processes for which transactions need to be processed, and 
	   allocate the required number of requests and status'. */
	nbr_get_ta = win->devinfo.w_smi.ta_proc_count[OS_TA_GET];
	for (nbr_os_ta = 0, ta_type = OS_TA_PUT; ta_type <= OS_TA_ACCU; ta_type++) {
		if (win->devinfo.w_smi.ta_proc_count[ta_type] > 0) {
			nbr_os_ta++;
			win->devinfo.w_smi.ta_proc_count[ta_type] = 0;
		}
	}
	if (nbr_os_ta > 0) {
		ALLOCATE (msg_reqs, MPI_Request *, sizeof(MPI_Request) * nbr_os_ta);
		ALLOCATE (msg_status, MPI_Status *, sizeof(MPI_Status) * nbr_os_ta);
		ZALLOCATE (msg_bufs, char **, sizeof(char *) * nbr_os_ta);
    } else {
		MPID_STAT_EXIT (sside_delay_flush);
		return MPI_SUCCESS;
	}
    if (nbr_get_ta > 0) {
		ALLOCATE (get_reqs, MPI_Request *, sizeof(MPI_Request) * nbr_get_ta);
		ZALLOCATE (get_bufs, char **, sizeof(char *) * nbr_get_ta);
		ZALLOCATE (get_vec, struct os_vec **, sizeof(struct os_vec *) * nbr_get_ta);
		ZALLOCATE (origin_addr, char ***, sizeof(char **) * nbr_get_ta);
		ZALLOCATE (nbr_get_blocks, int *, sizeof(int) * nbr_get_ta);
    }

	/* Now process the delayed transaction for all processes and transaction types. */
    for (proc = lower_proc; proc < upper_proc; proc++) {
		for (ta_type = OS_TA_PUT; ta_type <= OS_TA_ACCU; ta_type++) {
			if (win->devinfo.w_smi.ta_count[ta_type][proc] == 0)
				continue;

			MPID_SMI_Add_target_job(win, proc);

			/* Send announcement message. It needs to contain the tag of the
			   following data message. For thread-safety, this tag needs to be unique. */
			switch (ta_type) {
			case OS_TA_PUT:
				MPID_INIT_OS_PREPKT (prepkt, win->comm->send_context, win->comm->local_rank,
									 win->id, OS_TA_PUT, 0, win->devinfo.w_smi.delayed_tag,
									 win->devinfo.w_smi.ta_count[ta_type][proc], 
									 win->devinfo.w_smi.ta_total_len[ta_type][proc]);
				break;
			case OS_TA_ACCU:
				MPID_INIT_OS_PREPKT (prepkt, win->comm->send_context, win->comm->local_rank,
									 win->id, OS_TA_ACCU, win->devinfo.w_smi.ta_accu_op, 
									 win->devinfo.w_smi.delayed_tag,
									 win->devinfo.w_smi.ta_count[ta_type][proc], 
									 win->devinfo.w_smi.ta_total_len[ta_type][proc]);
				break;
			case OS_TA_GET:
				MPID_INIT_OS_PREPKT (prepkt, win->comm->send_context, win->comm->local_rank,
									 win->id, OS_TA_GET, 0, win->devinfo.w_smi.delayed_tag,
									 win->devinfo.w_smi.ta_count[ta_type][proc], 
									 win->devinfo.w_smi.ta_total_len[ta_type][proc]);
				break;
			}
			MPID_GETSENDPKT(pkt_desc, sizeof(MPID_PKT_ONESIDED_T), &prepkt, 0, 0, 
							win->comm->lrank_to_grank[proc], 0);
			while (MPID_SMI_SendControl(&pkt_desc) != MPI_SUCCESS)
				;
			/* XXX this signal could also be send *after* this loop - but then we 
			   would need to remember which processes we need to communicate with. */
			if (MPID_SMI_cfg.SSIDED_SIGNAL)
				SMI_Signal_send (win->comm->lrank_to_grank[proc]|SMI_SIGNAL_ANY);

			/* Now create and fill the buffer to send with the data message. It contains
			   the i/o-vector-style len-offset description, followed by the data
			   (data goes with put and accumulate only). */
			switch (ta_type) {
			case OS_TA_PUT:
			case OS_TA_ACCU:
				buf_len = win->devinfo.w_smi.ta_total_len[ta_type][proc] + 
					win->devinfo.w_smi.ta_count[ta_type][proc] * sizeof(struct os_vec);
				ALLOCATE (msg_bufs[ta_cnt], char *, buf_len);

				for (buf_pos = 0, ta = 0; ta < win->devinfo.w_smi.ta_count[ta_type][proc]; ta++) {
					os_ta = MPID_tree_remove_smallest (win->devinfo.w_smi.delayed_ta[ta_type][proc]);
					MPID_ASSERT (os_ta != NULL, "mismatch of nbr of stored os transactions.");

					os_desc = (struct os_vec *)(msg_bufs[ta_cnt] + buf_pos);
					os_desc->os_offset = os_ta->target_offset;
					os_desc->os_len    = os_ta->contig_size;
					buf_pos += sizeof(struct os_vec);

					MEMCPY_S(msg_bufs[ta_cnt] + buf_pos, os_ta->origin_addr, os_ta->contig_size);
					buf_pos += os_ta->contig_size;
					MPID_SBfree(MPID_SMI_os_ta_allocator, os_ta);

				}
				win->devinfo.w_smi.ta_count[ta_type][proc] = 0;
				break;
			case OS_TA_GET:
				/* First, post recv to get the data itself. */
				buf_len = win->devinfo.w_smi.ta_total_len[ta_type][proc];
				ALLOCATE (get_bufs[get_ta_cnt], char *, buf_len);
				MPICH_Irecv (get_bufs[get_ta_cnt], buf_len, MPI_BYTE, proc, 
							 win->devinfo.w_smi.delayed_tag, win->comm->self, &get_reqs[get_ta_cnt]);
	      
				/* Now, send the os_vec description of the data to get. Keep this vector to
				   be able to unpack the incoming data message with it. */
				buf_len = win->devinfo.w_smi.ta_count[ta_type][proc] * sizeof(void *);
				ALLOCATE (origin_addr[ta_cnt], void *, buf_len);

				buf_len = win->devinfo.w_smi.ta_count[ta_type][proc] * sizeof(struct iovec);
				ALLOCATE (msg_bufs[ta_cnt], void *, buf_len);
				get_vec[get_ta_cnt] = (struct os_vec *)msg_bufs[ta_cnt];

				for (buf_pos = 0, ta = 0; ta < win->devinfo.w_smi.ta_count[ta_type][proc]; ta++) {
					os_ta = MPID_tree_remove_smallest (win->devinfo.w_smi.delayed_ta[ta_type][proc]);
					MPID_ASSERT (os_ta != NULL, "mismatch of nbr of stored os transactions.");

					os_desc = (struct os_vec *)(msg_bufs[ta_cnt] + buf_pos);
					os_desc->os_offset = os_ta->target_offset;
					os_desc->os_len    = os_ta->contig_size;
					buf_pos += sizeof(struct os_vec);

					origin_addr[ta_cnt][ta] = (char *)os_ta->origin_addr;
					nbr_get_blocks[ta_cnt]++;
					MPID_SBfree(MPID_SMI_os_ta_allocator, os_ta);
				}
				win->devinfo.w_smi.ta_count[ta_type][proc] = 0;
				get_ta_cnt++;
				break;
			}
			MPICH_Isend (msg_bufs[ta_cnt], buf_len, MPI_BYTE, proc, 
						 win->devinfo.w_smi.delayed_tag, win->comm->self, &msg_reqs[ta_cnt]);
			ta_cnt++;
		}
    }

    MPICH_Waitall (ta_cnt, msg_reqs, msg_status);

    /* All request have been sent out; now wait for the responses (only relevant
       for 'get'-operations). */
    nbr_get_ta_complete = 0;
	is_complete = 0;
    while (get_ta_cnt > nbr_get_ta_complete) {
		for (req = 0; req < get_ta_cnt; req++) {
			if (get_reqs[req] != NULL) {
				MPI_Test (&get_reqs[req], &is_complete, &msg_status[req]);
				if (is_complete) {
					/* Unpack buffer */
					for (buf_offset = 0, ta = 0; ta < nbr_get_blocks[req]; ta++) {
						MEMCPY (origin_addr[req][ta], get_bufs[req] + buf_offset, get_vec[req][ta].os_len);
						buf_offset += get_vec[req][ta].os_len;
					}

					MPID_SMI_Remove_target_job(win, -1);
					nbr_get_ta_complete++;

					get_reqs[req] = NULL;
					FREE (origin_addr[req]);
					FREE (get_bufs[req]);

					is_complete   = 0;
				}
			}
		}
    }

	for (ta = 0; ta < ta_cnt; ta++) {
		FREE (msg_bufs[ta]);
	}
	FREE (msg_bufs);
	FREE (msg_reqs);
	FREE (msg_status);
	if (nbr_get_ta_complete > 0) {
		FREE (get_reqs);
		FREE (get_bufs);
		FREE (get_vec);
		FREE (origin_addr);
		FREE (nbr_get_blocks);
	}

	/* reset counters */
    for (proc = lower_proc; proc < upper_proc; proc++) {
		for (ta_type = OS_TA_PUT; ta_type <= OS_TA_ACCU; ta_type++) {
			win->devinfo.w_smi.ta_count[ta_type][proc] = 0;
			win->devinfo.w_smi.ta_total_len[ta_type][proc] = 0;
		}
	}

	MPID_STAT_EXIT (sside_delay_flush);
	return MPI_SUCCESS;
}


/* Process an incoming delayed onesided transaction control message.
   This means allocation the required buffers and post a recv for the
   data message to come. If the message has arrived, process the
   contained data by storing it at the final destination, or by sending
   the requested data (for a get-operation) */
int MPID_SMI_Os_delayed_process (MPID_PKT_T *pkt, int from_grank)
{
	MPID_PKT_ONESIDED_T *os_pkt = (MPID_PKT_ONESIDED_T *)pkt;
	MPID_Win *win;
    MPI_Request req;
    MPI_Status status;
	MPID_Datatype *dtype_ptr;
    os_ta_t ta_type;
	struct os_vec *os_desc;
	ulong buf_len, buf_offset;
	int data_len, nbr_ta, lrank, msg_tag, accu_op, ta, i;
	char *data_in, *data_out;

	MPID_STAT_ENTRY (sside_delay_process);

	/* Get the window for which this transaction is dedicated. */
	win = MPID_GET_WIN_PTR (os_pkt->target_winid);

	ta_type  = (os_ta_t)os_pkt->ta_type;
	data_len = os_pkt->data_len;
	nbr_ta   = os_pkt->nbr_ta;
	msg_tag  = os_pkt->data_tag;
	lrank    = os_pkt->lrank;

	MPID_SMI_Add_job_request(win, lrank);

	/* Allocate buffers and post receive for put/accumulate data or get request
	   description. */
	switch (ta_type) {
	case OS_TA_ACCU:
		dtype_ptr = MPIR_GET_DTYPE_PTR(os_pkt->dtype);
		accu_op   = os_pkt->accu_op;
		/* fall through */
	case OS_TA_PUT:
		buf_len = nbr_ta*sizeof(struct os_vec) + data_len;
		break;
	case OS_TA_GET:
		buf_len = nbr_ta*sizeof(struct os_vec);
		break;
	}

	MPID_SMI_FreeRecvPkt ((MPID_PKT_T *)os_pkt, from_grank, IS_CTRL_MSG);

	ALLOCATE (data_in, char *, buf_len);		
	MPICH_Irecv (data_in, buf_len, MPI_BYTE, lrank, msg_tag, win->comm->self, &req);
	MPICH_Waitall (1, &req, &status);

	switch (ta_type) {
	case OS_TA_PUT:
		for (buf_offset = 0, ta = 0; ta < nbr_ta; ta++) {
			os_desc = (struct os_vec *)(data_in + buf_offset);
			buf_offset += sizeof(struct os_vec);

			MEMCPY ((char*)win->start_address + os_desc->os_offset*win->disp_unit, 
					data_in + buf_offset, os_desc->os_len);

			buf_offset += os_desc->os_len;
		}
		MPID_SMI_Job_request_completed (win, lrank, 0);
		break;
	case OS_TA_ACCU:
		for (buf_offset = 0, ta = 0; ta < nbr_ta; ta++) {
			os_desc = (struct os_vec *)(data_in + buf_offset);
			buf_offset += sizeof(struct os_vec);

			/* XXX Only basic datatypes are supported! */
			switch (dtype_ptr->self) {
				/* integer types */
			case MPIR_INT:
			case MPIR_FORT_INT:
				for (i = 0; i < os_desc->os_len/sizeof(int); i++)
					DO_ACCU (int, (char*)win->start_address + os_desc->os_offset*win->disp_unit, 
							 data_in + buf_offset, accu_op);
				break;
			case MPIR_LONG:
				for (i = 0; i < os_desc->os_len/sizeof(long); i++)
					DO_ACCU (long, (char*)win->start_address + os_desc->os_offset*win->disp_unit, 
							 data_in + buf_offset, accu_op);
				break;
			case MPIR_SHORT:
				for (i = 0; i < os_desc->os_len/sizeof(short); i++)
					DO_ACCU (short, (char*)win->start_address + os_desc->os_offset*win->disp_unit, 
							 data_in + buf_offset, accu_op);
				break;
			case MPIR_CHAR: 
			case MPIR_BYTE:
				for (i = 0; i < os_desc->os_len/sizeof(char); i++)
					DO_ACCU (char, (char*)win->start_address + os_desc->os_offset*win->disp_unit, 
							 data_in + buf_offset, accu_op);
				break;
			case MPIR_UCHAR:
				for (i = 0; i < os_desc->os_len/sizeof(unsigned char); i++)
					DO_ACCU (unsigned char, (char*)win->start_address + os_desc->os_offset*win->disp_unit, 
							 data_in + buf_offset, accu_op);
				break;
			case MPIR_USHORT:
				for (i = 0; i < os_desc->os_len/sizeof(ushort); i++)
					DO_ACCU (ushort, (char*)win->start_address + os_desc->os_offset*win->disp_unit, 
							 data_in + buf_offset, accu_op);
				break;
			case MPIR_ULONG:
				for (i = 0; i < os_desc->os_len/sizeof(ulong); i++)
					DO_ACCU (ulong, (char*)win->start_address + os_desc->os_offset*win->disp_unit, 
							 data_in + buf_offset, accu_op);
				break;
			case MPIR_UINT:
				for (i = 0; i < os_desc->os_len/sizeof(uint); i++)
					DO_ACCU (uint, (char*)win->start_address + os_desc->os_offset*win->disp_unit, 
							 data_in + buf_offset, accu_op);
				break;
			case MPIR_LONGLONGINT:
				for (i = 0; i < os_desc->os_len/sizeof(longlong_t); i++)
					DO_ACCU (longlong_t, (char*)win->start_address + os_desc->os_offset*win->disp_unit, 
							 data_in + buf_offset, accu_op);
				break;

				/* float types */
			case MPIR_FLOAT:
				for (i = 0; i < os_desc->os_len/sizeof(float); i++)
					DO_ACCU_FLOAT (float, (char*)win->start_address + os_desc->os_offset*win->disp_unit, 
								   data_in + buf_offset, accu_op);
				break;
			case MPIR_DOUBLE:
				for (i = 0; i < os_desc->os_len/sizeof(double); i++)
					DO_ACCU_FLOAT (double, (char*)win->start_address + os_desc->os_offset*win->disp_unit, 
								   data_in + buf_offset, accu_op);
				break;
			case MPIR_LONGDOUBLE:
				for (i = 0; i < os_desc->os_len/sizeof(long double); i++)
					DO_ACCU_FLOAT (long double, (char*)win->start_address + os_desc->os_offset*win->disp_unit, 
								   data_in + buf_offset, accu_op);
				break;
				
				/* fortran complex types */
			case MPIR_COMPLEX:
				for (i = 0; i < os_desc->os_len/sizeof(s_complex); i++)
					DO_ACCU_COMPLEX (s_complex, (char*)win->start_address + os_desc->os_offset*win->disp_unit, 
								   data_in + buf_offset, accu_op);
				break;
			case MPIR_DOUBLE_COMPLEX:
				for (i = 0; i < os_desc->os_len/sizeof(d_complex); i++)
					DO_ACCU_COMPLEX (d_complex, (char*)win->start_address + os_desc->os_offset*win->disp_unit, 
									 data_in + buf_offset, accu_op);
				break;
			default:
				MPID_ABORT ("Unsupported datatype for delayed one-sided accumulate.");
				break;
			}
				
			buf_offset += os_desc->os_len;
		}
		MPID_SMI_Job_request_completed (win, lrank, 0);
		break;
	case OS_TA_GET:
		/* Store the requested data in a buffer and send it to the requester. */
		ALLOCATE (data_out, char *, data_len);

		os_desc = (struct os_vec *)data_in;
		for (buf_offset = 0, ta = 0; ta < nbr_ta; ta++) {			
			MEMCPY (data_out + buf_offset, 
					(char*)win->start_address + os_desc[ta].os_offset*win->disp_unit, 
					os_desc[ta].os_len);
			buf_offset += os_desc[ta].os_len;
		}
		MPICH_Send (data_out, data_len, MPI_BYTE, lrank, msg_tag, win->comm->self);
		MPID_SMI_Job_request_completed (win, lrank, 1);

		FREE (data_out);
		break;
	}
  
	FREE (data_in);
	
	MPID_STAT_EXIT (sside_delay_process);
	return MPI_SUCCESS;
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

