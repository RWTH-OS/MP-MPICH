/* $Id$ */

#include "smidebug.h"

#include "general_definitions.h"
#include "startup/startup.h"
#include "finalize.h"
#include "synchronization/sync_finalize.h"
#include "regions/free_shreg.h"
#include "redirect_io.h"
#include "utility/statistics.h"
#include "utility/query.h"
#include "dyn_mem/dyn_mem.h"


smi_error_t SMI_Finalize() {
    int region;
    smi_error_t error;
    smi_error_t mpi_error;

    DSECTION("SMI_Finalize");
    DSECTENTRYPOINT;
    
    fflush(stdout);
    fflush(stderr);
    
    _smi_ll_barrier();
    
    DNOTICE("finalizing message-pasing module");
    _smi_finalize_mp();
    
    DNOTICE("finalizing memcpy module");
    _smi_memcpy_finalize();

    DNOTICE("finalizing dynamic memory management");
    _smi_free_shregMMU();

    DNOTICE ("printing statistics");
    _smi_runtime_statistics();
    
    DNOTICE("terminating watchdog");
    _smi_finalize_watchdog();
    /* make sure *all* watchdog threads have terminated */
    _smi_ll_barrier();   
    
    DNOTICE("free internal shared memory regions");
    for (region = 0; region < _smi_nbr_machines; region++) {
	error = _smi_free_shreg(_smi_int_shreg_id[region]);
	if (error != SMI_SUCCESS) {
	    DPROBLEM("could not free all internal shared memory regions");
	}
    }

    DNOTICE("finalizing synchronization");
    _smi_synchronization_finalize();
    
    DNOTICE("lowlevel communication shutdown");
    mpi_error = _smi_lowlevel_shutdown();
    if (mpi_error!=MPI_SUCCESS) {
	DNOTICEI("ERROR (SMI_Finalize,1):%i\n",1000+mpi_error);
	SMI_Abort(1000+mpi_error);
    }
    
    DNOTICE("clear all resources");
    _smi_clear_all_resources();
    
    DNOTICE("close redirected io");
    _smi_close_redirected_io();

    _smi_finalize_query();

    DNOTICE("destroying lock for mis-structure");
    SMI_DESTROY_LOCK(&_smi_mis_lock);
    
    DSECTLEAVE;
    return(SMI_SUCCESS);
}




