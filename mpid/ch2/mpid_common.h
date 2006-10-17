/* $Id$ */

#ifndef _2345234_MPID_COMMON_H
#define _2345234_MPID_COMMON_H

/* 
   We could have a general set of actions for preparting data, but for
   now we'll stick to these 3.  Note that the "swap" form might eventually
   include extension/contraction of types with different lengths, and the 
   "OK" might split into OK and OK_FIX_SIZE.  Or we might change the 
   entire interface to return a pointer to a structure containing the
   actions.
 */
/* An MPID_Msg_pack_t describes "how a message can be packed for all members
   of a communicator", and is used by PACK.  */
typedef enum { MPID_MSG_OK, MPID_MSG_SWAP, MPID_MSG_XDR } MPID_Msg_pack_t;


#endif /* _2345234_MPID_COMMON_H */
