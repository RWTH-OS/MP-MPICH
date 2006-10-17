/* $Id$ */

/* Management of persistent communication, memroy registering etc. */

#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <assert.h>

#include "smi.h"

#include "adi2config.h"
#include "mpid.h"
#include "smidev.h"
#include "smirndv.h"
#include "smimem.h"
#include "smistat.h"
#include "smidef.h"


int MPID_SMI_Persistent_init (union MPIR_HANDLE *req)
{
    struct MPIR_DATATYPE *dtype_ptr;
    int extent, size, count;
    int *reg_id, sgmt_id;
    size_t contig_size;
    void *buf;

    /* Try to register the buffer if its size is in the rendez-vous area */
    /* We need to know the size of the buffer the data is located in; this means
       we look at the extent of the datatype. And we need to know the size of the
       "payload" to select the protocol. */
    switch (req->chandle.handle_type) {
    case MPIR_PERSISTENT_SEND:
	dtype_ptr = req->persistent_shandle.perm_datatype;
	count  = req->persistent_shandle.perm_count;
	buf    = req->persistent_shandle.perm_buf;
	reg_id = (int *)req->rhandle.rid;

	req->persistent_shandle.shandle.recv_handle = (MPID_RNDV_T)MPID_SBalloc(rndvinfo_allocator); 
	memset(req->persistent_shandle.shandle.recv_handle, 0, sizeof(MPID_SMI_RNDV_info)); 
	break;
    case MPIR_PERSISTENT_RECV:
	dtype_ptr = req->persistent_rhandle.perm_datatype;
	count = req->persistent_rhandle.perm_count;
	buf   = req->persistent_rhandle.perm_buf;
	reg_id = (int *)req->shandle.sid;

	req->persistent_rhandle.rhandle.recv_handle = (MPID_RNDV_T)MPID_SBalloc(rndvinfo_allocator); 
	memset(req->persistent_rhandle.rhandle.recv_handle, 0, sizeof(MPID_SMI_RNDV_info)); 
	break;
    }
    
    contig_size = MPIR_GET_DTYPE_SIZE(datatype,dtype_ptr);
    if (MPID_SMI_cfg.REGISTER && 
	dtype_ptr->size*count > MPID_SMI_EAGERSIZE && 
	contig_size > 0) {	
	MPID_SMI_Local_mem_register (buf, dtype_ptr->extent*count, reg_id, &sgmt_id);
	/* This call may have succeeded or not - it will show up when the data will be
	   transmitted by checking the address with SMI for an SMI region. If it succeeds,
	   the memory will be persistently registered. */
    } else {
	*reg_id = -1;
    }
    
    return MPI_SUCCESS;
}

int MPID_SMI_Persistent_free (union MPIR_HANDLE *req)
{
    void *buf;
    int reg_id;

    switch (req->chandle.handle_type) {
    case MPIR_PERSISTENT_SEND:
	buf    = req->persistent_shandle.perm_buf;
	reg_id = (int)req->shandle.sid[0];

	MPID_SBfree (rndvinfo_allocator, req->persistent_shandle.shandle.recv_handle);
	req->persistent_shandle.shandle.recv_handle = NULL;
	break;
    case MPIR_PERSISTENT_RECV:
	buf   = req->persistent_rhandle.perm_buf;
	reg_id = (int)req->rhandle.rid[0];

	MPID_SBfree (rndvinfo_allocator, req->persistent_rhandle.rhandle.recv_handle);
	req->persistent_rhandle.rhandle.recv_handle = NULL;
	break;
    }
    if (MPID_SMI_cfg.REGISTER && reg_id >= 0) {
	MPID_SMI_Local_mem_release (NULL, reg_id, MPID_SMI_RSRC_DESTROY);
    }
    
    return MPI_SUCCESS;
}


