
/* 
   -----------------------------------------------------
   THESE FILE IS INCLUDED BY: direct_ff.c
   
   To get a readable version of the complete source do
   a preprocessor run with direct_ff.c
   -----------------------------------------------------

   This file can be included to provide the following functions:

	function          - define before include
        ------------------------------------------------                     
	pack_leaf_basic_r - __PACK__ and __DIRECT__
	pack_leaf_basic_l - __PACK__ and __LBUF__
	unpack_leaf_basic - __UNPACK__

*/

#if defined __PACK__
#if defined __DIRECT__
  static int pack_leaf_basic_r (
#elif defined __LBUF__
  static int pack_leaf_basic_l (
#endif			      
#elif defined __UNPACK__
  static int unpack_leaf_basic (
#endif
		       char *src,
		       int dt_pos,
		       char *dest, 
		       MPIR_FF_LIST_ITEM *ff,
		       int max,
		       int *ca)
{

    int top;
    int si;
    int copied;
        
    max -= ff->contig_size;
    
    /* Save stack top and adjust src-position */
    top = ff->top;
    copied = 0;
    if (!dt_pos) dt_pos = ff->pos;
    
    do {
	for (si = top;ca[si]<ff->stack[si].count;ca[si]++) {
	    /* check if space is still available */
	    if (copied > max) {
		/* copy a part of the block if still space available */
		max = max + ff->contig_size - copied;
		if (max) {

		    #if defined __PACK__
 		     #if defined __DIRECT__
 		      MPID_STAT_CALL(directWRITE_part);
		      peelcpy_r (dest, src+dt_pos, max);
		      MPID_STAT_RETURN(directWRITE_part);
		     #elif defined __LBUF__
		      MPID_STAT_CALL(directWRITE_lbuf);
		      mpid_smi_peelcpy_l (dest, src+dt_pos, max);
		      MPID_STAT_RETURN(directWRITE_lbuf);
		     #endif
		    #elif defined __UNPACK__
		      MPID_STAT_CALL(directREAD_part);
		      MEMCPY_R (dest+dt_pos, src, max);
		      MPID_STAT_RETURN(directREAD_part);
		    #endif
		    
		    copied += max;
		}
		return copied;
	    }

	    #if defined __PACK__
	     #if defined __DIRECT__
	      MPID_STAT_CALL(directWRITE_direct);
	      peelcpy_r (dest, src+dt_pos, ff->contig_size);
	      MPID_STAT_RETURN(directWRITE_direct);
	     #elif defined __LBUF__
	      MPID_STAT_CALL(directWRITE_lbuf);
	      mpid_smi_peelcpy_l (dest, src+dt_pos, ff->contig_size);
	      MPID_STAT_RETURN(directWRITE_lbuf);
	     #endif

      	     dest += ff->contig_size;

	    #elif defined __UNPACK__
     	     MPID_STAT_CALL(directREAD);
 	     MEMCPY_R (dest+dt_pos, src, ff->contig_size);
	     MPID_STAT_RETURN(directREAD);

 	     src += ff->contig_size;

	    #endif
 
	    copied += ff->contig_size;
	    dt_pos += ff->stack[si].extent;
	}

	while (ca[si]==ff->stack[si].count) {
	    dt_pos -= ff->stack[si].count*ff->stack[si].extent;
	    ca[si] = 0;
	    si--;
	    dt_pos += ff->stack[si].extent;
	    ca[si]++;
	}
    } while (si);

    return copied;
}
  
