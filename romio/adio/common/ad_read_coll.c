/* 
 *   $Id: ad_read_coll.c 5 2000-04-12 17:56:25Z karsten $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "adio.h"
#include "adio_extern.h"
#ifdef PROFILE
#include "mpe.h"
#endif

/* prototypes of functions used for collective reads only. */
static void ADIOI_Read_and_exch(ADIO_File fd, void *buf, MPI_Datatype
			 datatype, int nprocs, int
			 nprocs_for_coll, int myrank, ADIOI_Access
			 *others_req, ADIO_Offset *offset_list,
			 int *len_list, int contig_access_count, ADIO_Offset
                         min_st_offset, ADIO_Offset fd_size,
			 ADIO_Offset *fd_start, ADIO_Offset *fd_end,
                         int *buf_idx, int *error_code);
static void ADIOI_R_Exchange_data(ADIO_File fd, void *buf, ADIOI_Flatlist_node
			 *flat_buf, ADIO_Offset *offset_list, int
                         *len_list, int *send_size, int *recv_size,
			 int *count, int *start_pos, int *partial_send, 
			 int *recd_from_proc, int nprocs, 
			 int nprocs_for_coll, int myrank, int
			 buftype_is_contig, int contig_access_count,
			 ADIO_Offset min_st_offset, ADIO_Offset fd_size,
			 ADIO_Offset *fd_start, ADIO_Offset *fd_end, 
			 ADIOI_Access *others_req, 
                         int *recv_buf_idx, int *curr_from_proc,
                         int *done_from_proc, int iter, 
			 MPI_Aint buftype_extent, int *buf_idx);
static void ADIOI_Fill_user_buffer(void *buf, ADIOI_Flatlist_node
                           *flat_buf, char **recv_buf, ADIO_Offset 
                           *offset_list, int *len_list, int *recv_size, 
                           MPI_Request *requests, MPI_Status *statuses,
                           int *recd_from_proc, int nprocs_for_coll, 
                           int contig_access_count, ADIO_Offset min_st_offset, 
                           ADIO_Offset fd_size, ADIO_Offset *fd_start, 
                           ADIO_Offset *fd_end, int *recv_buf_idx, 
                           int *curr_from_proc, int *done_from_proc,
                           MPI_Aint buftype_extent);


void ADIOI_GEN_ReadStridedColl(ADIO_File fd, void *buf, int count,
                       MPI_Datatype datatype, int file_ptr_type,
                       ADIO_Offset offset, ADIO_Status *status, int
                       *error_code)
{
/* Uses a generalized version of the extended two-phase method described
   in "An Extended Two-Phase Method for Accessing Sections of 
   Out-of-Core Arrays", Rajeev Thakur and Alok Choudhary,
   Scientific Programming, (5)4:301--317, Winter 1996. 
   http://www.mcs.anl.gov/home/thakur/ext2ph.ps */

    ADIOI_Access *my_req; 
    /* array of nprocs structures, one for each other process in
       whose file domain this process's request lies */
    
    ADIOI_Access *others_req;
    /* array of nprocs structures, one for each other process
       whose request lies in this process's file domain. */

    int i, filetype_is_contig, nprocs, nprocs_for_coll, myrank;
    int *len_list, contig_access_count, interleave_count, info_flag;
    int *count_my_req_per_proc, count_my_req_procs, count_others_req_procs;
    int buftype_is_contig, *buf_idx;
    ADIO_Offset *offset_list, start_offset, end_offset, *st_offsets, orig_fp;
    ADIO_Offset *fd_start, *fd_end, fd_size, min_st_offset, *end_offsets;
    ADIO_Offset off;
    char *value;
#ifdef HAVE_STATUS_SET_BYTES
    int bufsize, size;
#endif

#ifdef PROFILE
        MPE_Log_event(13, 0, "start computation");
#endif

    MPI_Comm_size(fd->comm, &nprocs);
    MPI_Comm_rank(fd->comm, &myrank);

/* the number of processes that actually perform I/O, nprocs_for_coll,
   is stored in the info object. it is either = nprocs or a smaller number
   set by the user. get it from info. */

    value = (char *) ADIOI_Malloc((MPI_MAX_INFO_VAL+1)*sizeof(char));
    MPI_Info_get(fd->info, "cb_nodes", MPI_MAX_INFO_VAL, value, &info_flag);
    nprocs_for_coll = atoi(value);
    ADIOI_Free(value);

/* For this process's request, calculate the list of offsets and
   lengths in the file and determine the start and end offsets. */

/* Note: end_offset points to the last byte-offset that will be accessed.
         e.g., if start_offset=0 and 100 bytes to be read, end_offset=99*/

    orig_fp = fd->fp_ind;
    ADIOI_Calc_my_off_len(fd, count, datatype, file_ptr_type, offset,
			   &offset_list, &len_list, &start_offset,
			   &end_offset, &contig_access_count); 
    
/*    for (i=0; i<contig_access_count; i++) {
	FPRINTF(stderr, "rank %d  off %ld  len %d\n", myrank, offset_list[i], len_list[i]);
    }*/

/* each process communicates its start and end offsets to other 
   processes. The result is an array each of start and end offsets stored
   in order of process rank. */ 
    
    st_offsets = (ADIO_Offset *) ADIOI_Malloc(nprocs*sizeof(ADIO_Offset));
    end_offsets = (ADIO_Offset *) ADIOI_Malloc(nprocs*sizeof(ADIO_Offset));

    MPI_Allgather(&start_offset, 1, ADIO_OFFSET, st_offsets, 1, ADIO_OFFSET, 
		  fd->comm);
    MPI_Allgather(&end_offset, 1, ADIO_OFFSET, end_offsets, 1, ADIO_OFFSET, 
		  fd->comm);

/* are the accesses of different processes interleaved? */
    interleave_count = 0;
    for (i=1; i<nprocs; i++)
	if (st_offsets[i] < end_offsets[i-1]) interleave_count++;
/* This is a rudimentary check for interleaving, but should suffice
   for the moment. */

    ADIOI_Datatype_iscontig(datatype, &buftype_is_contig);

    if (!interleave_count) {
/* no interleaving of requests. noncollective is good enough */
	ADIOI_Free(offset_list);
	ADIOI_Free(len_list);
	ADIOI_Free(st_offsets);
	ADIOI_Free(end_offsets);	

	fd->fp_ind = orig_fp;
	ADIOI_Datatype_iscontig(fd->filetype, &filetype_is_contig);

	if (buftype_is_contig && filetype_is_contig) {
	    if (file_ptr_type == ADIO_EXPLICIT_OFFSET) {
		off = fd->disp + (fd->etype_size) * offset;
		ADIO_ReadContig(fd, buf, count, datatype, ADIO_EXPLICIT_OFFSET,
                       off, status, error_code);
	    }
	    else ADIO_ReadContig(fd, buf, count, datatype, ADIO_INDIVIDUAL,
                       0, status, error_code);
	}
	else ADIO_ReadStrided(fd, buf, count, datatype, file_ptr_type,
                       offset, status, error_code);

	return;
    }

/* Divide the I/O workload among "nprocs_for_coll" processes. This is
   done by (logically) dividing the file into file domains (FDs); each
   process may directly access only its own file domain. */

    ADIOI_Calc_file_domains(st_offsets, end_offsets, nprocs,
			    nprocs_for_coll, &min_st_offset,
			    &fd_start, &fd_end, &fd_size);   


/* calculate what portions of the access requests of this process are
   located in the file domains of other processes */

    ADIOI_Calc_my_req(offset_list, len_list, contig_access_count,
			   min_st_offset, fd_start, fd_end, fd_size,
			   nprocs, nprocs_for_coll, &count_my_req_procs, 
			   &count_my_req_per_proc, &my_req,
			   &buf_idx);

/* based on everyone's my_req, calculate what requests of other
   processes lie in this process's file domain.
   count_others_req_procs = number of processes whose requests lie in
   this process's file domain (including this process itself) 
   count_others_req_per_proc[i] indicates how many separate contiguous
   requests of proc. i lie in this process's file domain. */

    ADIOI_Calc_others_req(fd, count_my_req_procs, 
			       count_my_req_per_proc, my_req, 
			       nprocs, myrank, nprocs_for_coll,
			       &count_others_req_procs, &others_req); 

    ADIOI_Free(count_my_req_per_proc);
    for (i=0; i<nprocs_for_coll; i++) {
	if (my_req[i].count) {
	    ADIOI_Free(my_req[i].offsets);
	    ADIOI_Free(my_req[i].lens);
	}
    }
    ADIOI_Free(my_req);


/* read data in sizes of no more than ADIOI_Coll_bufsize, 
   communicate, and fill user buf. */
    ADIOI_Read_and_exch(fd, buf, datatype, nprocs, nprocs_for_coll, myrank,
                        others_req, offset_list,
			len_list, contig_access_count, min_st_offset,
			fd_size, fd_start, fd_end, buf_idx, error_code);

    if (!buftype_is_contig) ADIOI_Delete_flattened(datatype);

/* free all memory allocated for collective I/O */

    for (i=0; i<nprocs; i++) {
	if (others_req[i].count) {
	    ADIOI_Free(others_req[i].offsets);
	    ADIOI_Free(others_req[i].lens);
	    ADIOI_Free(others_req[i].mem_ptrs);
	}
    }
    ADIOI_Free(others_req);

    ADIOI_Free(buf_idx);
    ADIOI_Free(offset_list);
    ADIOI_Free(len_list);
    ADIOI_Free(st_offsets);
    ADIOI_Free(end_offsets);
    ADIOI_Free(fd_start);
    ADIOI_Free(fd_end);

#ifdef HAVE_STATUS_SET_BYTES
    MPI_Type_size(datatype, &size);
    bufsize = size * count;
    MPIR_Status_set_bytes(status, datatype, bufsize);
/* This is a temporary way of filling in status. The right way is to 
   keep track of how much data was actually read and placed in buf 
   during collective I/O. */
#endif

    fd->fp_sys_posn = -1;   /* set it to null. */
}


void ADIOI_Calc_my_off_len(ADIO_File fd, int bufcount, MPI_Datatype
			    datatype, int file_ptr_type, ADIO_Offset
			    offset, ADIO_Offset **offset_list_ptr, int
			    **len_list_ptr, ADIO_Offset *start_offset_ptr,
			    ADIO_Offset *end_offset_ptr, int
			   *contig_access_count_ptr)
{
    int filetype_size, buftype_size, etype_size;
    int i, j, k, frd_size=0, old_frd_size=0, st_index=0;
    int n_filetypes, etype_in_filetype;
    ADIO_Offset abs_off_in_filetype=0;
    int bufsize, sum, n_etypes_in_filetype, size_in_filetype;
    int contig_access_count, *len_list, flag, filetype_is_contig;
    MPI_Aint filetype_extent, filetype_lb;
    ADIOI_Flatlist_node *flat_file;
    ADIO_Offset *offset_list, off, end_offset=0, disp;
    
/* For this process's request, calculate the list of offsets and
   lengths in the file and determine the start and end offsets. */

    ADIOI_Datatype_iscontig(fd->filetype, &filetype_is_contig);

    MPI_Type_size(fd->filetype, &filetype_size);
    MPI_Type_extent(fd->filetype, &filetype_extent);
    MPI_Type_lb(fd->filetype, &filetype_lb);
    MPI_Type_size(datatype, &buftype_size);
    etype_size = fd->etype_size;

    if (filetype_is_contig) {
	*contig_access_count_ptr = 1;        
	*offset_list_ptr = (ADIO_Offset *) ADIOI_Malloc(2*sizeof(ADIO_Offset));
	*len_list_ptr = (int *) ADIOI_Malloc(2*sizeof(int));
        /* 2 is for consistency. everywhere I malloc one more than needed */

	offset_list = *offset_list_ptr;
	len_list = *len_list_ptr;
        offset_list[0] = (file_ptr_type == ADIO_INDIVIDUAL) ? fd->fp_ind : 
                 fd->disp + etype_size * offset;
	len_list[0] = bufcount * buftype_size;
	*start_offset_ptr = offset_list[0];
	*end_offset_ptr = offset_list[0] + len_list[0] - 1;

	/* update file pointer */
	if (file_ptr_type == ADIO_INDIVIDUAL) fd->fp_ind = *end_offset_ptr + 1;
    }

    else {

       /* First calculate what size of offset_list and len_list to allocate */
   
       /* filetype already flattened in ADIO_Open or ADIO_Fcntl */
	flat_file = ADIOI_Flatlist;
	while (flat_file->type != fd->filetype) flat_file = flat_file->next;
	disp = fd->disp;

	if (file_ptr_type == ADIO_INDIVIDUAL) {
	    offset = fd->fp_ind; /* in bytes */
	    n_filetypes = -1;
	    flag = 0;
	    while (!flag) {
		n_filetypes++;
		for (i=0; i<flat_file->count; i++) {
		    if (disp + flat_file->indices[i] + 
			(ADIO_Offset) n_filetypes*filetype_extent + flat_file->blocklens[i] 
                            >= offset) {
			st_index = i;
			frd_size = (int) (disp + flat_file->indices[i] + 
			    (ADIO_Offset) n_filetypes*filetype_extent
			        + flat_file->blocklens[i] - offset);
			flag = 1;
			break;
		    }
		}
	    }
	}
	else {
	    n_etypes_in_filetype = filetype_size/etype_size;
	    n_filetypes = (int) (offset / n_etypes_in_filetype);
	    etype_in_filetype = (int) (offset % n_etypes_in_filetype);
	    size_in_filetype = etype_in_filetype * etype_size;
 
	    sum = 0;
	    for (i=0; i<flat_file->count; i++) {
		sum += flat_file->blocklens[i];
		if (sum > size_in_filetype) {
		    st_index = i;
		    frd_size = sum - size_in_filetype;
		    abs_off_in_filetype = flat_file->indices[i] +
			size_in_filetype - (sum - flat_file->blocklens[i]);
		    break;
		}
	    }

	    /* abs. offset in bytes in the file */
	    offset = disp + (ADIO_Offset) n_filetypes*filetype_extent + abs_off_in_filetype;
	}

         /* calculate how much space to allocate for offset_list, len_list */

	old_frd_size = frd_size;
	contig_access_count = i = 0;
	j = st_index;
	bufsize = buftype_size * bufcount;
	frd_size = ADIOI_MIN(frd_size, bufsize);
	while (i < bufsize) {
	    if (frd_size) contig_access_count++;
	    i += frd_size;
	    j = (j + 1) % flat_file->count;
	    frd_size = ADIOI_MIN(flat_file->blocklens[j], bufsize-i);
	}

        /* allocate space for offset_list and len_list */

	*offset_list_ptr = (ADIO_Offset *)
	         ADIOI_Malloc((contig_access_count+1)*sizeof(ADIO_Offset));  
	*len_list_ptr = (int *) ADIOI_Malloc((contig_access_count+1)*sizeof(int));
        /* +1 to avoid a 0-size malloc */

	offset_list = *offset_list_ptr;
	len_list = *len_list_ptr;

      /* find start offset, end offset, and fill in offset_list and len_list */

	*start_offset_ptr = offset; /* calculated above */

	i = k = 0;
	j = st_index;
	off = offset;
	frd_size = ADIOI_MIN(old_frd_size, bufsize);
	while (i < bufsize) {
	    if (frd_size) {
		offset_list[k] = off;
		len_list[k] = frd_size;
		k++;
	    }
	    i += frd_size;
	    end_offset = off + frd_size - 1;

     /* Note: end_offset points to the last byte-offset that will be accessed.
         e.g., if start_offset=0 and 100 bytes to be read, end_offset=99*/

	    if (off + frd_size < disp + flat_file->indices[j] +
		   flat_file->blocklens[j] + (ADIO_Offset) n_filetypes*filetype_extent)
		off += frd_size;
	    /* did not reach end of contiguous block in filetype.
	       no more I/O needed. off is incremented by frd_size. */
	    else {
		if (j < (flat_file->count - 1)) j++;
		else {
		    j = 0;
		    n_filetypes++;
		}
		off = disp + flat_file->indices[j] + (ADIO_Offset) n_filetypes*filetype_extent;
		frd_size = ADIOI_MIN(flat_file->blocklens[j], bufsize-i);
	    }
	}

	/* update file pointer */
	if (file_ptr_type == ADIO_INDIVIDUAL) fd->fp_ind = off;

	*contig_access_count_ptr = contig_access_count;
	*end_offset_ptr = end_offset;
    }
}



void ADIOI_Calc_file_domains(ADIO_Offset *st_offsets, ADIO_Offset
			     *end_offsets, int nprocs, int nprocs_for_coll,
			     ADIO_Offset *min_st_offset_ptr,
			     ADIO_Offset **fd_start_ptr, ADIO_Offset 
			     **fd_end_ptr, ADIO_Offset *fd_size_ptr)
{
/* Divide the I/O workload among "nprocs_for_coll" processes. This is
   done by (logically) dividing the file into file domains (FDs); each
   process may directly access only its own file domain. */

    ADIO_Offset min_st_offset, max_end_offset, *fd_start, *fd_end, fd_size;
    int i;

/* find min of start offsets and max of end offsets of all processes */

    min_st_offset = st_offsets[0];
    max_end_offset = end_offsets[0];

    for (i=1; i<nprocs; i++) {
	min_st_offset = ADIOI_MIN(min_st_offset, st_offsets[i]);
	max_end_offset = ADIOI_MAX(max_end_offset, end_offsets[i]);
    }

/* determine the "file domain (FD)" of each process, i.e., the portion of
   the file that will be "owned" by each process */

/* partition the total file access range equally among nprocs_for_coll
   processes */ 
    fd_size = ((max_end_offset - min_st_offset + 1) + nprocs_for_coll -
	       1)/nprocs_for_coll; 
    /* ceiling division as in HPF block distribution */

    *fd_start_ptr = (ADIO_Offset *)
	ADIOI_Malloc(nprocs_for_coll*sizeof(ADIO_Offset)); 
    *fd_end_ptr = (ADIO_Offset *)
	ADIOI_Malloc(nprocs_for_coll*sizeof(ADIO_Offset)); 

    fd_start = *fd_start_ptr;
    fd_end = *fd_end_ptr;

    fd_start[0] = min_st_offset;
    fd_end[0] = min_st_offset + fd_size - 1;

    for (i=1; i<nprocs_for_coll; i++) {
	fd_start[i] = fd_end[i-1] + 1;
	fd_end[i] = fd_start[i] + fd_size - 1;
    }

/* take care of cases in which the total file access range is not
   divisible by the number of processes. In such cases, the last
   process, or the last few processes, may have unequal load (even 0).
   For example, a range of 97 divided among 16 processes.
   Note that the division is ceiling division. */

    for (i=0; i<nprocs_for_coll; i++) {
	if (fd_start[i] > max_end_offset)
	    fd_start[i] = fd_end[i] = -1;
	if (fd_end[i] > max_end_offset)
	    fd_end[i] = max_end_offset;
    }

    *fd_size_ptr = fd_size;
    *min_st_offset_ptr = min_st_offset;
}



void ADIOI_Calc_my_req(ADIO_Offset *offset_list, int *len_list, int
			    contig_access_count, ADIO_Offset 
			    min_st_offset, ADIO_Offset *fd_start,
			    ADIO_Offset *fd_end, ADIO_Offset fd_size,
                            int nprocs, int nprocs_for_coll, 
                            int *count_my_req_procs_ptr,
			    int **count_my_req_per_proc_ptr,
			    ADIOI_Access **my_req_ptr,
			    int **buf_idx_ptr)
{
/* calculate what portions of the access requests of this process are
   located in the file domains of other processes */

    int *count_my_req_per_proc, count_my_req_procs, *buf_idx;
    int i, l, proc, len, rem_len, curr_idx;
    ADIO_Offset off;
    ADIOI_Access *my_req;

    *count_my_req_per_proc_ptr = (int *) ADIOI_Calloc(nprocs,sizeof(int)); 
    count_my_req_per_proc = *count_my_req_per_proc_ptr;
/* count_my_req_per_proc[i] gives the no. of contig. requests of this
   process in process i's file domain. calloc initializes to zero.
   I'm allocating memory of size nprocs, so that I can do an 
   MPI_Alltoall later on.*/

    buf_idx = (int *) ADIOI_Malloc(nprocs_for_coll*sizeof(int));
/* buf_idx is relevant only if buftype_is_contig.
   buf_idx[i] gives the index into user_buf where data received
   from proc. i should be placed. This allows receives to be done
   without extra buffer. This can't be done if buftype is not contig. */
   
/* initialize buf_idx to -1 */
    for (i=0; i<nprocs_for_coll; i++) buf_idx[i] = -1;

/* one pass just to calculate how much space to allocate for
   my_req */
    for (i=0; i<contig_access_count; i++) { 

/* proc_no = CD(offset_list[i]-min_st_offset+1, fd_size) - 1 */
/* CD = ceiling division. CD(j,k) = (j+k-1)/k */

	proc = (int) ((offset_list[i] - min_st_offset + fd_size)/fd_size - 1);
        /* sanity check */
	if (proc >= nprocs_for_coll) {
	    FPRINTF(stderr, "Error: proc >= nprocs_for_coll, file %s, line %d\n", __FILE__, __LINE__);
	    MPI_Abort(MPI_COMM_WORLD, 1);
	}

	off = offset_list[i];
	len = (int) (((off+len_list[i]-1) <= fd_end[proc]) ? len_list[i] : 
	                           (fd_end[proc] - off + 1));
	rem_len = len_list[i] - len;
	count_my_req_per_proc[proc]++;

	while (rem_len != 0) {
	    proc++;
	    off = fd_start[proc];
	    len = (int) (((off+rem_len-1) <= fd_end[proc]) ? rem_len : 
	                           (fd_end[proc] - off + 1));
	    rem_len -= len;
	    count_my_req_per_proc[proc]++;
	}
    }

/* now allocate space for my_req, offset, and len */

    *my_req_ptr = (ADIOI_Access *)
	ADIOI_Malloc(nprocs_for_coll*sizeof(ADIOI_Access)); 
    my_req = *my_req_ptr;

    count_my_req_procs = 0;
    for (i=0; i<nprocs_for_coll; i++) {
	if (count_my_req_per_proc[i]) {
	    my_req[i].offsets = (ADIO_Offset *)
		ADIOI_Malloc(count_my_req_per_proc[i] * sizeof(ADIO_Offset));
	    my_req[i].lens = (int *)
		ADIOI_Malloc(count_my_req_per_proc[i] * sizeof(int));
	    count_my_req_procs++;
	}	    
	my_req[i].count = 0;  /* will be incremented where needed
				      later */
    }

/* now fill in my_req */
    curr_idx = 0;
    for (i=0; i<contig_access_count; i++) { 

	/* for each separate contiguous request from this process */

	proc = (int) ((offset_list[i] - min_st_offset + fd_size)/fd_size - 1);
	if (buf_idx[proc] == -1) buf_idx[proc] = curr_idx;

	l = my_req[proc].count;
	off = offset_list[i];
	len = (int) (((off+len_list[i]-1) <= fd_end[proc]) ? len_list[i] : 
	                           (fd_end[proc] - off + 1));
	curr_idx += len;
	/* the length may go beyond proc's file domain */
	rem_len = len_list[i] - len;

	/* store the proc, offset, and len information in an array
         of structures, my_req. Each structure contains the 
         offsets and lengths located in that process's FD, 
	 and the associated count. */

	my_req[proc].offsets[l] = off;
	my_req[proc].lens[l] = len;
	my_req[proc].count++;

	/* this request may span the file domains of more than one
	   process */
	while (rem_len != 0) {
	    proc++;
	    if (buf_idx[proc] == -1) buf_idx[proc] = curr_idx;
	    l = my_req[proc].count;
	    off = fd_start[proc];
	    len = (int) (((off+rem_len-1) <= fd_end[proc]) ? rem_len : 
	                           (fd_end[proc] - off + 1));
	    curr_idx += len;
	    rem_len -= len;
	    my_req[proc].offsets[l] = off;
	    my_req[proc].lens[l] = len;
	    my_req[proc].count++;
	}
    }
    *count_my_req_procs_ptr = count_my_req_procs;
    *buf_idx_ptr = buf_idx;
}



void ADIOI_Calc_others_req(ADIO_File fd, int count_my_req_procs, 
				int *count_my_req_per_proc,
				ADIOI_Access *my_req, 
				int nprocs, int myrank, int nprocs_for_coll, 
				int *count_others_req_procs_ptr,
				ADIOI_Access **others_req_ptr)  
{
/* determine what requests of other processes lie in this process's
   file domain */

/* count_others_req_procs = number of processes whose requests lie in
   this process's file domain (including this process itself) 
   count_others_req_per_proc[i] indicates how many separate contiguous
   requests of proc. i lie in this process's file domain. */

    int *count_others_req_per_proc, count_others_req_procs;
    int i, j;
    MPI_Request *send_requests, *recv_requests;
    MPI_Status *statuses;
    ADIOI_Access *others_req;

/* first find out how much to send/recv and from/to whom */

    count_others_req_per_proc = (int *) ADIOI_Malloc(nprocs*sizeof(int));

    MPI_Alltoall(count_my_req_per_proc, 1, MPI_INT,
		 count_others_req_per_proc, 1, MPI_INT, fd->comm);

    *others_req_ptr = (ADIOI_Access *)
	ADIOI_Malloc(nprocs*sizeof(ADIOI_Access)); 
    others_req = *others_req_ptr;

    count_others_req_procs = 0;
    for (i=0; i<nprocs; i++) {
	if (count_others_req_per_proc[i]) {
	    others_req[i].count = count_others_req_per_proc[i];
	    others_req[i].offsets = (ADIO_Offset *)
		ADIOI_Malloc(count_others_req_per_proc[i]*sizeof(ADIO_Offset));
	    others_req[i].lens = (int *)
		ADIOI_Malloc(count_others_req_per_proc[i]*sizeof(int)); 
	    others_req[i].mem_ptrs = (MPI_Aint *)
		ADIOI_Malloc(count_others_req_per_proc[i]*sizeof(MPI_Aint)); 
	    count_others_req_procs++;
	}
	else others_req[i].count = 0;
    }
    
/* now send the calculated offsets and lengths to respective processes */

    send_requests = (MPI_Request *)
	ADIOI_Malloc(2*(count_my_req_procs+1)*sizeof(MPI_Request)); 
    recv_requests = (MPI_Request *)
	ADIOI_Malloc(2*(count_others_req_procs+1)*sizeof(MPI_Request)); 
/* +1 to avoid a 0-size malloc */

    j = 0;
    for (i=0; i<nprocs; i++) {
	if (others_req[i].count) {
	    MPI_Irecv(others_req[i].offsets, others_req[i].count, 
                      ADIO_OFFSET, i, i+myrank, fd->comm, &recv_requests[j]);
	    j++;
	    MPI_Irecv(others_req[i].lens, others_req[i].count, 
                      MPI_INT, i, i+myrank+1, fd->comm, &recv_requests[j]);
	    j++;
	}
    }

    j = 0;
    for (i=0; i<nprocs_for_coll; i++) {
	if (my_req[i].count) {
	    MPI_Isend(my_req[i].offsets, my_req[i].count, 
                      ADIO_OFFSET, i, i+myrank, fd->comm, &send_requests[j]);
	    j++;
	    MPI_Isend(my_req[i].lens, my_req[i].count, 
                      MPI_INT, i, i+myrank+1, fd->comm, &send_requests[j]);
	    j++;
	}
    }

    statuses = (MPI_Status *) ADIOI_Malloc((1 + 2* \
                   ADIOI_MAX(count_my_req_procs,count_others_req_procs)) * \
                       sizeof(MPI_Status));
/* +1 to avoid a 0-size malloc */

    MPI_Waitall(2*count_my_req_procs, send_requests, statuses);
    MPI_Waitall(2*count_others_req_procs, recv_requests, statuses);

    ADIOI_Free(send_requests);
    ADIOI_Free(recv_requests);	    
    ADIOI_Free(statuses);
    ADIOI_Free(count_others_req_per_proc);

    *count_others_req_procs_ptr = count_others_req_procs;
}


static void ADIOI_Read_and_exch(ADIO_File fd, void *buf, MPI_Datatype
			 datatype, int nprocs, int
			 nprocs_for_coll, int myrank, ADIOI_Access
			 *others_req, ADIO_Offset *offset_list,
			 int *len_list, int contig_access_count, ADIO_Offset
                         min_st_offset, ADIO_Offset fd_size,
			 ADIO_Offset *fd_start, ADIO_Offset *fd_end,
                         int *buf_idx, int *error_code)
{
/* Read in sizes of no more than coll_bufsize, an info parameter.
   Send data to appropriate processes. 
   Place recd. data in user buf.
   The idea is to reduce the amount of extra memory required for
   collective I/O. If all data were read all at once, which is much
   easier, it would require temp space more than the size of user_buf,
   which is often unacceptable. For example, to read a distributed
   array from a file, where each local array is 8Mbytes, requiring
   at least another 8Mbytes of temp space is unacceptable. */

    int i, j, m, size, ntimes, max_ntimes, buftype_is_contig;
    ADIO_Offset st_loc=-1, end_loc=-1, off, done, real_off, req_off;
    char *read_buf, *tmp_buf;
    int *curr_offlen_ptr, *count, *send_size, *recv_size;
    int *partial_send, *recd_from_proc, *start_pos, for_next_iter;
    int *recv_buf_idx, *curr_from_proc, *done_from_proc;
    int real_size, req_len, flag, for_curr_iter, rank;
    MPI_Status status;
    ADIOI_Flatlist_node *flat_buf=NULL;
    MPI_Aint buftype_extent;
    int info_flag, coll_bufsize;
    char *value;

    *error_code = MPI_SUCCESS;  /* changed below if error */
    /* only I/O errors are currently reported */
    
/* calculate the number of reads of size coll_bufsize
   to be done by each process and the max among all processes.
   That gives the no. of communication phases as well.
   coll_bufsize is obtained from the info object. */

    value = (char *) ADIOI_Malloc((MPI_MAX_INFO_VAL+1)*sizeof(char));
    MPI_Info_get(fd->info, "cb_buffer_size", MPI_MAX_INFO_VAL, value, 
		 &info_flag);
    coll_bufsize = atoi(value);
    ADIOI_Free(value);

    for (i=0; i<nprocs; i++) {
	if (others_req[i].count) {
	    st_loc = others_req[i].offsets[0];
	    end_loc = others_req[i].offsets[0];
	    break;
	}
    }

    for (i=0; i<nprocs; i++)
	for (j=0; j<others_req[i].count; j++) {
	    st_loc = ADIOI_MIN(st_loc, others_req[i].offsets[j]);
	    end_loc = ADIOI_MAX(end_loc, (others_req[i].offsets[j]
				       + others_req[i].lens[j] - 1));
	}


    /* ntimes=ceiling_div(end_loc - st_loc + 1, coll_bufsize)*/

    ntimes = (int) ((end_loc - st_loc + coll_bufsize)/coll_bufsize);

    if ((st_loc==-1) && (end_loc==-1)) ntimes = 0; /* this process does no I/O. */

    MPI_Allreduce(&ntimes, &max_ntimes, 1, MPI_INT, MPI_MAX, fd->comm); 

    if (ntimes) read_buf = (char *) ADIOI_Malloc(coll_bufsize);

    curr_offlen_ptr = (int *) ADIOI_Calloc(nprocs, sizeof(int)); 
    /* its use is explained below. calloc initializes to 0. */

    count = (int *) ADIOI_Malloc(nprocs*sizeof(int));
    /* to store count of how many off-len pairs per proc are satisfied
       in an iteration. */

    partial_send = (int *) ADIOI_Calloc(nprocs, sizeof(int));
    /* if only a portion of the last off-len pair is sent to a process 
       in a particular iteration, the length sent is stored here.
       calloc initializes to 0. */

    send_size = (int *) ADIOI_Malloc(nprocs*sizeof(int));
    /* total size of data to be sent to each proc. in an iteration */

    recv_size = (int *) ADIOI_Malloc(nprocs*sizeof(int));
    /* total size of data to be recd. from each proc. in an iteration.
       Of size nprocs so that I can use MPI_Alltoall later. */

    recd_from_proc = (int *) ADIOI_Calloc(nprocs_for_coll, sizeof(int));
    /* amount of data recd. so far from each proc. Used in
       ADIOI_Fill_user_buffer. initialized to 0 here. */

    recv_buf_idx = (int *) ADIOI_Malloc(nprocs_for_coll*sizeof(int));
    curr_from_proc = (int *) ADIOI_Malloc(nprocs_for_coll*sizeof(int));
    done_from_proc = (int *) ADIOI_Malloc(nprocs_for_coll*sizeof(int));
    /* Above three are used in ADIOI_Fill_user_buffer*/

    start_pos = (int *) ADIOI_Malloc(nprocs*sizeof(int));
    /* used to store the starting value of curr_offlen_ptr[i] in 
       this iteration */

    ADIOI_Datatype_iscontig(datatype, &buftype_is_contig);
    if (!buftype_is_contig) {
	ADIOI_Flatten_datatype(datatype);
	flat_buf = ADIOI_Flatlist;
        while (flat_buf->type != datatype) flat_buf = flat_buf->next;
    }
    MPI_Type_extent(datatype, &buftype_extent);

    done = 0;
    off = st_loc;
    for_curr_iter = for_next_iter = 0;

    MPI_Comm_rank(fd->comm, &rank);

#ifdef PROFILE
        MPE_Log_event(14, 0, "end computation");
#endif

    for (m=0; m<ntimes; m++) {
       /* read buf of size coll_bufsize (or less) */
       /* go through all others_req and check if any are satisfied
          by the current read */

       /* since MPI guarantees that displacements in filetypes are in 
          monotonically nondecreasing order, I can maintain a pointer
	  (curr_offlen_ptr) to 
          current off-len pair for each process in others_req and scan
          further only from there. There is still a problem of filetypes
          such as:  (1, 2, 3 are not process nos. They are just numbers for
          three chunks of data, specified by a filetype.)

                   1  -------!--
                   2    -----!----
                   3       --!-----

          where ! indicates where the current read_size limitation cuts 
          through the filetype.  I resolve this by reading up to !, but
          filling the communication buffer only for 1. I copy the portion
          left over for 2 into a tmp_buf for use in the next
	  iteration. i.e., 2 and 3 will be satisfied in the next
	  iteration. This simplifies filling in the user's buf at the
	  other end, as only one off-len pair with incomplete data
	  will be sent. I also don't need to send the individual
	  offsets and lens along with the data, as the data is being
	  sent in a particular order. */ 

          /* off = start offset in the file for the data actually read in 
                   this iteration 
             size = size of data read corresponding to off
             real_off = off minus whatever data was retained in memory from
                  previous iteration for cases like 2, 3 illustrated above
             real_size = size plus the extra corresponding to real_off
             req_off = off in file for a particular contiguous request 
                       minus what was satisfied in previous iteration
             req_size = size corresponding to req_off */

#ifdef PROFILE
        MPE_Log_event(13, 0, "start computation");
#endif
	size = (int) (ADIOI_MIN(coll_bufsize, end_loc-st_loc+1-done)); 
	real_off = off - for_curr_iter;
	real_size = size + for_curr_iter;

	for (i=0; i<nprocs; i++) count[i] = send_size[i] = 0;
	for_next_iter = 0;

	for (i=0; i<nprocs; i++) {
	    /* FPRINTF(stderr, "rank %d, i %d, others_count %d\n", rank, i, others_req[i].count); */
	    if (others_req[i].count) {
		start_pos[i] = curr_offlen_ptr[i];
		for (j=curr_offlen_ptr[i]; j<others_req[i].count;
		     j++) {
		    if (partial_send[i]) {
			/* this request may have been partially
			   satisfied in the previous iteration. */
			req_off = others_req[i].offsets[j] +
			    partial_send[i]; 
                        req_len = others_req[i].lens[j] -
			    partial_send[i];
			partial_send[i] = 0;
			/* modify the off-len pair to reflect this change */
			others_req[i].offsets[j] = req_off;
			others_req[i].lens[j] = req_len;
		    }
		    else {
			req_off = others_req[i].offsets[j];
                        req_len = others_req[i].lens[j];
		    }
		    if (req_off < real_off + real_size) {
			count[i]++;
			MPI_Address(read_buf+req_off-real_off, 
                               &(others_req[i].mem_ptrs[j]));
			send_size[i] += ADIOI_MIN(real_off + real_size - 
						  req_off, req_len);

			if (real_off+real_size-req_off < req_len) {
			    partial_send[i] = (int) (real_off+real_size-req_off);
			    if ((j+1 < others_req[i].count) && 
                                 (others_req[i].offsets[j+1] < 
                                     real_off+real_size)) { 
				/* this is the case illustrated in the
				   figure above. */
				for_next_iter = (int) (ADIOI_MAX(for_next_iter,
					  real_off + real_size - 
                                             others_req[i].offsets[j+1])); 
				/* max because it must cover requests 
				   from different processes */
			    }
			    break;
			}
		    }
		    else break;
		}
		curr_offlen_ptr[i] = j;
	    }
	}

	flag = 0;
	for (i=0; i<nprocs; i++)
	    if (count[i]) flag = 1;

#ifdef PROFILE
        MPE_Log_event(14, 0, "end computation");
#endif
	if (flag) {
	    ADIO_ReadContig(fd, read_buf+for_curr_iter, size, MPI_BYTE,
			    ADIO_EXPLICIT_OFFSET, off, &status, error_code);
	    if (*error_code != MPI_SUCCESS) return;
	}
	
	for_curr_iter = for_next_iter;
	
#ifdef PROFILE
        MPE_Log_event(7, 0, "start communication");
#endif
	ADIOI_R_Exchange_data(fd, buf, flat_buf, offset_list, len_list,
			    send_size, recv_size, count, 
       			    start_pos, partial_send, recd_from_proc, nprocs,
			    nprocs_for_coll, myrank, 
			    buftype_is_contig, contig_access_count,
			    min_st_offset, fd_size, fd_start, fd_end,
			    others_req, recv_buf_idx, 
                            curr_from_proc, done_from_proc, m,
                            buftype_extent, buf_idx); 

#ifdef PROFILE
        MPE_Log_event(8, 0, "end communication");
#endif

	if (for_next_iter) {
	    tmp_buf = (char *) ADIOI_Malloc(for_next_iter);
	    memcpy(tmp_buf, read_buf+real_size-for_next_iter, for_next_iter);
	    ADIOI_Free(read_buf);
	    read_buf = (char *) ADIOI_Malloc(for_next_iter+coll_bufsize);
	    memcpy(read_buf, tmp_buf, for_next_iter);
	    ADIOI_Free(tmp_buf);
	}

	off += size;
	done += size;
    }

    for (i=0; i<nprocs; i++) count[i] = send_size[i] = 0;
#ifdef PROFILE
        MPE_Log_event(7, 0, "start communication");
#endif
    for (m=ntimes; m<max_ntimes; m++) 
/* nothing to send, but check for recv. */
	ADIOI_R_Exchange_data(fd, buf, flat_buf, offset_list, len_list,
			    send_size, recv_size, count, 
			    start_pos, partial_send, recd_from_proc, nprocs,
			    nprocs_for_coll, myrank, 
			    buftype_is_contig, contig_access_count,
			    min_st_offset, fd_size, fd_start, fd_end,
			    others_req, recv_buf_idx, 
                            curr_from_proc, done_from_proc, m,
                            buftype_extent, buf_idx); 
#ifdef PROFILE
        MPE_Log_event(8, 0, "end communication");
#endif

    if (ntimes) ADIOI_Free(read_buf);
    ADIOI_Free(curr_offlen_ptr);
    ADIOI_Free(count);
    ADIOI_Free(partial_send);
    ADIOI_Free(send_size);
    ADIOI_Free(recv_size);
    ADIOI_Free(recd_from_proc);
    ADIOI_Free(start_pos);
    ADIOI_Free(recv_buf_idx);
    ADIOI_Free(curr_from_proc);
    ADIOI_Free(done_from_proc);
}


static void ADIOI_R_Exchange_data(ADIO_File fd, void *buf, ADIOI_Flatlist_node
			 *flat_buf, ADIO_Offset *offset_list, int
                         *len_list, int *send_size, int *recv_size,
			 int *count, int *start_pos, int *partial_send, 
			 int *recd_from_proc, int nprocs, 
			 int nprocs_for_coll, int myrank, int
			 buftype_is_contig, int contig_access_count,
			 ADIO_Offset min_st_offset, ADIO_Offset fd_size,
			 ADIO_Offset *fd_start, ADIO_Offset *fd_end, 
			 ADIOI_Access *others_req, 
                         int *recv_buf_idx, int *curr_from_proc,
                         int *done_from_proc, int iter, 
			 MPI_Aint buftype_extent, int *buf_idx)
{
    int i, j, k, tmp, nprocs_recv, nprocs_send;
    char **recv_buf; 
    MPI_Request *requests;
    MPI_Datatype send_type;
    MPI_Status *statuses;

/* exchange send_size info so that each process knows how much to
   receive from whom and how much memory to allocate. */

    MPI_Alltoall(send_size, 1, MPI_INT, recv_size, 1, MPI_INT, fd->comm);

    nprocs_recv = 0;
    for (i=0; i<nprocs_for_coll; i++) if (recv_size[i]) nprocs_recv++;

    nprocs_send = 0;
    for (i=0; i<nprocs; i++) if (send_size[i]) nprocs_send++;

    requests = (MPI_Request *)
	ADIOI_Malloc((nprocs_send+nprocs_recv+1)*sizeof(MPI_Request));
/* +1 to avoid a 0-size malloc */

/* post recvs. if buftype_is_contig, data can be directly recd. into
   user buf at location given by buf_idx. else use recv_buf. */

    if (buftype_is_contig) {
	j = 0;
	for (i=0; i<nprocs_for_coll; i++) 
	    if (recv_size[i]) {
		MPI_Irecv(((char *) buf) + buf_idx[i], recv_size[i], 
		  MPI_BYTE, i, myrank+i+100*iter, fd->comm, requests+j);
		j++;
		buf_idx[i] += recv_size[i];
	    }
    }
    else {
/* allocate memory for recv_buf and post receives */
	recv_buf = (char **) ADIOI_Malloc(nprocs_for_coll*sizeof(char*));
	for (i=0; i<nprocs_for_coll; i++) 
	    if (recv_size[i]) recv_buf[i] = 
                                  (char *) ADIOI_Malloc(recv_size[i]);

	    j = 0;
	    for (i=0; i<nprocs_for_coll; i++) 
		if (recv_size[i]) {
		    MPI_Irecv(recv_buf[i], recv_size[i], MPI_BYTE, i, 
			      myrank+i+100*iter, fd->comm, requests+j);
		    j++;
		    /* FPRINTF(stderr, "node %d, recv_size %d, tag %d \n", myrank, recv_size[i], myrank+i+100*iter); */
		}
    }

/* create derived datatypes and send data */

    j = 0;
    for (i=0; i<nprocs; i++) {
	if (send_size[i]) {
/* take care if the last off-len pair is a partial send */
	    if (partial_send[i]) {
		k = start_pos[i] + count[i] - 1;
		tmp = others_req[i].lens[k];
		others_req[i].lens[k] = partial_send[i];
	    }
	    MPI_Type_hindexed(count[i], 
                 &(others_req[i].lens[start_pos[i]]),
	            &(others_req[i].mem_ptrs[start_pos[i]]), 
			 MPI_BYTE, &send_type);
	    /* absolute displacement; use MPI_BOTTOM in send */
	    MPI_Type_commit(&send_type);
	    MPI_Isend(MPI_BOTTOM, 1, send_type, i, myrank+i+100*iter,
		      fd->comm, requests+nprocs_recv+j);
	    MPI_Type_free(&send_type);
	    if (partial_send[i]) others_req[i].lens[k] = tmp;
	    j++;
	}
    }

    statuses = (MPI_Status *) ADIOI_Malloc((nprocs_send+nprocs_recv+1) * \
                                     sizeof(MPI_Status)); 
     /* +1 to avoid a 0-size malloc */

    if (!buftype_is_contig && nprocs_recv) 
	ADIOI_Fill_user_buffer(buf, flat_buf, recv_buf,
			       offset_list, len_list, recv_size, 
			       requests, statuses, recd_from_proc, 
			       nprocs_for_coll, contig_access_count,
			       min_st_offset, fd_size, fd_start, fd_end,
			       recv_buf_idx, curr_from_proc, done_from_proc,
			       buftype_extent);

    if (buftype_is_contig) {
#ifdef NEEDS_MPI_TEST
	j = 0;
	while (!j) MPI_Testall(nprocs_recv, requests, &j, statuses);
#else
	MPI_Waitall(nprocs_recv, requests, statuses);
#endif
    }

    /* wait on the sends*/
    MPI_Waitall(nprocs_send, requests+nprocs_recv, statuses+nprocs_recv);

    ADIOI_Free(statuses);
    ADIOI_Free(requests);

    if (!buftype_is_contig) {
	for (i=0; i<nprocs_for_coll; i++) 
	    if (recv_size[i]) ADIOI_Free(recv_buf[i]);
	ADIOI_Free(recv_buf);
    }
}



#define ADIOI_BUF_INCR \
{ \
    while (buf_incr) { \
	size_in_buf = ADIOI_MIN(buf_incr, flat_buf_sz); \
	user_buf_idx += size_in_buf; \
	flat_buf_sz -= size_in_buf; \
	if (!flat_buf_sz) { \
            if (flat_buf_idx < (flat_buf->count - 1)) flat_buf_idx++; \
            else { \
                flat_buf_idx = 0; \
                n_buftypes++; \
            } \
            user_buf_idx = flat_buf->indices[flat_buf_idx] + \
                              n_buftypes*buftype_extent; \
	    flat_buf_sz = flat_buf->blocklens[flat_buf_idx]; \
	} \
	buf_incr -= size_in_buf; \
    } \
}


#define ADIOI_BUF_COPY \
{ \
    while (size) { \
	size_in_buf = ADIOI_MIN(size, flat_buf_sz); \
	memcpy(((char *) buf) + user_buf_idx, \
	       &(recv_buf[p][recv_buf_idx[p]]), size_in_buf); \
	recv_buf_idx[p] += size_in_buf; \
	user_buf_idx += size_in_buf; \
	flat_buf_sz -= size_in_buf; \
	if (!flat_buf_sz) { \
            if (flat_buf_idx < (flat_buf->count - 1)) flat_buf_idx++; \
            else { \
                flat_buf_idx = 0; \
                n_buftypes++; \
            } \
            user_buf_idx = flat_buf->indices[flat_buf_idx] + \
                              n_buftypes*buftype_extent; \
	    flat_buf_sz = flat_buf->blocklens[flat_buf_idx]; \
	} \
	size -= size_in_buf; \
	buf_incr -= size_in_buf; \
    } \
    ADIOI_BUF_INCR \
}


static void ADIOI_Fill_user_buffer(void *buf, ADIOI_Flatlist_node
                           *flat_buf, char **recv_buf, ADIO_Offset 
                           *offset_list, int *len_list, int *recv_size, 
                           MPI_Request *requests, MPI_Status *statuses,
                           int *recd_from_proc, int nprocs_for_coll, 
                           int contig_access_count, ADIO_Offset min_st_offset, 
                           ADIO_Offset fd_size, ADIO_Offset *fd_start, 
                           ADIO_Offset *fd_end, int *recv_buf_idx, 
                           int *curr_from_proc, int *done_from_proc,
                           MPI_Aint buftype_extent)
{
/* this function is only called if buftype is not contig */

    int i, p, flat_buf_idx, rem_len, size, buf_incr;
    int flat_buf_sz, len, size_in_buf, jj, n_buftypes;
    ADIO_Offset off, user_buf_idx;
#ifdef NEEDS_MPI_TEST
    int j;
#endif

/*  curr_from_proc[p] = amount of data recd from proc. p that has already
                        been accounted for so far
    done_from_proc[p] = amount of data already recd from proc. p and 
                        filled into user buffer in previous iterations
    user_buf_idx = current location in user buffer 
    recv_buf_idx[p] = current location in recv_buf of proc. p  */

    for (i=0; i<nprocs_for_coll; i++) {
	recv_buf_idx[i] = curr_from_proc[i] = 0;
	done_from_proc[i] = recd_from_proc[i];
    }
    jj = 0;

    user_buf_idx = flat_buf->indices[0];
    flat_buf_idx = 0;
    n_buftypes = 0;
    flat_buf_sz = flat_buf->blocklens[0];

    /* flat_buf_idx = current index into flattened buftype
       flat_buf_sz = size of current contiguous component in 
                flattened buf */

    for (i=0; i<contig_access_count; i++) { 
	p = (int) ((offset_list[i] - min_st_offset + fd_size)/fd_size - 1);
	off = offset_list[i];
	len = (int) (((off+len_list[i]-1) <= fd_end[p]) ? len_list[i] : 
	           (fd_end[p] - off + 1));

	if (recv_buf_idx[p] < recv_size[p]) {
	    if (curr_from_proc[p]+len > done_from_proc[p]) {
		if (!recv_buf_idx[p]) {
#ifdef NEEDS_MPI_TEST
		    j = 0;
		    while (!j) MPI_Test(requests+jj, &j, statuses+jj);
#else
		    MPI_Wait(requests+jj, statuses+jj);
#endif
		    jj++;
		}
		if (done_from_proc[p] > curr_from_proc[p]) {
		    size = ADIOI_MIN(curr_from_proc[p] + len - 
			     done_from_proc[p], recv_size[p]-recv_buf_idx[p]);
		    buf_incr = done_from_proc[p] - curr_from_proc[p];
		    ADIOI_BUF_INCR
                    buf_incr = curr_from_proc[p] + len - done_from_proc[p];
		    curr_from_proc[p] = done_from_proc[p] + size;
		    ADIOI_BUF_COPY
		}
		else {
		    size = ADIOI_MIN(len, recv_size[p]-recv_buf_idx[p]); 
		    buf_incr = len;
		    curr_from_proc[p] += size;
		    ADIOI_BUF_COPY
		}
	    }
	    else {
		curr_from_proc[p] += len;
		buf_incr = len;
		ADIOI_BUF_INCR
	    }
	}
	else {
	    buf_incr = len;
	    ADIOI_BUF_INCR
	}

	/* this request may span the file domains of more 
	          than one process */
	rem_len = len_list[i] - len;
	while (rem_len != 0) {
	    p++;
	    off = fd_start[p];
	    len = (int) (((off+rem_len-1) <= fd_end[p]) ? rem_len : 
			(fd_end[p] - off + 1));

	    if (recv_buf_idx[p] < recv_size[p]) {
		if (curr_from_proc[p]+len > done_from_proc[p]) {
		    if (!recv_buf_idx[p]) {
#ifdef NEEDS_MPI_TEST
			j = 0;
			while (!j) MPI_Test(requests+jj, &j, statuses+jj);
#else
			MPI_Wait(requests+jj, statuses+jj);
#endif
			jj++;
		    }
		    if (done_from_proc[p] > curr_from_proc[p]) {
			size = ADIOI_MIN(curr_from_proc[p] + len - 
			      done_from_proc[p], recv_size[p]-recv_buf_idx[p]);
			buf_incr = done_from_proc[p] - curr_from_proc[p];
			ADIOI_BUF_INCR
			buf_incr = curr_from_proc[p]+len-done_from_proc[p];
			curr_from_proc[p] = done_from_proc[p] + size;
			ADIOI_BUF_COPY
		    }
		    else {
			size = ADIOI_MIN(len,recv_size[p]-recv_buf_idx[p]);
			buf_incr = len;
			curr_from_proc[p] += size;
			ADIOI_BUF_COPY
		    }
		}
		else {
		    curr_from_proc[p] += len;
		    buf_incr = len;
		    ADIOI_BUF_INCR
		}
	    }
	    else {
		buf_incr = len;
		ADIOI_BUF_INCR
	    }
	    rem_len -= len;
	}
    }
    for (i=0; i<nprocs_for_coll; i++) 
	if (recv_size[i]) recd_from_proc[i] = curr_from_proc[i];
}
