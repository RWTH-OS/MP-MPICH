/* $Id$ */

#ifndef SVM

#ifdef WIN32
#define  WIN32_LEAN_AND_MEAN    /* removes uneccessary includes files */
#include <wtypes.h>
#include <winioctl.h>
#endif

#include "store_barrier.h"
#include "dyn_mem/dyn_mem.h"
#include "env/smidebug.h"
#include "env/general_definitions.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

static int             barriers_init = false;
static volatile char*  remote_memory = NULL;
static volatile char  local_memory[16];

#ifdef WIN32
#pragma optimize("", off)
#endif

#if defined(WIN32) && defined(_M_IX86)
#define cpuid __asm _emit 0x0F __asm _emit 0xA2
#endif

/* use the old "manual" barriers or the SISCI calls ? */
#define OLD_BARRIER_CODE 0


#ifdef NO_SISCI
void _smi_init_load_store_barriers()
{
}

void _smi_store_barrier()
{
}

void _smi_local_store_barrier()
{
}

int _smi_load_barrier()
{
  return(SMI_SUCCESS);
}

int _smi_range_store_barrier(volatile void* start, unsigned int size, int home_of_data)
{
  return(SMI_SUCCESS);
}

int _smi_range_load_barrier(volatile void* start, unsigned int size, int home_of_data)
{
  return(SMI_SUCCESS);
}

/* SMPs are cache-coherent -> nothing to flush */
smi_error_t SMI_Flush_read (int process)
{
    return SMI_SUCCESS;
}

smi_error_t SMI_Flush_write (int process)
{
    return SMI_SUCCESS;
}

#else /* NO_SISCI */
/***************************************************************************/
/* _smi_allocate SMI_NbrStreambufs*SMI_StreambufSize Byte of remote memory */
/* (where is not important;                                                */
/* this is enough for the Intel systems as well as the sun systems))       */
/* to do so, search for a process on a remote node                         */
/* the first address of remote memory is ensured to lay in the first byte  */
/* of the first sci buffer                                                 */
/***************************************************************************/
void _smi_init_load_store_barriers()
{
#if OLD_BARRIER_CODE
  int remote_proc;
  int i;
  volatile int* tmp_ptr;
  
  _smi_ll_barrier();
  
  for(i = 0; i < _smi_nbr_procs; i++) {
    if (_smi_all_on_one == false) {
      remote_proc = i + 1;
      if (remote_proc > _smi_nbr_procs-1)
	remote_proc = 0;
      
      while (_smi_machine_rank[i] == _smi_machine_rank[remote_proc]) {
	remote_proc = remote_proc + 1;
	if (remote_proc > _smi_nbr_procs-1)
	  remote_proc = 0;
      }
      SMI_Cmalloc(2*ALLSTREAMS_SIZE, remote_proc|INTERNAL, (char**)&tmp_ptr);
      if(_smi_my_proc_rank == i) {      			 
	remote_memory = (volatile char *) tmp_ptr;
	if ((unsigned int)remote_memory % (ALLSTREAMS_SIZE) != 0)
	  remote_memory = (volatile char*)
	    ((unsigned int)remote_memory 
	     + ((ALLSTREAMS_SIZE)-((unsigned int)remote_memory % (ALLSTREAMS_SIZE))));
      }
    }
    _smi_ll_barrier();
  }
#endif
  
  barriers_init=true;
}

#if defined(WIN32) && defined(_M_AMD64)
extern void __cpuid(int* CPUInfo, int InfoType);
#pragma intrinsic(__cpuid)
#endif

/***************************************************************************/
/* store barrier which is only effective within one Intel-based SMP node   */
/* Amazingly, the 'cpuid' instruction frees as a side-effect all           */
/* processor/cache write buffers.                                          */
/***************************************************************************/
void _smi_intel_processor_store_barrier(void)
{ 
#ifdef WIN32

#ifdef _M_IX86
	/* we can assume that we use Microsoft Wisual C++, which uses the */
	/* way to incorporate an assembler instruction:                   */
	__asm {pusha};
	__asm {cpuid};
	__asm {popa};	 
  
#elif defined(_M_AMD64)
	int CPUInfo[4] = {-1};

	// __cpuid with an InfoType argument of 0 returns the number of
    // valid Ids in CPUInfo[0] and the CPU identification string in
    // the other three array elements. The CPU identification string is
    // not in linear order. The code below arranges the information 
    // in a human readable form.
    __cpuid(CPUInfo, 0);
#else
#error unsupported platform
#endif
	/* fprintf(stderr," @@@ processor store barrier\n");   */
#else /* WIN32 */

#ifdef X86
   /* we assume that we are using a GNU C compiler or something      */
   /* similar which uses the following way to incorporate an         */
   /* into the code:                                                 */ 
   asm ("pusha");
   asm ("cpuid");
   asm ("popa");
#endif

#endif /* WIN32 */
}




/***************************************************************************/
/* store barrier which is only effective within one Sparc-based SMP node   */
/* We simply perform 16 writes, hoping that this flushes all processor/    */
/* cache write buffers                                                     */
/***************************************************************************/
void _smi_sparc_processor_store_barrier(void)
{
#if 0
  int i;

  /* this is the SBUS / SuperSparc version */
  for(i=0;i<16;i++)
      local_memory[i] = (char)i;
#else
#ifdef SPARC
  /* this should do a better job for modern Sparc systems (>= V8) */
  asm ("stbar");
#endif
#endif

}




/***************************************************************************/
/* Architecture-independent SMP-wide store                                 */
/***************************************************************************/
void _smi_local_store_barrier()
{
  
#ifdef X86
      _smi_intel_processor_store_barrier();
#elif SPARC
      _smi_sparc_processor_store_barrier();
#endif  
}






/*****************************************************************************/
/* this functions states a sci-load barrier for a specific memory range that */
/* shall be load fresh afterwards.                                           */
/*****************************************************************************/
int _smi_range_load_barrier(volatile void* start, unsigned int size, int home_of_data)
{
#if OLD_BARRIER_CODE
    /* XXX old code which does a "manual" load barrier */
    int i;
    unsigned int int_start = (unsigned int)start;
    int first_stream, last_stream;
    volatile int sum;
    volatile int val[MAX_NBR_STREAMBUFS];
            
    if (size==0 || _smi_all_on_one == true) 
	return 0;  
    
    if (home_of_data <0 || _smi_machine_rank[home_of_data] != _smi_my_machine_rank) {
	int_start = int_start - ALLSTREAMS_SIZE*(int_start/(ALLSTREAMS_SIZE));
	first_stream = int_start / _smi_StreambufSize;
	last_stream = (int_start + size - 1) / _smi_StreambufSize;
	
	for(i = first_stream; i <= last_stream; i++)
	    val[i] = ((int *)remote_memory)[INTS_PER_STREAM*i];
	for(i = first_stream; i <= last_stream; i++)
	    sum += val[i];
    }
    
    return sum;
#else
    int i;
    
    _smi_local_store_barrier();
    
    if (home_of_data < 0)  {
      for(i = 0; i < _smi_nbr_machines; i++) {
	if (_smi_my_machine_rank != i)
	  SCIFlushReadBuffers(_smi_node_sequence[i]);
      }
    } else {
      if (_smi_machine_rank[home_of_data] != _smi_my_machine_rank)
	SCIFlushReadBuffers(_smi_node_sequence[_smi_machine_rank[home_of_data]]);
    }
    
    return 0;
#endif
}




/******************************************************************************/
/* this functions states a sci-store barrier for a specific memory range that */
/* shall be load fresh afterwards.                                            */
/******************************************************************************/
int _smi_range_store_barrier(volatile void* start, unsigned int size, int home_of_data)
{
#if OLD_BARRIER_CODE
    /* XXX old code which does a "manual" store barrier */
    int i;
    unsigned int int_start = (unsigned int)start;
    int first_stream, last_stream;
    
    /* 
       There has been a funny guy using _smi_range_store_barrier in the routine 
       which initializes the remote_memory. As long as this bug is not fixed
       we ignore _smi_range_store_barrier before initialization
    */
    if (barriers_init == true) {
	_smi_local_store_barrier();
	
	if (size==0 || _smi_all_on_one==true) 
	    return 0;  
	
	if (home_of_data<0 || _smi_machine_rank[home_of_data] != _smi_my_machine_rank) {
	    int_start = int_start - ALLSTREAMS_SIZE*(int_start/ALLSTREAMS_SIZE);
	    first_stream = int_start / _smi_StreambufSize;
	    last_stream = (int_start + size - 1) / _smi_StreambufSize;
	    
	    for(i = first_stream; i <= last_stream; i++)
		((int*)remote_memory)[INTS_PER_STREAM*i + INTS_PER_STREAM-1] = i;
	}
	
	_smi_local_store_barrier();       
	return(remote_memory[0]);
    } else {
	DWARNING("Usage of _smi_range_store_barrier before initialization");
	return 0;
    }
#else
    //sci_error_t			  sci_error;
    int i;
    
    _smi_local_store_barrier();
    
    if (home_of_data < 0)  {
	for(i = 0; i < _smi_nbr_machines; i++) {
	    if (_smi_my_machine_rank != i) {
		SMI_SCIStoreBarrier (_smi_node_sequence[i], 0, &sci_error);
	    }
	}
    } else {
	if (_smi_machine_rank[home_of_data] != _smi_my_machine_rank) {
	    SMI_SCIStoreBarrier(_smi_node_sequence[_smi_machine_rank[home_of_data]], 0, &sci_error);
	}
    }
    
    return 0;
#endif
}






/***************************************************************************/
/* Load barrier for SCI-PCI adapter card                                   */
/***************************************************************************/
int _smi_load_barrier()
{
    if (_smi_all_on_one == true) 
	return(SMI_SUCCESS);
    
    _smi_range_load_barrier(NULL, ALLSTREAMS_SIZE, -1);

    return(SMI_SUCCESS);
}


/***************************************************************************/
/* Store barrier for SCI-SBUS adapter card                                 */
/***************************************************************************/
void _smi_sci_sbus_store_barrier(void)
{
    /* the actual store barrier consists of a remote read operation, */
    /* followed by some operations that ensure that this operations  */
    /* really has finished when returning from this function (e.g.   */
    /* not delayed by some type of out of order execution, ...)      */
    
    *local_memory = *remote_memory;
}




/***************************************************************************/
/* Store barrier for SCI-PCI adapter card                                  */
/***************************************************************************/
void _smi_sci_pci_store_barrier(void)
{
  _smi_range_store_barrier(NULL, ALLSTREAMS_SIZE, -1);
}




/***************************************************************************/
/* System-wide store                                                       */
/***************************************************************************/
void _smi_store_barrier()
{	
    _smi_local_store_barrier();
    
    if (_smi_all_on_one == true) {
      _smi_local_store_barrier();
      return;
    }
    
#ifdef PCI
    _smi_sci_pci_store_barrier();
#elif SBUS
    _smi_sci_sbus_store_barrier();
#endif  
    
}


smi_error_t SMI_Flush_read (int process)
{
    int node, off;

    /* For multiple adapter boards, we need the correct sequence which is the one
       which is bound to the adapter which imports remote segments. */
    off = _smi_adpt_import * _smi_nbr_machines;

    if (process == SMI_FLUSH_ALL)  {
	for (node = 0; node < _smi_nbr_machines; node++) {
	    if (_smi_my_machine_rank != node)
		SCIFlushReadBuffers (_smi_node_sequence[off + node]);
	}
    } else {
	if (_smi_my_machine_rank != _smi_machine_rank[process]) {
	    SCIFlushReadBuffers (_smi_node_sequence[off + _smi_machine_rank[process]]);
	}
    }
    
    return SMI_SUCCESS;
}

smi_error_t SMI_Flush_write (int process)
{
    //sci_error_t	sci_error;
    int node, off;

    /* For multiple adapter boards, we need the correct sequence which is the one
       which is bound to the adapter which imports remote segments. */
    off = _smi_adpt_import * _smi_nbr_machines;

    if (process == SMI_FLUSH_ALL)  {
	for (node = 0; node < _smi_nbr_machines; node++) {
	    if (_smi_my_machine_rank != node) {
		SMI_SCIStoreBarrier (_smi_node_sequence[off + node], 0, &sci_error);
	    }
	}
    } else {
	if (_smi_my_machine_rank != _smi_machine_rank[process]) {
	    SMI_SCIStoreBarrier (_smi_node_sequence[off + _smi_machine_rank[process]], 0, &sci_error);
	}
    }

    return SMI_SUCCESS;
}

#endif /* NO_SISCI */

#ifdef WIN32
#pragma optimize("", on)
#endif


#endif





