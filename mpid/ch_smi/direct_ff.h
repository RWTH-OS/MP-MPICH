
/* $Id: direct_ff.h,v 1.4 2002/05/23 14:58:51 joachim Exp $

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


int MPID_SMI_Pack_ff ( char *inbuf, struct MPIR_DATATYPE *dtype_ptr, 
		       char *outbuf, int dest, int max, int outlen);

int MPID_SMI_UnPack_ff ( char *inbuf, struct MPIR_DATATYPE *dtype_ptr, 
		       char *outbuf, int from, int max, int outlen);
