
/* $Id$

  These file provides the functions to copy non-contiguous data directly into the
  shared memory with the use of the ff algorithm.

  Exported functions:
  -------------------

  int MPID_SMI_Pack_ff ( char *inbuf, struct MPIR_DATATYPE *dtype_ptr, 
                         char *outbuf, int dest, int max, int *outlen)

   -> Pack the datatype from <inbuf> to <outbuf>. <max> bytes are available in
      <outbuf> and <outlen> bytes are already copied.

  int MPID_SMI_UnPack_ff ( char *inbuf, struct MPIR_DATATYPE *dtype_ptr, 
                           char *outbuf, int dest, int max, int *outlen)

   -> Unpack the datatype from <inbuf> to <outbuf>. <max> bytes are available in
      <inbuf> and <outlen> bytes are already copied.
  
*/
 

#include "smidef.h"
#include "smisync.h"
#include "mpid.h"
#include "smidev.h"
#include "smimem.h"
#include "smistat.h"

#include "direct_ff.h"


/*
   find_pos: used internally to find the position in the datatype for
             interrupted copies. <start> gives the amount of bytes already
	     copied, <ff> holds the ff-list and <ca> is the counter array.
	     After the call, the function returns the restart position
	     in the source buffer, <start> contains the number of bytes
	     that still have to be copied when the copy was interrupted
	     in a block, <ff> holds the pointer to the restart ff-list item
	     and the <ca> is initialized, so we can restart.
*/
static int find_pos (int *start, MPIR_FF_LIST_ITEM **ff, int *ca)
{
    int pos = 0;
    int i;

    /* first advance through the stacks that are all smaller than start */
    while ((*ff) && (((*ff)->stack[1].count * (*ff)->stack[1].size) <= *start)) {
	*start -= (*ff)->stack[1].count * (*ff)->stack[1].size;
	(*ff) = (*ff)->next;
    }
    if (!(*ff)) return -1; /* if we have no stack left, there is nothing to send */

    /* the interrupt happend in this stack, so adjust the starting position */
    pos += ((*ff)->pos);

    /* now advance through the remaining stack */
    for (i=1;i<=(*ff)->top;i++) {
	ca[i] = *start / (*ff)->stack[i].size;     /* gives us the # of blocks that fit */
	*start = *start % (*ff)->stack[i].size;    /* gives us the # of bytes that are left */
	pos += (*ff)->stack[i].extent * ca[i];     /* adjust the position */
    }

    return pos;
}


/* Destination addressses of remote-SCI-copy-operations need to be aligned for 
   optimum performance, depending on the CPU and PCI buffers (32 byte on PentiumIII, 
   64 byte on P4, ...).
   
   This value can be set in the device configuration;  the rest is done by 
   this little function. */
void peelcpy_r (char *dest, char *src, ulong len){
    ulong peel;

    /* Calculate how much we're off, then...  */
    if (MPID_SMI_cfg.NC_ALIGN > 0 && MPID_SMI_cfg.NC_ALIGN < len) {
	peel = ((size_t)dest) & (MPID_SMI_cfg.NC_ALIGN - 1);    
	if (peel != 0) {
	    peel = MPID_SMI_cfg.NC_ALIGN - peel;
	    len -= peel; 
	    
	    /* do a small copy with this length (bad performance), ... */
	    MEMCPY_S(dest, src, peel);
	    dest += peel; src += peel;
	}
    }
    
    /* ...and copy the remaining (hopefully larger) block with proper alignment. */
    if (len > 0) 
	REMOTE_COPY(dest, src , len);

    return;
}
	

/* 
   OK, "direct_ff.inc" and "direct_leaf.inc" are generic, the algo
   is the same for send and recv and local buffer send. Only the
   modus and direction of copying changes. There are some
   complications because of the need to error check the send-ops.
   The Code of the direct_*.inc files may be a little hard to
   understand, so better do an preprocessor run of this file
   (with gcc: 'gcc -E direct_ff.c') and you get understandable
   sourcecode ;-)
*/

/* Include pack_leaf_basic_r() for direct packing */   
#define __PACK__
#define __DIRECT__
#include "direct_leaf.inc"
#undef __DIRECT__

/* Include pack_leaf_basic_l() for local buffer packing */
#define __LBUF__
#include "direct_leaf.inc"
#undef __LBUF__

/* Include the main MPID_SMI_Pack_ff() */
#include "direct_ff.inc"
#undef __PACK__

/* Include unpack_leaf_basic() for unpacking */
#define __UNPACK__
#include "direct_leaf.inc"

/* Include the main MPID_SMI_Unpack_ff() */
#include "direct_ff.inc"
#undef __UNPACK__
