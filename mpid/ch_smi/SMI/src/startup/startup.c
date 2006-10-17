/* $Id$ */

#include <signal.h>
#include <unistd.h>

#include "env/general_definitions.h"
#include "env/smidebug.h"
#include "startup.h"

static void alarm_handler(int num)
{
    DERROR("Startup synchronization timeout: not all processes present!");
    SMI_Abort(-1);
}       


smi_error_t _smi_lowlevel_init (smi_args_t *sArgs)
{
    DSECTION ("_smi_lowlevel_init");
    smi_error_t RetVal;
#ifndef WIN32
    void (*old_alarm_handler)(int);;
#endif

    DSECTENTRYPOINT;
     
    /* Default settings for the startup (internal regions etc.). May be changed
       lateron through the user via SMI_Set_adapter(). */
    _smi_adpt_export = _smi_DefAdapterNbr;
    _smi_adpt_import = _smi_DefAdapterNbr;
    
#ifndef NO_SISCI
    _smi_all_on_one = FALSE; 
    
    if (sArgs->iNumProcs < 0) {
	DNOTICE("All procs on one machine - using SYS V shared memory");
	_smi_all_on_one = TRUE;
	sArgs->iNumProcs = abs(sArgs->iNumProcs);
    }
#else
    _smi_all_on_one = TRUE; 
    if (sArgs->iNumProcs < 0) {
	sArgs->iNumProcs = abs(sArgs->iNumProcs);
    } else {
	DWARNING("Please make sure that really all processes are on one node,");
	DWARNING("since SMI has been compiled without SCI support");
    }
#endif
    
    ASSERT_R(sArgs->iNumProcs > 0,"Invalid Number of Processes",-1);
    ASSERT_R(sArgs->iProcRank >= 0 && sArgs->iProcRank < sArgs->iNumProcs, "Invalid Proc Rank",-1);
    
    DNOTICEI("This proc has ID  : ", sArgs->iProcRank);
    DNOTICEI("Total nbr of procs: ", sArgs->iNumProcs);

#ifndef WIN32
    if (_smi_use_watchdog) {
    	old_alarm_handler = signal(SIGALRM, alarm_handler);  
    	alarm(SYNC_STATIC_TIMEOUT + (sArgs->iNumProcs * SYNC_PERPROC_TIMEOUT));
    }
#endif

#ifdef NO_SISCI
    RetVal =_smi_smp_startup(sArgs);
#else 
    RetVal = _smi_all_on_one ? _smi_smp_startup(sArgs) : _smi_sci_startup(sArgs);
#endif
#ifndef WIN32 
    if (_smi_use_watchdog) {
    	alarm(0); 
    	/* Signal is set to SIG_IGN, due to problems when using default or previous handler routine */
    	signal(SIGALRM, SIG_IGN); 
    }
#endif

    DSECTLEAVE;
    return(RetVal);
}

smi_error_t _smi_lowlevel_shutdown (void)
{
#ifdef NO_SISCI
    return(_smi_smp_shutdown());
#else 
    return (_smi_all_on_one ? _smi_smp_shutdown() : _smi_sci_shutdown());
#endif 
}

