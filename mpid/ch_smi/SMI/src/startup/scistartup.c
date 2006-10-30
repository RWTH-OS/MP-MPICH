/* $Id$ */

/* initialize the basic SCI communication facilities */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "env/general_definitions.h"

#ifndef NO_SISCI
#include "env/smidebug.h"
#include "proper_shutdown/resource_list.h"
#include "proper_shutdown/watchdog.h"
#include "env/safety.h"
#include "utility/query.h"
#include "env/smi_init.h"
#include "startup/tcpsync.h"
#include "utility/smi_time.h"
#include "utility/general.h"
#include "memory/shseg_key.h"

#include "startup.h"
#include "tcpsync.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* maximum segment ID that should be used for the base segment
   (before we give up) */
#define MAX_BASESGMT_ID 1024
  
static int  my_proc_rank;
static int  nbr_procs;
static int volatile *basesgmt_addr = (int*)0x0;

/* data for local shared memory area */
static volatile int *sgmt_addr = (int*)0x0;
static int	sgmt_key = 9966;
static int	sgmt_id;
#if WD_TYPE_CALLBACK
static smi_sci_seg_t segment;
#endif

static 	sci_desc_t SCIfd;
static 	sci_error_t SCIerror;
static 	sci_local_segment_t	local_basesgmt;
static 	sci_remote_segment_t	remote_basesgmt;
static 	sci_map_t		basesgmt_map;

static 	int basesgmt_id = 0;
static 	unsigned int basesgmt_offset = 0;

#if BASESEGMENT_DYNAMIC_SIZE 
static 	unsigned int basesgmt_size;
#else
static 	unsigned int basesgmt_size   = BASESGMT_SIZE(i);
#endif
static 	unsigned int basesgmt_flags  = 0;

static  int basesgmt_is_remote = FALSE;

int _smi_check_if_adapter_reaches_all_procs(sci_desc_t SCIfd, int iLocalAdapter, 	   
					    smi_sci_info_t* pSciInfo, 
					    int iNumProcs, int iProcRank);
#if WD_TYPE_CALLBACK
#if USE_THREAD_FOR_WATCHDOG

static _smi_wd_watchdog_cb_callback(sci_segment_cb_reason_t reason, pthread_t wd_master_id) {
    if (_smi_wd_shutdown_in_progress()) {
	_smi_ll_sgmt_ok = FALSE;	
	return SCI_CALLBACK_CANCEL;
    }
        
    switch (reason) {
    case SCI_CB_CONNECT:
	/* update the connection table for the segment */
	break;
    case SCI_CB_DISCONNECT:
	DPROBLEM("a process disconnected from base segment");
	if (!_smi_wd_shutdown_in_progress()) {
	    _smi_ll_sgmt_ok = FALSE;
	    _smi_wd_shutdown_process (wd_master_id, SIGTERM);
	}
	
	return SCI_CALLBACK_CANCEL;
	/*break;*/
    case SCI_CB_OPERATIONAL:
	/* the segment is usable */
	break;
    case SCI_CB_NOT_OPERATIONAL:
	/* the situation may recover */
	break;
    case SCI_CB_LOST:
	DPROBLEM("a process lost connection to base segment");
	if (!_smi_wd_shutdown_in_progress()) {
	    _smi_ll_sgmt_ok = FALSE;
	    _smi_wd_shutdown_process (wd_master_id, SIGTERM);
	}
	return SCI_CALLBACK_CANCEL;
    default:
	DERROR ("invalid segment callback reason!");
	break;
    }
    
    return SCI_CALLBACK_CONTINUE;    
}

static sci_callback_action_t _smi_wd_watchdog_lcb_callback(void* arg,
							   sci_local_segment_t segment,
							   sci_segment_cb_reason_t reason,
							   unsigned int CallerNodeId,
							   unsigned int ReceiverAdapterNo,
							   sci_error_t error) { 
    pthread_t wd_master_id = (pthread_t) arg;   
    return (_smi_wd_watchdog_cb_callback(reason, wd_master_id));
}

static sci_callback_action_t _smi_wd_watchdog_rcb_callback(void* arg,
							   sci_remote_segment_t segment,
							   sci_segment_cb_reason_t reason,
							   sci_error_t status) { 
    
    pthread_t wd_master_id = (pthread_t) arg;
    return (_smi_wd_watchdog_cb_callback(reason, wd_master_id));
}
#else /* USE_THREAD_FOR_WATCHDOG */
#error cannot use watchdog callback without threads enabled
#endif /* USE_THREAD_FOR_WATCHDOG */
#endif /* WD_TYPE_CALLBACK */


int _smi_OpenSCI(sci_desc_t* pSCIfd, int* pSCIid, smi_args_t *sArgs)
{
    DSECTION ("_smi_OpenSCI");
    sci_query_adapter_t adapter_query;
    sci_error_t SCIerror;
    smi_error_t error;
    smi_sci_info_t* pSciInfo;
    tcp_ident_t idLocal;
    int i, ret = SMI_SUCCESS;
    
    DSECTENTRYPOINT;  
    /* SCIInitialize() has been called in _smi_query_init() */
    rs_SCIOpen(pSCIfd,0,&SCIerror);
#ifdef DOLPHIN_SISCI
    if (SCIerror == SCI_ERR_INCONSISTENT_VERSIONS) {
	DERROR ("Version mismatch between SISCI API library and SISCI driver");
    }
#endif
    ABORT_IF_FAIL("SCIOpen() failed", SCIerror, -1);
    
    ALLOCATE(pSciInfo, smi_sci_info_t*, sizeof(smi_sci_info_t)*nbr_procs);
    
    error = SMI_Query(SMI_Q_SCI_NBRADAPTERS, 0, &(pSciInfo[my_proc_rank].iNbrLocalAdapter));
    ASSERT_R((error == SMI_SUCCESS), "could not get number of adapters",SMI_ERR_OTHER);
    ASSERT_A((pSciInfo[my_proc_rank].iNbrLocalAdapter <= MAX_ADAPTER), "too many PCI-SCI adapters installed"
	     , SMI_ERR_OTHER);
    ASSERT_A((pSciInfo[my_proc_rank].iNbrLocalAdapter > 0), "no PCI-SCI adapter found", SMI_ERR_NODEVICE);
    
    for (i = 0; i < pSciInfo[my_proc_rank].iNbrLocalAdapter; i++) {
	error = SMI_Query(SMI_Q_SCI_ID, i, &(pSciInfo[my_proc_rank].SciIds[i])); 
	ASSERT_R((error == SMI_SUCCESS), "could not get SCI ID of an adapter", SMI_ERR_OTHER);
    }
    
    _smi_tcp_mkident (&idLocal, my_proc_rank, (sArgs->iMagicNumber)++, sArgs->szExecName, 0);
    ret = _smi_TcpAllgather (sArgs->szSyncHost, sArgs->iPortNumber,  &(pSciInfo[my_proc_rank]), 
			     pSciInfo, sizeof(smi_sci_info_t), nbr_procs, &idLocal);
    ASSERT_A((ret==0),"Could not contact server",-1);
    
    /* find first adapter that reaches all nodes */
    for (i = 0; i < pSciInfo[my_proc_rank].iNbrLocalAdapter; i++) {
	if (_smi_adapter_available(i))
	    if (_smi_check_if_adapter_reaches_all_procs (*pSCIfd, i, pSciInfo, nbr_procs, my_proc_rank) == FALSE)
		_smi_set_adapter_unavailable(i);
    }
    DNOTICEI("nbr of local PCI-SCI adapters:",pSciInfo[my_proc_rank].iNbrLocalAdapter);
    
    for (i = 0; i < pSciInfo[my_proc_rank].iNbrLocalAdapter; i++)
	if (_smi_adapter_available(i))
	    break;
    
    ASSERT_A((i < pSciInfo[my_proc_rank].iNbrLocalAdapter),"Cannot reach all nodes!",SMI_ERR_OTHER);
    _smi_DefAdapterNbr = i; 
    DNOTICEI("First adapter that reaches all nodes:",_smi_DefAdapterNbr);
    
    adapter_query.localAdapterNo = _smi_DefAdapterNbr;
    adapter_query.subcommand     = SCI_Q_ADAPTER_NODEID;
    adapter_query.data           = (void *)pSCIid;
    SCIQuery(SCI_Q_ADAPTER, (void *)&adapter_query, 0, &SCIerror);
    ABORT_IF_FAIL("no SCI adapter found", SCIerror, -1);
    
    free(pSciInfo);
    
    DSECTLEAVE;
    return(ret);
}

/* probe the SCI sync node and get some information on the local PCI-SCI adapter(s) */
int _smi_QuerySCI (sci_desc_t SCIfd, int sync_id) {
    int i, ret;
    
    DSECTION ("_smi_QuerySCI");
    DSECTENTRYPOINT;
    
    /* probe SCI sync node */
    if (my_proc_rank != 0) {
	/* workaround for initial sci-ping problems: driver is loaded dynamically on the first
	   access and returns "NO_LINK_ACCESS" for some time until the SCI link is established */
	i = 0;
	do {
	    i++;
	    ret = SCIProbeNode(SCIfd, _smi_DefAdapterNbr, sync_id, 0, &SCIerror);
	    if (!ret) {
		DNOTICEI ("Could not reach sync node, trying again. SCI-ID:", sync_id);
		sleep (SCI_PROBE_DELAY);
	    }
	} while ( !ret && (i < SCI_PROBE_TRIES));
	if (!ret) {
	    DERRORI ("SCI sync node not reachable, SCI ID", sync_id);
	    SMI_Abort((SMI_ERR_OTHER));
	}
    }
    
    
    /* build a printable type of the PCI-SCI adapter type. If multiple adapters are
       present, they need to be of the same type. */
    ret = SMI_Query (SMI_Q_SCI_ADAPTERTYPE, 0, &_smi_AdapterType);
    ASSERT_R(ret == SMI_SUCCESS, "SMI_Query(SMI_Q_SCI_ADAPTERTYPE) failed", SMI_ERR_OTHER);
    switch (_smi_AdapterType) {
    case 304:
    case 307:
	/* are these supported bye the drivers any longer ? */
	strcpy (_smi_AdapterTypeS, "D304/D307 (LC-2, SBUS)");
	DPROBLEMI ("SCI Adaptertype not supported: D", _smi_AdapterType);
	DSECTLEAVE;
	return (SMI_ERR_NODEVICE);
	break;
    case 308:
    case 310:
    case 312:
	strcpy (_smi_AdapterTypeS, "D310/D311/D312 (LC-2, 32bit PSB)");
	break;
    case 320:
    case 321:
	strcpy (_smi_AdapterTypeS, "D320/D321 (LC-2, 64bit PSB)");
	break;
    case 330:
    case 331:
    case 332:
	sprintf (_smi_AdapterTypeS, "D%d (Single LC-3, 64bit PSB)", _smi_AdapterType);
	break;
    case 334:
    case 335:
	sprintf (_smi_AdapterTypeS, "D%d (Dual LC-3 (1 slot), 64bit PSB)", _smi_AdapterType);
	break;
    case 336:
	strcpy (_smi_AdapterTypeS, "D336 (Triple LC-3 (1 slot), 64bit PSB)");
	break;
    case 337:
	strcpy (_smi_AdapterTypeS, "D337 (Triple LC-3 (2 slots), 64bit PSB)");
	break;
    case 339:
	strcpy (_smi_AdapterTypeS, "D339 (Dual LC-3 (2 slots), 64bit PSB)");
	break;
    case 350:
	strcpy (_smi_AdapterTypeS, "D350 (Dual-Link LC-3 (1 slot), 64bit PSB via PCI-Express)");
	break;
    case 351:
	strcpy (_smi_AdapterTypeS, "D350 (Single LC-3 (1 slot), 64bit PSB via PCI-Express)");
	break;
    case 352:
	strcpy (_smi_AdapterTypeS, "D350 (Dual LC-3 (1 slot), 64bit PSB via PCI-Express)");
	break;
    case 353:
	strcpy (_smi_AdapterTypeS, "D350 (Triple LC-3 (2 slot), 64bit PSB via PCI-Express)");
	break;
    default: 
	strcpy (_smi_AdapterTypeS, "unknown or not supported type of PCI-SCI adapter");
	DPROBLEMI ("SCI Adaptertype not supported: D", _smi_AdapterType);
	break;
    }
    
    /* get the current number of stream buffers */  
    ret = SMI_Query (SMI_Q_SCI_NBRSTREAMBUFS, 0, &_smi_NbrStreambufs);
    ASSERT_R(ret == SMI_SUCCESS, "SMI_Query(SMI_Q_SCI_NBRSTREAMBUFS) failed", SMI_ERR_OTHER);
    if (_smi_NbrStreambufs > MAX_NBR_STREAMBUFS) {
	DERRORI ("Too many stream buffers:", _smi_NbrStreambufs);
	DNOTICE ("   adjust MAX_NBR_STREAMBUFS in general_definitions.h and recompile library");
	SMI_Abort (-1);
    }
    
    /* get the size of the stream buffers */  
    ret = SMI_Query (SMI_Q_SCI_STREAMBUFSIZE, 0, &_smi_StreambufSize);
    ASSERT_R(ret == SMI_SUCCESS, "SMI_Query(SMI_Q_SCI_STREAMBUFSIZE) failed", SMI_ERR_OTHER);
    
    DSECTLEAVE;
    return SMI_SUCCESS;
}

static int _smi_probe_node(sci_desc_t SCIfd, int iLocalAdapter, int iSciId)
{ 
    int i = 0;
    int ret;
    
    DSECTION ("_smi_probe_node");
    DSECTENTRYPOINT;

    /* workaround for initial sci-ping problems: driver is loaded dynamically on the first
       access and returns "NO_LINK_ACCESS" for some time until the SCI link is established */
    do {
	i++;
	DNOTICEI("Probing SCI node ", iSciId);
	ret = SCIProbeNode(SCIfd, iLocalAdapter, iSciId, 0, &SCIerror);
	if (!ret) {
	    DPROBLEMI ("SCI node unreachable: ", iSciId);
	    DPROBLEMP ("SCIProbeNode failed, SISCI error ", SCIerror);       	    
	    sleep (SCI_PROBE_DELAY);
	}
    } while ( !ret && (i < SCI_PROBE_TRIES));

    DSECTLEAVE; return (ret);
}

int _smi_check_if_adapter_reaches_all_procs(sci_desc_t SCIfd, int iLocalAdapter, 	   
					    smi_sci_info_t* pSciInfo, 
					    int iNumProcs, int iProcRank)
{
    DSECTION("_smi_check_if_adapter_reaches_all_procs");
#ifdef DOLPHIN_SISCI
    int i,j;
    int reachable;    
    
    DSECTENTRYPOINT;

    DNOTICEI("testing local adapter:", iLocalAdapter);

    for (i = 0; i < iNumProcs; i++) {
	DNOTICEI("trying if process is reachable:", i);
	if (pSciInfo[i].SciIds[0] != pSciInfo[iProcRank].SciIds[0]) {
	    reachable = 0;
	    for (j = 0; j < pSciInfo[i].iNbrLocalAdapter; j++) {
		DNOTICEI("checking if remote adapter is reachable:", j);
		if (_smi_probe_node(SCIfd, iLocalAdapter, pSciInfo[i].SciIds[j]) == TRUE) {
		    DNOTICE("...yes");
		    reachable++;
		}
		else {
		    DNOTICE("...no");
		}
		if (reachable > 0)
		    break;
	    }
	    if (reachable == 0) {
	      DWARNING("Processes can not talk to each other via SCI!");
		DSECTLEAVE return(FALSE);
	    }
	}
    }
#elif defined SCALI_SISCI
    DWARNING ("Node-probing disabled due to long delays in Scali-SISCI for SCIProbeNode()");
    DWARNING ("-> returning TRUE");
#endif
    DSECTLEAVE return(TRUE);
}

smi_error_t _smi_sci_startup(smi_args_t *sArgs)
{
    DSECTION("_smi_sci_startup");
    tcp_ident_t idLocal;
    int  pTrans[2], sync_id, i, ret;
    smi_error_t error;
    double start_secs;
    DSECTENTRYPOINT;
    
    sync_id      = sArgs->iSCIId;
    my_proc_rank = sArgs->iProcRank;
    nbr_procs    = sArgs->iNumProcs;
#if BASESEGMENT_DYNAMIC_SIZE 
    basesgmt_size = _smi_power_of_2_ge(BASESGMT_SIZE(nbr_procs),(64*1024));
#endif

    /* Currently only tcpstartup is supported */
    ASSERT_R((sArgs->eStartupMethod == smi_startup_use_tcp),"tcp startup mechanism is required!",
	     SMI_ERR_NOTIMPL);
    if (_smi_tcp_init() == -1) {
	DERROR("could not init tcp");
	SMI_Abort(-1);
    }
    
    /* allocate sufficient storage to hold all sci-ranks of all processes 
       and get default local adapter and its SCI id. The SCI id's of the other
       processes are detemined in _smi_init_sci_subsystem() */
    ALLOCATE(_smi_sci_rank, int *, nbr_procs*sizeof(int));
    _smi_OpenSCI(&SCIfd, &(_smi_sci_rank[my_proc_rank]), sArgs);
    
    /* fill the whole array with my ID - the AllGather on this array which will be
       done later will operate with the real process rank and not with the sync rank 
       which might cause the exchange of other array elements then sci_rank[my_procrank] */
    for (i = 0; i < nbr_procs; i++) 
	_smi_sci_rank[i] = _smi_sci_rank[my_proc_rank];
    

    DNOTICE("Allocating local shared memory area (all processes)");
    do {
	sgmt_key = _smi_modify_key(sgmt_key);
	error = rs_shmget(sgmt_key, BASESGMT_SIZE(nbr_procs), IPC_CREAT|IPC_EXCL|0600);
    } while (error == -1);
    sgmt_id = error;
    
    if (my_proc_rank == 0) {
	basesgmt_is_remote = FALSE;
	
	/* this process creates the base segment */
	_smi_QuerySCI (SCIfd, sync_id);
	
#ifdef DOLPHIN_SISCI
	DNOTICE("Allocating base SCI segment located on this machine");
	for (basesgmt_id = 1; basesgmt_id < MAX_BASESGMT_ID; basesgmt_id++) {
#if (defined(WD_TYPE_CALLBACK)) && (USE_IMPLICIT_LOCALSEG_CALLBACK)
	    rs_SCICreateSegment(SCIfd, &local_basesgmt, basesgmt_id, basesgmt_size, 
				_smi_wd_watchdog_lcb_callback, (void*) pthread_self(), SCI_FLAG_USE_CALLBACK, &SCIerror);
#else
	    rs_SCICreateSegment(SCIfd, &local_basesgmt, basesgmt_id, basesgmt_size, 
				NULL, NULL, 0, &SCIerror);
#endif	    
	    if (SCIerror == SCI_ERR_OK)
		break;
	    else {
		DWARNING("failed to create base SCI segment, using next SCI id");
		rs_SCIClose(SCIfd, 0, &SCIerror);
		rs_SCIOpen (&SCIfd, 0, &SCIerror);
		EXIT_IF_FAIL("initial SCIOpen failed (loop)", SCIerror, 1);
	    }
	}
	
	if (basesgmt_id == MAX_BASESGMT_ID) {
	    DERROR("fatal error: could not create base SCI segment ");
	    SMI_Abort(-1);
	}
	
	SCIPrepareSegment(local_basesgmt,_smi_DefAdapterNbr,0,&SCIerror);
	EXIT_IF_FAIL("SCIPrepareSegment for base segment failed", SCIerror, 2);    
	
	DNOTICEI("mapping local base segment, ID =",basesgmt_id);
	basesgmt_addr = (int*)rs_SCIMapLocalSegment(local_basesgmt, &basesgmt_map, basesgmt_offset, 
                                                    basesgmt_size, (void *)basesgmt_addr, 
                                                    basesgmt_flags, &SCIerror);
	EXIT_IF_FAIL("SCIMapLocalSegment for base segment failed", SCIerror, 3);
	DNOTICEP("address for local base segment =", basesgmt_addr);

	DNOTICE("exporting the base segment...");
	SCISetSegmentAvailable(local_basesgmt,_smi_DefAdapterNbr,0,&SCIerror);
	EXIT_IF_FAIL("SCISetSegmentAvailable for base segment failed", SCIerror, 40);
#else
	/* Scali's SISCI has another semantic of the segment IDs - this is a work-around */
	DNOTICE("Allocating base SCI segment located on this machine");
	for (basesgmt_id = 1; basesgmt_id < MAX_BASESGMT_ID; basesgmt_id++) {
	    rs_SCICreateSegment(SCIfd, &local_basesgmt, basesgmt_id, basesgmt_size, 
				NULL, NULL, 0, &SCIerror);
	    EXIT_IF_FAIL("SCICreateSegment for base segment failed", SCIerror, 1);      
	    
	    SCIPrepareSegment(local_basesgmt,_smi_DefAdapterNbr , 0, &SCIerror);
	    EXIT_IF_FAIL("SCIPrepareSegment for base segment failed", SCIerror, 2);
	    
	    DNOTICE("exporting the base segment...");
	    SCISetSegmentAvailable(local_basesgmt, _smi_DefAdapterNbr, 0, &SCIerror);
	    if (SCIerror != SCI_ERR_OK) {
		DWARNING("failed to export base SCI segment, using next segment id");
		rs_SCIRemoveSegment(local_basesgmt, 0, &SCIerror);
		rs_SCIClose(SCIfd, 0, &SCIerror);
		rs_SCIOpen (&SCIfd, 0, &SCIerror);
		EXIT_IF_FAIL("initial SCIOpen failed (loop)", SCIerror, 1);
	    } else
		break;
	}
	
	if (basesgmt_id == MAX_BASESGMT_ID) {
	    DERROR("fatal error: could not create base SCI segment ");
	    SMI_Abort(-1);
	}
	
	DNOTICEI("mapping local base segment, ID =",basesgmt_id);
	basesgmt_addr = (int*)rs_SCIMapLocalSegment(local_basesgmt, &basesgmt_map, basesgmt_offset, 
                                                    basesgmt_size, (void *)basesgmt_addr, 
                                                    basesgmt_flags, &SCIerror);
	EXIT_IF_FAIL("SCIMapLocalSegment for base segment failed", SCIerror, 3);
	DNOTICEP("address for local base segment =", basesgmt_addr);      
#endif
	
	for(i = 0 ; i < nbr_procs; i++)
	    SEC_SET(basesgmt_addr[BASESGMT_OFFSET_BARRIER+ i*INTS_PER_STREAM],DEP_FALSE(i));
	
	DNOTICE("Initializing flushing");
	_smi_init_flush(basesgmt_addr, basesgmt_is_remote ? basesgmt_map : 0);
	
	if (_smi_use_watchdog) {
	    DNOTICE("Initializing watchdog");
	    /* initialize, but do not yet start the watchdog (some other structures need to
	       get initialized first) */
#ifdef WD_TYPE_CALLBACK
	    segment.pSegment.local = &local_basesgmt;
	    segment.type = localseg; 
	    segment.mainthread = pthread_self();
	    _smi_init_watchdog_cb(my_proc_rank, nbr_procs, basesgmt_addr +
				  BASESGMT_OFFSET_ALIVECHECK, &segment); 
#else
	    _smi_init_watchdog(my_proc_rank, nbr_procs, basesgmt_addr + BASESGMT_OFFSET_ALIVECHECK); 
#endif
	    _smi_wd_disable();
	}
	
	DNOTICE("broadcasting ID of base segment...");
	pTrans[0] = basesgmt_id;
	pTrans[1] = _smi_sci_rank[my_proc_rank];
	
	_smi_tcp_mkident(&idLocal,my_proc_rank,(sArgs->iMagicNumber)++,sArgs->szExecName, sgmt_key);
	ret=_smi_TcpBroadcast(sArgs->szSyncHost, sArgs->iPortNumber,
			      pTrans, sizeof(int)*2, 0, nbr_procs, &idLocal);
	ASSERT_A((ret==0),"Could not contact all clients",-1);
    } 
    else {
	basesgmt_is_remote = TRUE;
	
	/* this process connects to the remote base segment */
	DNOTICE("getting ID of base segment..."); 
	
	
	_smi_tcp_mkident(&idLocal, my_proc_rank, (sArgs->iMagicNumber)++,
			 sArgs->szExecName, sgmt_key);
	ret = _smi_TcpBroadcast(sArgs->szSyncHost, sArgs->iPortNumber, pTrans,
			      sizeof(int)*2, 0, nbr_procs, &idLocal);
	ASSERT_A((ret==0),"Could not contact server",-1);
	
	
	/* am i first proc on node? */
	if (idLocal.SmpInfo.iLocalProcRank != 0) { /* No */
	    /* remove unrequired shared memory */
	    rs_shmctl(sgmt_id, IPC_RMID, NULL);
	    sgmt_key = idLocal.SmpInfo.iLocalMemKey;
	    start_secs = _smi_get_seconds();
	    do {
		error = rs_shmget(sgmt_key, BASESGMT_SIZE(nbr_procs), 0600);
	    } while ((error == -1) && (_smi_get_seconds() - start_secs <= SMP_STARTUP_TIMEOUT));
	    
	    sgmt_id = error;
	}

	DNOTICEI("iLocalProcRank",idLocal.SmpInfo.iLocalProcRank);
	DNOTICEI("iLocalProcSize",idLocal.SmpInfo.iLocalProcSize);
	DNOTICEI("iLocalMemKey",idLocal.SmpInfo.iLocalMemKey);
	DNOTICEI("iNodeRank",idLocal.SmpInfo.iNodeRank);
	DNOTICEI("iNodeSize",idLocal.SmpInfo.iNodeSize);
	
	basesgmt_id = pTrans[0];
	sync_id     = pTrans[1];
	_smi_QuerySCI (SCIfd, sync_id);
	
	DNOTICEI("attempting to connect to base segment, ID =", basesgmt_id);
	i = 0;
	do {
	    i++; 
#if (defined(WD_TYPE_CALLBACK)) && (USE_IMPLICIT_REMOTESEG_CALLBACK)
	    rs_SCIConnectSegment( SCIfd, &remote_basesgmt, sync_id, basesgmt_id,
				  _smi_DefAdapterNbr,
				  _smi_wd_watchdog_rcb_callback, (void*) pthread_self(),
				  SCI_INFINITE_TIMEOUT, SCI_FLAG_USE_CALLBACK, &SCIerror);
#else
	    rs_SCIConnectSegment( SCIfd, &remote_basesgmt, sync_id, basesgmt_id, _smi_DefAdapterNbr, NULL, NULL,
				  SCI_INFINITE_TIMEOUT, 0, &SCIerror);
#endif
	    if (SCIerror != SCI_ERR_OK)
		usleep(SCI_CONNECT_DELAY);
	} while (((SCIerror == SCI_ERR_CONNECTION_REFUSED) || (SCIerror == SCI_ERR_TIMEOUT)) 
		 && (i < SCI_CONNECT_TRIES));
	EXIT_IF_FAIL("SCIConnectSegment for base segment failed", SCIerror, 5);
	DNOTICE("connection established!");
	
	DNOTICE("mapping base segment ...");
	basesgmt_addr = (int*)rs_SCIMapRemoteSegment(remote_basesgmt, &basesgmt_map, basesgmt_offset, 
                                                     basesgmt_size, (void *)basesgmt_addr, 
                                                     basesgmt_flags, &SCIerror);
	EXIT_IF_FAIL("SCIMapRemoteSegment for base segment failed", SCIerror, 6);
	DNOTICEP("address of base segment =", basesgmt_addr);
	
	DNOTICE("Initializing flushing");
	_smi_init_flush(basesgmt_addr, basesgmt_is_remote ? basesgmt_map : 0);
	
	if (_smi_use_watchdog) {
	    DNOTICE ("Starting watchdog");
#ifdef WD_TYPE_CALLBACK
	    segment.pSegment.remote = &remote_basesgmt;
	    segment.type = remoteseg;
	    segment.mainthread = pthread_self();
	    _smi_init_watchdog_cb(my_proc_rank, nbr_procs, basesgmt_addr +
				  BASESGMT_OFFSET_ALIVECHECK, &segment); 
#else
	    _smi_init_watchdog(my_proc_rank, nbr_procs, basesgmt_addr + BASESGMT_OFFSET_ALIVECHECK); 
#endif
	    _smi_wd_disable();
	}
    }    
    
    ASSERT_X((sgmt_id != -1),"Could not create local shmem segment",SMI_ERR_NOMEM);
    
    /* Map the local shared Memory */
    sgmt_addr = (int*)shmat(sgmt_id,(char*)sgmt_addr,0600);
    ASSERT_A((size_t)sgmt_addr != -1, "Could not map local shmem segment", SMI_ERR_MAPFAILED);
    DNOTICEP("Mapped base segment to: ",sgmt_addr);
    
    DNOTICE ("Startup parameters:");
    DNOTICEI("   sync node SCI ID       :", sync_id);
    DNOTICEI("   this process sync rank :", my_proc_rank);
    DNOTICEI("   total nbr of processes :", nbr_procs);
    DNOTICE ("Adapter information:");
    DNOTICES("   PCISCI adapter type    :", _smi_AdapterTypeS);
    DNOTICEI("   nbr of adapters        :", _smi_nbr_adapters[my_proc_rank]);
    DNOTICEI("   SCI ID of default adptr:", _smi_sci_rank[my_proc_rank]);
    DNOTICEI("   nbr of streambuffers   :", _smi_NbrStreambufs);
    DNOTICEI("   size of streambuffers  :", _smi_StreambufSize);
    DNOTICE("");
    
    DNOTICE("Initialising low level message passing");
    _smi_ll_init(idLocal.SmpInfo.iNodeRank,
		  idLocal.SmpInfo.iNodeSize, basesgmt_addr, 
		  idLocal.SmpInfo.iLocalProcRank,
		  idLocal.SmpInfo.iLocalProcSize, sgmt_addr,
		  my_proc_rank, nbr_procs);
    
    SEC_SET(basesgmt_addr[SEC_INDEX(my_proc_rank)+BASESGMT_OFFSET_INIT],0);
    
    DNOTICE ("Waiting for other processes");
    _smi_ll_barrier();
    DNOTICE ("SCI startup complete!");
    
    _smi_tcp_finalize();
       
    DSECTLEAVE; 
    return (0);
}


smi_error_t _smi_sci_shutdown(void)
{
    int loops;
    
    DSECTION("_smi_sci_shutdown");
    DSECTENTRYPOINT;    

    if (basesgmt_is_remote == FALSE) {
	DNOTICE("unmapping base segment");
	rs_SCIUnmapSegment(basesgmt_map, 0, &SCIerror);
	EXIT_IF_FAIL("SCIUnmapSegment for base segment failed", SCIerror, 7);
	DNOTICE("removing base segment");
	/* only wait for a limited time for the other processes to discnnect*/
	for (loops = 0; loops < SCI_REMOVE_TRIES; loops++) {
	    rs_SCIRemoveSegment(local_basesgmt, 0, &SCIerror);
	    if (SCIerror == SCI_ERR_OK)
		break;
	    else
		usleep (SCI_REMOVE_DELAY);
	}
    } 
    else {
	DNOTICE("unmapping base segment");
	rs_SCIUnmapSegment(basesgmt_map, 0, &SCIerror);
	EXIT_IF_FAIL("SCIUnmapSegment for base segment failed", SCIerror, 7);
	DNOTICE("disconnecting base segment");
	rs_SCIDisconnectSegment(remote_basesgmt, 0, &SCIerror);
	EXIT_IF_FAIL("SCIDisconnectSegment for base segment failed", SCIerror, 9);
    }
    
    DNOTICE("Closing base SCI device descriptor");
    rs_SCIClose(SCIfd,0,&SCIerror);
    EXIT_IF_FAIL("SCIClose for base segment failed", SCIerror, 10);
   
    DNOTICE ("SCI finalization complete!");
    
    DSECTLEAVE;
    return(0);
}

#endif
