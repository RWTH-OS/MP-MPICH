#ifndef MPID_SCIMEMCPY
#define MPID_SCIMEMCPY

/* this is a special memcpy version for x86 architectures
   which optimizes the use of the stream buffers on the
   SCI board */

void sci_memcpy ( void *, void *, int );

#endif
