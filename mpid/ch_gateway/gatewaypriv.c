/* $Id$ */
/*
   This file contains routines that are private and unique to the ch_gateway
   implementation
 */
#ifndef WIN32
#include <unistd.h>
#endif

#include "mpid.h"
#include "mpiddev.h"
#include "sbcnst.h"
#include "../../src/coll/coll.h"

#include "metampi.h"

#define LOCAL_SYNCID 0
#define HOST_SYNCID  1

/* XXX I don't really want to do this this way, but for now... */
/* should use the according defines instead */
static struct _MPIR_COLLOPS meta_collops =  {
#ifdef MPID_Barrier
    MPID_FN_Barrier,
#else
    NULL,
#endif
#ifdef MPID_Bcast
    MPID_FN_Bcast,
#else
    NULL,
#endif
    NULL,
    NULL,
    NULL,
    NULL,
#ifdef MPID_Allgather
    MPID_FN_Allgather,
#else
    NULL,
#endif
#ifdef MPID_Allgatherv
    MPID_FN_Allgatherv,
#else
    NULL,
#endif
    NULL,
    NULL,
#ifdef MPID_Reduce
    MPID_FN_Reduce,
#else
    NULL,
#endif
#ifdef MPID_Allreduce
    MPID_FN_Allreduce,
#else
    NULL,
#endif
#ifdef MPID_Reduce_scatter
    MPID_FN_Reduce_scatter,
#else
    NULL,
#endif
#ifdef FOO
#ifdef MPID_Reduce_scatterv
    MPID_FN_Reduce_scatterv,
#else
    NULL,
#endif
#endif
    NULL,
    1                              /* Giving it a refcount of 1 ensures it
				    * won't ever get freed.
				    */
};

MPIR_COLLOPS MPIR_meta_collops = &meta_collops;

/* 
 * global, exported variables 
 */

/* this buffer is used in when sending a message from an application process 
   to the router process (then it contains the meta header and the MPI message contents) */
Meta_Header  *meta_msg;
size_t       meta_msgsize;

static unsigned int MPID_Gateway_global_msgid;

int next_router_to_metahost( int target_metahost );

void MPID_Gateway_init( argc, argv )
int  *argc;
char **argv;
{
   
    int i, qid, rp_rank, rp_n, metahost;
    int g_rank, rp_offset;       
    char **rp_argv;

    /* setup of the internal mapping between GLOBAL and HOST: which routing process to use 
       (each MPI proc has one fixed routing proc for every remote host) */
    MPIR_meta_cfg.granks_to_router = (int *)MALLOC((MPIR_meta_cfg.np + MPIR_meta_cfg.nrp) * sizeof(int));
    g_rank = 0; /* global rank */
    rp_offset = 0;
    for (metahost = 0; metahost < MPIR_meta_cfg.nbr_metahosts; metahost++) {
	if( MPIR_meta_cfg.useRouterToMetahost[metahost] ) 
	    rp_rank = next_router_to_metahost( metahost );
	else
	    /* no routing to own host */
	    rp_rank = -1;
	
	for (i = g_rank; i < g_rank + MPIR_meta_cfg.np_metahost[metahost] + MPIR_meta_cfg.nrp_metahost[metahost] ; i++)
	    MPIR_meta_cfg.granks_to_router[i] = MPIR_meta_cfg.metahost_firstrank + rp_rank;
		
	g_rank += MPIR_meta_cfg.np_metahost[metahost] +  MPIR_meta_cfg.nrp_metahost[metahost];
	rp_offset += MPIR_meta_cfg.my_nrp[metahost];
    }

    /* memory allocation */
    meta_msg = (Meta_Header *)malloc(INIT_META_MSGSIZE);
    meta_msgsize = INIT_META_MSGSIZE;

    MPID_Gateway_global_msgid = 0;
}


Meta_Header *MPID_Gateway_Wrap_Msg (buf, src, dest, len, tag, ctxt_id, mode, msgrep, msgid)
void  *buf;
int   src, dest, len, tag, ctxt_id;
MPI_Sendmode mode;
int   msgrep;
unsigned int msgid;
{
    if (meta_msgsize < sizeof(Meta_Header) + len) {
	meta_msgsize = sizeof(Meta_Header) + len;
	if (!(meta_msg = (Meta_Header *) realloc (meta_msg, meta_msgsize)))
	return NULL;
    }

    meta_msg->mode                   = MPI_MSG;
    meta_msg->msg.MPI.src_comm_lrank = src;
    meta_msg->msg.MPI.dest_grank     = dest;  
    meta_msg->msg.MPI.count          = len;
    meta_msg->msg.MPI.tag            = tag;
    meta_msg->msg.MPI.context_id     = ctxt_id;
    meta_msg->msg.MPI.mode           = mode;
    meta_msg->msg.MPI.msgrep         = msgrep;
    meta_msg->msg.MPI.msgid          = msgid;

    if (buf != NULL)
        memcpy (meta_msg + 1, buf, len);
    
    return meta_msg;
}


#if 0
int MPID_Gateway_Barrier ( comm_ptr )
     struct MPIR_COMMUNICATOR *comm_ptr;
{
    MPI_Status status;
    int host, i;
    int dest;
    
    /* in the current state, the Gateway-Barrier can only be used
       for synchronizing MPI_COMM_WORLD. For other (custom) communicators,
       we would have to get to know on which hosts the participating
       processes are running. This is not too complex, but not yet done. */
    
    /* local Barrier first */
    MPI_Barrier (MPI_COMM_LOCAL);
    
    
    /* now sync with all hosts via their local_rank_zero procs */
    if( MPID_MyLocalRank == 0 ) {
	dest = 0;
	for (host = 0; host < MPIR_meta_cfg.nbr_metahosts; host++) {
	    if (host != MPIR_meta_cfg.my_metahost_rank) {
		MPI_Sendrecv( (void *)0, 0, MPI_INT, dest, MPIR_BARRIER_TAG,
			      (void *)0, 0, MPI_INT, dest, MPIR_BARRIER_TAG, 
			      MPI_COMM_META, &status);
	    }
	    dest += MPIR_meta_cfg.np_metahost[host];
	}
    }
    
    /* closing local Barrier */
    MPI_Barrier (MPI_COMM_LOCAL);
    return MPI_SUCCESS;
}
#endif


void MPID_Gateway_finalize()
{
    VOLATILE int *globid;

    free (meta_msg);

    fflush(stdout);
    fflush(stderr);

    free (MPIR_meta_cfg.granks_to_router);
}


int next_router_to_metahost( int target_metahost )
{
    static int routerlist_index = 0;

    routerlist_index++;
    if( routerlist_index == rh_getNumRouters() )
	routerlist_index = 0;
    
    while( routerlist[routerlist_index].host != target_metahost ) {
	routerlist_index++;
	if( routerlist_index == rh_getNumRouters() )
	    routerlist_index = 0;
    }

    return routerlist[routerlist_index].metahostrank;
}

unsigned int MPID_Gateway_get_global_msgid( void )
{
    return MPID_Gateway_global_msgid++;
}
