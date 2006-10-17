/* $Id: sync_init.c,v 1.1 2004/03/19 22:14:23 joachim Exp $ */

#include "env/general_definitions.h"
#include "env/smidebug.h"
#include "sync.h"
#include "sync_init.h"
#include "mutex.h"
#include "barrier.h"

/*********************************************************************************/
/*** This function intializes the mutex and barrier modules                    ***/
/*********************************************************************************/ 
smi_error_t _smi_synchronization_init(int use_signals)
{
    DSECTION("_smi_synchronization_init");
#ifdef USE_SCHULZSYNC
    errCode_t	err;
#endif
    
    DSECTENTRYPOINT;
    
    DNOTICE("initializing barrier module");
    _smi_barrier_module_init();

    DNOTICE("initializing mutex module");
    _smi_mutex_module_init();

#ifdef USE_SCHULZSYNC
    if (_smi_all_on_one != TRUE) {
	/* To be removed after SCHULZLOCK test phase */
	ASSERT_A(_smi_sci_rank != NULL, "sci ranks not initialized",SMI_ERR_OTHER);
	
	DNOTICE("initializing SchulzLocks");
	err = syncMod_start(_smi_my_proc_rank, _smi_nbr_procs, _smi_sci_rank, 0x456, 0x455); 
	ASSERT_R(err == 0, "Could not start syncMod", SMI_ERR_OTHER);
    }
    else {
	DWARNING("SCHULZLOCKS only work if a minimum of two nodes is involved!");
    }
#endif     

    /* XXX Scali SISCI now seems to support interrupts, too. However, it 
       crashes when removing the interrupts. Disable signalling again...*/
#ifndef SCALI_SISCI
    if (use_signals)
	_smi_signal_init(_smi_my_proc_rank, _smi_nbr_procs);
#endif

    DSECTLEAVE;
    return(SMI_SUCCESS);
}
