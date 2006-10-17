/* $Id$ */

#include "env/smidebug.h"
#include "env/general_definitions.h"
#include "startup.h"
#include "sciflush.h"

#ifdef WIN32
#pragma optimize( "",off)
#endif


#if defined(WIN32) && defined(_M_IX86)
#define cpuid __asm _emit 0x0F __asm _emit 0xA2
#endif /* WIN32 */


static int volatile *mpisgmt_addr = NULL;
#ifndef NO_SISCI
sci_sequence_t _smi_mpisgmt_seq = 0;
#endif /* NO_SISCI */

#if defined(WIN32) && !defined(_M_IX86)
extern void __cpuid(int* CPUInfo, int InfoType);
#pragma intrinsic(__cpuid)
#endif

/***************************************************************************/
/* store barrier which is only effective within one Intel-based SMP node   */
/* Amazingly, the 'cpuid' instruction frees as a side-effect all           */
/* processor/cache write buffers.                                          */
/***************************************************************************/
static void _smi_intel_store_barrier(void)
{
#ifdef WIN32

#ifdef _M_IX86
  /* we can assume that we use Microsoft Wisual C++, which uses the */
  /* way to incorporate an assembler instruction:                   */
   __asm {cpuid}                 
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

#else /* WIN32 */

#ifdef X86
   /* we assume that we are using a GNU C compiler or something      */
   /* similar which uses the following way to incorporate an         */
   /* assembler instruction into the code:                           */ 
#ifdef LINUX
   asm ("pusha"); 
#endif
   asm ("cpuid");
#ifdef LINUX
   asm ("popa"); 
#endif

#endif /* X86 */
#endif /* WIN32 */
}

#define FLUSH_WITH_SEQCHECK 0
#define SISCI_FLUSH 1

#ifndef NO_SISCI
void _smi_init_flush(int volatile *BaseAddr, sci_map_t mpisgmt_map) 
{
    sci_error_t SCIerror;
#if FLUSH_WITH_SEQCHECK
    sci_sequence_status_t seq_state;
#endif

    DSECTION ("_smi_init_flush");

    mpisgmt_addr = BaseAddr;

    if (mpisgmt_map != 0) {
	rs_SCICreateMapSequence(mpisgmt_map, &_smi_mpisgmt_seq, 0, &SCIerror);
	ASSERT_A(SCIerror == SCI_ERR_OK, "could not create SCI map sequence", -1);
#if FLUSH_WITH_SEQCHECK
	do {
	    seq_state = SCIStartSequence(_smi_mpisgmt_seq, 0, &SCIerror);
	} while ((seq_state != SCI_SEQ_OK) && (seq_state != SCI_SEQ_NOT_RETRIABLE));
	if (seq_state == SCI_SEQ_NOT_RETRIABLE) {
	    /* something has gone wrong with this session! */
	    DERROR("SCI session terminated - remote process crashed ?");
	    SMI_Abort(-1);
	}
#endif /* FLUSH_WITH_SEQCHECK */
    }
}



int _smi_flush_write_buffers() 
{
    
    int MemOffset = BASESGMT_OFFSET_FLUSH;
    int dummy = 0x12345678;
#if SISCI_FLUSH
    sci_error_t sci_err;
#else
	int i;
#endif
    if  (mpisgmt_addr == NULL)
      return(-1);

#if SISCI_FLUSH
    if (_smi_mpisgmt_seq != 0) {
		SMI_SCIStoreBarrier(_smi_mpisgmt_seq, 0, &sci_err);
    }
#else
#if FLUSH_WITH_SEQCHECK
    do {
	for (i = 0; i < FLUSH_SIZE/sizeof(int); i++) {
	    mpisgmt_addr[i+MemOffset] = dummy; 
	}
	_smi_intel_store_barrier();
    } while (_smi_check_transfer(_smi_mpisgmt_seq, 0) != SMI_SUCCESS);
#else
    /* XXX debug */
    for (i = 0; i < FLUSH_SIZE/sizeof(int); i++) {
	mpisgmt_addr[i+MemOffset] = dummy; 
    }
    _smi_intel_store_barrier();
#endif /* FLUSH_WITH_SEQCHECK */
#endif /* SISCI_FLUSH */
    
    return(mpisgmt_addr[MemOffset]);
}
		  
int _smi_flush_read_buffers(void) 
{
    int MemOffset = BASESGMT_OFFSET_FLUSH;
    int dummy = 0;
#if SISCI_FLUSH
    sci_error_t sci_err;
#else
	int i;
#endif
	
    if  (mpisgmt_addr == NULL)
      return(-1);

#if SISCI_FLUSH
    if (_smi_mpisgmt_seq != 0) {
	SCIFlushReadBuffers(_smi_mpisgmt_seq);
    }
#else
    for (i = 0; i < FLUSH_SIZE/sizeof(int); i++) {
	dummy += mpisgmt_addr[i + MemOffset];
    }
#endif    
    return(dummy);

}


#else /* NO_SISCI */


void _smi_init_flush(int volatile *BaseAddr, int dummy) 
{
  mpisgmt_addr = BaseAddr;
}

int _smi_flush_write_buffers() 
{
  _smi_intel_store_barrier();
  return(0);
}
		  
int _smi_flush_read_buffers(void) 
{
  return(0);
}
#endif /* NO_SISCI */

#ifdef WIN32
#pragma optimize( "",on)
#endif 
