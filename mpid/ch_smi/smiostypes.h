/* $Id$ */

#ifndef MPID_SMI_ONESIDED_TYPES_H
#define MPID_SMI_ONESIDED_TYPES_H

#include <sys/types.h>
#include <pthread.h>

#include "fifo.h"
#include "sbcnst2.h"
#include "smidef.h"
#include "tree.h"

/*
 * type definitions for one-sided communication
 */

typedef enum { OS_TA_PUT, OS_TA_GET, OS_TA_ACCU } os_ta_t;

/* Information about all remote memory areas of this window */
typedef struct {
	int		      winid;		        /* local winid of owner */
	void         *start_address;
	size_t		  length;
	size_t	      disp_unit;
	int           is_shared;
#if 0
	volatile int *job_count;
#endif
} MPID_SMI_Win_frame;


/* for general active-target sychronization */
typedef struct _MPID_SMI_Win_expose {
	volatile int	is_set;
	MPID_Group     *group;
	int				assert;
} MPID_SMI_Win_expose;


/* Store delayed transactions. */
typedef struct _MPID_SMI_Delayed_ta {
	int target_lrank;
	os_ta_t ta_type;
	int ta_accu_op;

	void  *origin_addr;
	ulong  target_offset;
	size_t contig_size;
} MPID_SMI_Delayed_ta_t;


/* Device-specific information for MPID_Win */
typedef struct {
	int					lrank, lsize;  /* communicator-relative rank and size */
	int					is_shared;     /* flag: local window located in shared memory? */
	MPID_SMI_Win_frame	*frames;       /* properties of all parts of the window */
	int					*winlocks;     /* IDs of global locks for (remote) window parts */

	/* Locks for multi-threaded device utilization. */
	MPID_SMI_LOCK_T		job_lock;
	MPID_SMI_LOCK_T		accu_lock;
	MPID_SMI_LOCK_T		jobcnt_lock;

	/* Unique tags for window-internal communication */
	/* XXX replace by ctrl messages */
	int					start_post_tag, complete_wait_tag,
						fence_tag, delayed_tag;
	                 
   /* job counters for window completion verification:
	  'ogn_jobcnt'
	    - jobs issued as origin proc (one counter for jobs to *all* target procs) which require
          response from target procs (emulated put/accumulate/get, remote-put)
      'ogn_cmpltcnt'
         - array in shared memory in which the counters for the target processes are placed
      'tgt_jobcnt'
   	    - number of jobs still to process as a target process to complete all currently 
          open exposure epoch (one counter for all potential origin procs)
      'tgt_cmpltcnt_rmt' (array of ptrs to places located at origin process)
      'tgt_cmpltcnt_loc' (cached copy of this counters, in local memory)
  	    - jobs completed as a target process (one counter for each potential origin proc) */
	volatile int   ogn_jobcnt;
	volatile int  *ogn_cmpltcnt;
	volatile int   tgt_jobcnt;
	volatile int **tgt_cmpltcnt_rmt, *tgt_cmpltcnt_loc;

	MPID_SMI_Win_expose	local_expose,	     /* post, wait, test */
						remote_expose;	     /* start, complete */
	int                 fenced;              /* true if we are in a fence-access-epoch */

	/* For non-blocking transfers, the requests need to be stored for completion check.
	   We need different fifo's for 'put'/'accumulate' and for 'get' due to the different handling
	   of the job-counters for these types of operations. */
	MPID_FIFO_t   putaccu_req_fifo, get_req_fifo;

	/* Queues for delayed transactions (one for each process and transaction type) */
	MPID_tree_t         *delayed_ta[3];
	/* Bookkeeping of stored transactions */
	int                 *ta_count[3];
	ulong               *ta_total_len[3], ta_proc_count[3];
	int                 ta_accu_op;          /* accumulate operation is currently gathered */
} MPID_SMI_Win;





























#endif /* MPID_SMI_ONESIDED_TYPES_H */



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
