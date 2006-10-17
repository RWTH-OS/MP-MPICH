#ifndef MPID_SSIDE_H
#define MPID_SSIDE_H

#include "adi3types.h"


/* memory management functions */
void * MPID_Alloc_mem (size_t size, MPID_Info * info);

int MPID_Free_mem (void * ptr);

/* window functions */
MPID_Win * MPID_Win_create (void *base, size_t size, int disp_unit,
							MPID_Info * info, MPID_Comm * comm);
int MPID_Win_free (MPID_Win * win);
int MPID_Win_incr (MPID_Win * win, int incr);

int MPID_Win_lock (MPID_Win * win, int rank, int type);
int MPID_Win_unlock (MPID_Win * win, int rank);
int MPID_Win_sync (int type, MPID_Win * win, MPID_Group * group, int assert);

/* put, get functions */
int MPID_Put_sametype (	void * origin_buf, int n, MPID_Datatype * dtype, 
						MPI_Aint target_offset, int target_rank, 
						MPID_Win * win, volatile int * local_flag, 
						MPI_Aint target_flag);

int MPID_Get_sametype (	void * origin_buf, int n, MPID_Datatype * dtype, 
						MPI_Aint target_offset, int target_rank, 
						MPID_Win * win, volatile int * local_flag, 
						MPI_Aint target_flag);

int MPID_Put_contig (	void * origin_buf, int n, MPI_Aint target_offset,
						int target_rank, MPID_Win *win,
						volatile int * local_flag, MPI_Aint target_flag);

int MPID_Get_contig (	void * origin_buf, int n, MPI_Aint target_offset,
						int target_rank, MPID_Win *win,
						volatile int * local_flag, MPI_Aint target_flag);

int MPID_Rhcv (	int rank, MPID_Win * win, MPID_Handler_id id,
				const struct iovec vector[], int count, int * local_flag);



	




























#endif /* MPID_SSIDE_H */



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
