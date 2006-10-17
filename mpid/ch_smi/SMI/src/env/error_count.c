/* $Id$ */


#include "error_count.h"
#include "utility/statistics.h"
#include "utility/query.h"
#include "general_definitions.h"
#include "regions/region_layout.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif


#define SCI_SEQ_OK_RETRIES 60
#define DO_VERIFY          1           /* disable all verifcations by setting to 0 - ONLY FOR TESTING! */

#define ADPT_OFFSET        (_smi_adpt_import*_smi_nbr_machines)

/* exports */
#ifndef NO_SISCI
sci_sequence_t *_smi_node_sequence;   /* one sequence towards every node */
#endif
boolean *_smi_error_seq_initialized;

static boolean connection_ok;

#ifndef NO_SISCI
/* returns the result for all remote-memory transfers since the last call to
   this function */
smi_error_t SMI_Check_transfer(int flags)
{
  REMDSECTION("SMI_Check_transfer");
  int node;

  DSECTENTRYPOINT;
#if DO_VERIFY
  if (_smi_all_on_one) {
      DSECTLEAVE; return (SMI_SUCCESS);
  }

  node = (_smi_my_machine_rank + 1 )% _smi_nbr_machines;
  if (!_smi_error_seq_initialized[node]) {
    /* this means no remote SCI segments has been mapped */
      DSECTLEAVE; return (SMI_SUCCESS);
  }
  
  return _smi_check_transfer (_smi_node_sequence[ADPT_OFFSET + node], flags);
#else
  return (SMI_SUCCESS);
#endif
}

/* returns the result for all remote-memory transfers since the last call to
   this function */
smi_error_t SMI_Check_transfer_addr(void *addr, int flags)
{
  REMDSECTION("SMI_Check_transfer");
  int node;

  DSECTENTRYPOINT;

#if DO_VERIFY
  if (_smi_all_on_one) {
    DSECTLEAVE; return (SMI_SUCCESS);
  }
  /* XXX This is not always bullet-proof! Only for undivided segments - but for
     blocked segments spread across mulitple nodes, we might check not for all
     nodes concerned. The question is: do we have to check all nodes to detect
     an error? */
  node = _smi_address_to_node(addr);
  ASSERT_R ((node >= 0), "No SMI address!", SMI_SUCCESS);
  if (!_smi_error_seq_initialized[node] || node == _smi_my_machine_rank) {
    /* this means no remote SCI segments has been mapped */
    DSECTLEAVE; return (SMI_SUCCESS);
  }
  
  DSECTLEAVE; 
  return _smi_check_transfer (_smi_node_sequence[ADPT_OFFSET + node], flags);
#else
  DSECTLEAVE; return (SMI_SUCCESS);
#endif
}

smi_error_t SMI_Check_transfer_proc(int proc, int flags )
{
    REMDSECTION("SMI_Check_transfer_proc");
    int node;

    DSECTENTRYPOINT;

#if DO_VERIFY
    node = _smi_machine_rank[proc];
    if (!_smi_error_seq_initialized[node] || node == _smi_my_machine_rank) {
	/* this means no remote SCI segments has been mapped */
	DSECTLEAVE; return (SMI_SUCCESS);	
    }
    DSECTLEAVE; 
    return _smi_check_transfer (_smi_node_sequence[ADPT_OFFSET + node], flags);
#else
    DSECTLEAVE; return (SMI_SUCCESS);
#endif
}

/* Check transfer for a given region towards the specified proc. If proc is SMI_PROC_ALL,
   check for all processes which are relevant to this region.  */
smi_error_t SMI_Check_transfer_region(int region_id, int proc, int flags )
{
    return SMI_SUCCESS;
}


/* only returns the state of the internal SMI variable. */
smi_error_t _smi_get_connection_state() {
    return ( connection_ok ? SMI_SUCCESS : SMI_ERR_PENDING );
}

/* really determine the connection state. The current implementation will
   reset the error counter - this should be considered! */
smi_error_t _smi_probe_connection_state(int proc) {
    sci_error_t sci_error;
    sci_sequence_status_t seq_state;

    DSECTION("_smi_probe_connection_state");

    if(_smi_all_on_one == TRUE) {
	return SMI_SUCCESS;
    }

    /* sequence checking makes no sense for processes on my local machine, I assume */
    seq_state = _smi_error_seq_initialized[ADPT_OFFSET + _smi_machine_rank[proc]] ? 
	SCICheckSequence(_smi_node_sequence[ADPT_OFFSET + _smi_machine_rank[proc]], 0, &sci_error) : SCI_SEQ_OK ;
    
    if (seq_state != SCI_SEQ_OK) {
	DWARNINGI ("SCI connection problem with node:", _smi_machine_rank[proc]);
	DWARNINGI ("              sequence status is:", seq_state);
    }
    return (seq_state == SCI_SEQ_OK ? SMI_SUCCESS : SMI_ERR_TRANSFER);
}


smi_error_t _smi_init_error_counter(int nbr_nodes)
{
    int i;

    connection_ok = true;

    ALLOCATE(_smi_node_sequence, sci_sequence_t *, nbr_nodes*sizeof(sci_sequence_t)*_smi_nbr_adapters[_smi_my_proc_rank]);
    ALLOCATE (_smi_error_seq_initialized, boolean *, nbr_nodes*sizeof(boolean)*_smi_nbr_adapters[_smi_my_proc_rank]);
    for (i = 0; i < nbr_nodes*_smi_nbr_adapters[_smi_my_proc_rank]; i++)
	_smi_error_seq_initialized[i] = FALSE;

    return SMI_SUCCESS;
}

/* internal variant: returns the result for all remote-memory transfers since 
   the last call to this function for the given sequence */
smi_error_t _smi_check_transfer( sci_sequence_t seq, int flags)
{
    REMDSECTION("_smi_check_transfer");
    sci_error_t sci_error;
    sci_sequence_status_t seq_state;
    int retries = 0;

    if (!seq)
	/* for SMP or local segment, we need no sequences  */
	return (SMI_SUCCESS);

#ifdef SCALI_SISCI
    SCIStoreBarrier(seq, 0, &sci_error);
#endif
    SMI_STAT_COUNT(err_check);
    seq_state = SCICheckSequence(seq, flags, &sci_error);
    switch (seq_state) {
    case SCI_SEQ_OK: 	
      connection_ok = true;
      break;

    case SCI_SEQ_RETRIABLE:
	SMI_STAT_COUNT(err_rtrbl);
	_smi_sci_errcnt++;
	connection_ok = true;
	DWARNING ("SCI transfer error - retry necessary");
	return (SMI_ERR_TRANSFER);

    case SCI_SEQ_NOT_RETRIABLE:
	SMI_STAT_COUNT(err_not_rtrbl);
	_smi_sci_errcnt++;
	connection_ok = false;

	if (flags & SMI_CHECK_PROBE)
	    return (SMI_ERR_TRANSFER);

	/* fatal situation */
	DWARNING ("SCI transfer error SCI_SEQ_NOT_RETRIABLE - trying to re-connect");
	do {
	    SMI_STAT_COUNT(err_not_rtrbl_retry);
	    seq_state = SCIStartSequence(seq, 0, &sci_error);
	    sleep (1);
	    retries++;
	} while ((seq_state != SCI_SEQ_OK) && (retries < SCI_SEQ_OK_RETRIES) );
	if (seq_state != SCI_SEQ_OK) {
	    DERROR ("SCI connection lost - Aborting.");
	    SMI_Abort(-1);
	}
	connection_ok = true;

	DNOTICE ("SCI connection re-established");
	return (SMI_ERR_TRANSFER);
	break;

    case SCI_SEQ_PENDING:
	SMI_STAT_COUNT(err_pndng);
	_smi_sci_errcnt++;
	connection_ok = false;

	if (flags & SMI_CHECK_PROBE)
	    return (SMI_ERR_TRANSFER);
	
	DWARNING ("pending SCI transfer error - restarting sequence");
	/* treat this error as described in the SISCI documentation */
	do {
	    SMI_STAT_COUNT(err_pndng_retry);
	    seq_state = SCIStartSequence(seq, 0, &sci_error);
	} while (seq_state == SCI_SEQ_PENDING);

	switch (seq_state) {
	case SCI_SEQ_NOT_RETRIABLE:
	    /* XXX needs to be improved ! */
	    SMI_STAT_COUNT(err_pndng_not_rtrbl);
#if 1
	    do {
		SMI_STAT_COUNT(err_pndng_not_rtrbl_retry);
		if (retries == 0)
		    DWARNING("SCI session invalid (pending sequence returned SCI_NOT_RETRIABLE) - retrying");
		seq_state = SCIStartSequence(seq, 0, &sci_error);
		sleep (1);
		retries++;
	    } while ((seq_state != SCI_SEQ_OK) && (retries < SCI_SEQ_OK_RETRIES) );
#endif
	    if (seq_state != SCI_SEQ_OK) {
		DERROR ("SCI connection lost - Aborting.");
		SMI_Abort(-1);
	    }
	    DWARNING("SCI session OK after SCI_NOT_RETRIABLE - continuing");

	    /* we can continue, but the transmission needs to be repeated */
	    connection_ok = true;
	    return (SMI_ERR_TRANSFER);
	    break;
	case SCI_SEQ_OK:
	    /* we can continue, but the transmission needs to be repeated */
	    SMI_STAT_COUNT(err_pndng_ok);
	    connection_ok = true;
	    return (SMI_ERR_TRANSFER);
	    break;
	}
    }

    return (SMI_SUCCESS);
}


#else

smi_error_t SMI_Get_error_counter(int *count)
{
  *count = 0;
  return(SMI_SUCCESS);
}

smi_error_t SMI_Check_transfer(int flags)
{
  return SMI_SUCCESS;
}

smi_error_t SMI_Check_transfer_proc(int proc, int flags )
{
  return SMI_SUCCESS;
}

smi_error_t SMI_Check_transfer_addr(void *addr, int flags)
{
  return SMI_SUCCESS;
}

smi_error_t _smi_get_connection_state(void) 
{
  return SMI_SUCCESS;
}

smi_error_t _smi_probe_connection_state(int proc) 
{
  return SMI_SUCCESS;
}

smi_error_t _smi_check_transfer( void )
{
  return SMI_SUCCESS;
}

smi_error_t _smi_init_error_counter(int nbr_nodes)
{
  return SMI_SUCCESS;
}

#endif

