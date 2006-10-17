/* 
   -----------------------------------------------------
   THESE FILE IS INCLUDED BY: direct_ff.c
   
   To get a readable version of the complete source do
   a preprocessor run with direct_ff.c
   -----------------------------------------------------

   This file can be used to include the following functions:

	function           - define before include
        ------------------------------------------------                     
	MPID_SMI_Pack_ff   - __PACK__
	MPID_SMI_UnPack_ff - __UNPACK__
*/

#if defined __PACK__

    /* When packing the dt_buf ist the non-contig source buffer
       and lin_buf the destination */

int MPID_SMI_Pack_ff 
(char *dt_buf, struct MPIR_DATATYPE *dtype_ptr,
 char *lin_buf, int partner, int max, int outlen)

#elif defined __UNPACK__

    /* When Unpacking the lin_buf ist the linear source buffer
       and dt_buf the destination */

int MPID_SMI_UnPack_ff
(char *lin_buf, struct MPIR_DATATYPE *dtype_ptr,
 char *dt_buf, int partner, int max, int outlen)

#endif

{
    char    *start_buf; /* always the starting point of the dt_buf */

#if defined __PACK__
    char         *lbuf; /* used local buffer */
    char        *ldest; /* destination for the full lbuf */
    int           llen; /* length of the lbuf */
    int        *tmp_ca; /* tmp counter array to save the initial array in case of transm. errors */
    int        save_ca; /* if the initial counter array isn't all zero we
			   have to know that we need to save it, in case a
			   transmission has an error ;) */
#endif

    int   cpl, tmp_inc; /* length of bytes copied in this run */
    int              i; /* counter */
    int *counter_array; /* counter array to keep the state for the stack */
    int     pos, start; /* pos indicator */

    MPIR_FF_LIST_ITEM *ff; /* pointer to the ff list */
 
#if defined __PACK__   
    MPID_STAT_ENTRY(directPack);
#elif defined __UNPACK__
    MPID_STAT_ENTRY(directUnpack);
#endif
 
    /* get memory for the counter array */
    ALLOCATE (counter_array, int *, sizeof(int) * dtype_ptr->depth);

#ifdef __PACK__
    lbuf = NULL;
    ALLOCATE (tmp_ca, int *, sizeof(int) * dtype_ptr->depth);
#endif

    /* zero bytes copied so far in this run */
    cpl = 0;

    start_buf = dt_buf;

    do {
	/* init... */
	ff = dtype_ptr->ff;
	for (i=0;i <= ff->top;i++) counter_array[i] = 0; /* Initialize counter... */

#ifdef __PACK__
	save_ca = 0;
#endif
	
	/* if outlen > 0 we copied some bytes before. We now have to find the position
	   to restart in the src buffer and adjust the counter_array and ff pointer */

	start = outlen % dtype_ptr->size;
	dt_buf = start_buf + ((int) outlen / dtype_ptr->size) * dtype_ptr->extent;

	if (start > 0) {
	    pos = find_pos (&start, &ff,counter_array);
	    if (pos < 0) return 0; /* find_pos was not successful, so we have an error */
#ifdef __PACK__
	    save_ca = 1;
	    memcpy (tmp_ca, counter_array, sizeof(int) * dtype_ptr->depth);
#endif
	} else {
#ifdef __PACK__
	    save_ca = 0;
#endif
	    pos = 0;
	}

#ifdef __PACK__
	/* Now we know the size of the first leave to copy and can decide
	   about the use of the local buffer */

	if ((!lbuf) && (ff->contig_size < MPID_SMI_cfg.NC_MINSIZE)) {
	    ALLOCATE (lbuf, char *, max);
	    ldest = lin_buf+cpl;
	    llen = max;
	}
#endif
	    
	/* do the work... */
	
	if (start > 0) {
	    /* if start is not 0 after find_pos, we have a partial copy here,
	       so we copy the rest and adjust the counter_array,cpl and pos. 
	       this should only happen for the first time we enter the loop!!! 
	    */
	    if (cpl > 0) { fprintf (stderr,"MPID_SMI_Pack/Unpack: cpl not 0, but tried a partial copy!!!\n"); }
	    cpl = ff->contig_size - start;
	    if (cpl > max) cpl = max;
#if defined __PACK__
	    if (lbuf) {
		MPID_STAT_CALL(directWRITE_lbuf);
		mpid_smi_peelcpy_l (lbuf, dt_buf + pos + start, cpl);
		MPID_STAT_RETURN(directWRITE_lbuf);
	    } else {
		MPID_STAT_CALL(directWRITE_part);
	        peelcpy_r (lin_buf, dt_buf + pos + start, cpl);
		while (MPID_SMI_cfg.DO_VERIFY && SMI_Check_transfer_proc(partner, SMI_CHECK_FULL) != SMI_SUCCESS) {
	            peelcpy_r (lin_buf, dt_buf + pos + start, cpl);
	            MPID_STAT_COUNT(sci_transm_error);
	    	}
		MPID_STAT_RETURN(directWRITE_part);
	    }
#elif defined __UNPACK__
	    MPID_STAT_CALL(directREAD_part);
            MEMCPY_R (dt_buf + pos + start, lin_buf, cpl);
	    MPID_STAT_RETURN(directREAD_part);
#endif
	    counter_array[ff->top]++;
	    outlen += cpl;
	    max -= cpl;
	    pos += ff->stack[ff->top].extent;
	}
	
	/* Now more space avail, bail out */
	if (max < 1) {
	    /* free all used buffers */
	    FREE (counter_array);
#if defined __PACK__
	    if (tmp_ca) FREE (tmp_ca);
	    if (lbuf) {
		MPID_STAT_CALL(directFLUSH_lbuf);
 		MEMCPY_W (ldest, lbuf, llen, partner);
		MPID_STAT_RETURN(directFLUSH_lbuf);
		free (lbuf);
	    }
	    MPID_STAT_EXIT(directPack);
#elif defined __UNPACK__
	    MPID_STAT_EXIT(directUnpack);
#endif
	    return cpl;
	}
	
	/* Main loop */
	while (ff && (max > 0)) {
   
	    /* do one leaf */

#if defined __PACK__
	    MPID_STAT_CALL(directPackLeave);
	    if (!lbuf) {
		tmp_inc = pack_leaf_basic_r (dt_buf,pos,lin_buf+cpl,ff,max,counter_array);
		while (MPID_SMI_cfg.DO_VERIFY && SMI_Check_transfer_proc(partner, SMI_CHECK_FULL) != SMI_SUCCESS) {
		    if (save_ca) memcpy (counter_array, tmp_ca, sizeof(int) * dtype_ptr->depth);
		    else for (i=0;i <= ff->top;i++) counter_array[i] = 0; /* Initialize counter... */
		    MPID_STAT_COUNT(sci_transm_error);
		    tmp_inc = pack_leaf_basic_r (dt_buf,pos,lin_buf+cpl,ff,max,counter_array);
		}
	    } else {
		tmp_inc = pack_leaf_basic_l (dt_buf,pos,lbuf+cpl,ff,max,counter_array);
	    }
	    MPID_STAT_RETURN(directPackLeave);

	    save_ca = 0; /* after the first send, all counter arrays get initialized to zero */
#elif defined __UNPACK__
	    MPID_STAT_CALL(directUnpackLeave);
	    tmp_inc = unpack_leaf_basic (lin_buf+cpl,pos,dt_buf,ff,max,counter_array);
	    MPID_STAT_RETURN(directUnpackLeave);
#endif

	    cpl += tmp_inc;     /* increase # of bytes copied in this run */
	    outlen += tmp_inc;  /* of course also # of total bytes copied is increased */
	    max -= tmp_inc;     /* the available size decreases */
	    
	    pos = 0; /* pos get's set to zero, so pack_leaf_basic set's it to the correct value */
	    ff = ff->next; /* go to the next leaf */
	    
	    if (ff) for (i=0;i <= ff->top;i++) counter_array[i] = 0; /* Initialize counter... */
	}
	
    } while (max > 0);

    /* free all used buffers */
    FREE (counter_array);

#if defined __PACK__
    if (tmp_ca) FREE (tmp_ca);
    if (lbuf) {
	MPID_STAT_CALL(directFLUSH_lbuf);
	MEMCPY_W (ldest,lbuf,llen,partner);
	MPID_STAT_RETURN(directFLUSH_lbuf);
	free(lbuf);
    }
    MPID_STAT_EXIT(directPack);
#elif defined __UNPACK__
    MPID_STAT_EXIT(directUnpack);
#endif

    return cpl;
}

