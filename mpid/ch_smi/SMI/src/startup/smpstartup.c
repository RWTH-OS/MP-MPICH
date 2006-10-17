/* $Id$ */

/* set up the basic shared memory communication facilities */

#include <stdlib.h> 

#include "env/smidebug.h"
#include "env/smi_init.h"
#include "utility/smi_time.h"
#include "memory/shseg_key.h"
#include "tcpsync.h"
#include "startup.h" 


#ifndef WIN32
#include <errno.h>
#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif
#endif

#ifdef DMALLOC
#include <dmalloc.h>
#endif

static int  my_proc_rank;
static int  nbr_procs;

static volatile int *sgmt_addr = (int*)0x0;
static int	sgmt_key = 9966;
static int	sgmt_id;

smi_error_t _smi_smp_startup (smi_args_t *sArgs)
{
    tcp_ident_t idLocal;
    int i, ret, start_secs, error;
    
    DSECTION("_smi_smp_startup");
    DSECTENTRYPOINT;
    
    my_proc_rank = sArgs->iProcRank;
    nbr_procs    = sArgs->iNumProcs;
    
    /* Currently only tcpstartup is supported */
    ASSERT_R((sArgs->eStartupMethod == smi_startup_use_tcp),"tcp startup mechanism is required!",
	     SMI_ERR_NOTIMPL);
    if (_smi_tcp_init() == -1) {
	DERROR("could not init tcp");
	SMI_Abort(-1);
    }
    
    if (my_proc_rank == 0) {
	DNOTICE("Allocating base segment");
	do {
	    sgmt_key = _smi_modify_key(sgmt_key);
	    error = rs_shmget(sgmt_key, BASESGMT_SIZE(nbr_procs), IPC_CREAT|IPC_EXCL|0600);
	} while (error == -1);
	
	DNOTICEI("Initializing via base segment, key", sgmt_key);
	
	DNOTICE("Broadcasting key of base segment...");
	
	_smi_tcp_mkident(&idLocal, my_proc_rank, sArgs->iMagicNumber, sArgs->szExecName, 0);
	ret = _smi_TcpBroadcast(sArgs->szSyncHost, sArgs->iPortNumber, &sgmt_key, sizeof(int), 
				0, nbr_procs, &idLocal);
	_smi_tcp_finalize();
	ASSERT_A((ret==0),"Could not contact all clients",-1);
	
    } 
    else {
	DNOTICE("Trying to connect to base segment");
	DNOTICE("Getting key of base segment...");

	_smi_tcp_init();
	_smi_tcp_mkident(&idLocal, my_proc_rank, sArgs->iMagicNumber, sArgs->szExecName, 0);
	ret = _smi_TcpBroadcast(sArgs->szSyncHost, sArgs->iPortNumber, &sgmt_key, sizeof(int), 
				0, nbr_procs, &idLocal);
	_smi_tcp_finalize();
	ASSERT_A((ret==0),"Could not contact all clients",-1);
	    
	start_secs = _smi_get_seconds();
	do {
	    error = rs_shmget(sgmt_key, BASESGMT_SIZE(nbr_procs), 0600);
	} while ((error == -1) && (_smi_get_seconds() - start_secs <= SMP_STARTUP_TIMEOUT));
	
	_smi_init_flush(sgmt_addr, 0);
    }
    sgmt_id = error;
    ASSERT_X((sgmt_id != -1),"Could not create base segment",SMI_ERR_NOMEM);
    
    sgmt_addr = (int*)shmat(sgmt_id,(char*)sgmt_addr,0600);
    ASSERT_A((size_t)sgmt_addr != -1, "Could not map base segment", SMI_ERR_MAPFAILED);
    DNOTICEP("Mapped base segment to: ",sgmt_addr);
    
    if (my_proc_rank==0) {
		int ii;
		for(ii=0;ii<nbr_procs;ii++) {
			SEC_SET(sgmt_addr[BASESGMT_OFFSET_BARRIER + ii*INTS_PER_STREAM],DEP_FALSE(ii));
		}
    } 
    else {
		_smi_SECRead(&(sgmt_addr[BASESGMT_OFFSET_BARRIER]), DEP_FALSE(0), DEP_TRUE(0));
    }
    
    _smi_ll_init( 0, 1, NULL, my_proc_rank, nbr_procs, sgmt_addr, my_proc_rank, nbr_procs);
    _smi_init_flush(sgmt_addr, 0);
    
    if (_smi_use_watchdog) {
	DNOTICE("Initializing watchdog");
	_smi_init_watchdog(my_proc_rank, nbr_procs, sgmt_addr + BASESGMT_OFFSET_ALIVECHECK);
	_smi_wd_disable();
    }
    
    /* For compatibility with SCI mode: give all nodes the same "virtual" SCI node id. */
    ALLOCATE(_smi_sci_rank, int *, nbr_procs*sizeof(int));
    for (i = 0; i < nbr_procs; i++)
	_smi_sci_rank[i] = 4;
    
    _smi_tcp_finalize();
    
    DNOTICE("lowlevel init completed!");
    DSECTLEAVE;
    return(SMI_SUCCESS);
}


smi_error_t _smi_smp_shutdown()
{
    DSECTION("_smi_smp_shutdown");
    int error;
    
    error = shmdt((void*)sgmt_addr);
    ASSERT_R((error == 0), "detach of shared memory failed", SMI_ERR_MAPFAILED);
    
#ifndef WIN32
    if (my_proc_rank == 0)
#endif
    {
	error = rs_shmctl(sgmt_id, IPC_RMID, 0);
    }
    free(_smi_sci_rank);
    
    return (SMI_SUCCESS);
}
