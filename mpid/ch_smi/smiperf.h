/* $Id$ */

#ifndef _MPID_SMI_PERF_H
#define _MPID_SMI_PERF_H

/* This file defines macros for "performance modeling". Performance modelling
   allows to specify various performance parameters like
   - latency (for control messages):
     - gap latency (delay between two control msgs, only affects the sender)
     - send latency (delay for sending, affects sender and receiver)
     - recv latency (delay between receiving two msgs, only affects the receiver)
   - bandwidth:
     - (relative) bandwidth reduction factor
     - (absolute) maximum bandwidth

   Unfortunately, it is only possible to *increase* the latency and *reduce* the
   the bandwidth (relative to the actual performance without these parameters),
   not the other way round... ;-) 

   XXX: Instead of polling to wait, we could also use blocking (although the
   granularity would be much coarser) and see which effects this has.

   XXX: The bandwidth modelling has the problem that the effective MPI bandwidth
   is much lower than the low-level-memcpy bandwidht which is influenced this way.

   XXX: The bandwidth limiting is not threadsafe due to global variables used.
*/

#include "smidef.h"
#include "smistat.h"

extern longlong_t _mpid_smi_perf_tickref;
extern size_t _mpid_smi_perf_size;

#define PERF_TICKSCALE 12

#ifdef MPID_DEBUG_ALL  
/* Poll for 'ltncy' ticks - the ticks-to-time-translation must be done somewhere else. */
/* XXX: We use a 64-bit-counter for tick-counting - what about overflows? */
#define MPID_SMI_POLL(delay) if ((delay) > 0) { \
                                longlong_t tickref, tickcnt; \
                                SMI_Get_ticks(&tickref); \
                                do { \
                                   SMI_Get_ticks(&tickcnt); \
                                } while (tickcnt - tickref < (delay)); \
                              }

#define PERF_GAP_LATENCY(ltncy)  MPID_SMI_POLL(ltncy)
#define PERF_SEND_LATENCY(ltncy) MPID_SMI_POLL(ltncy);
#define PERF_RECV_LATENCY(ltncy) MPID_SMI_POLL(ltncy);

#define PERF_BANDWIDTH_START(max, reduce) if ((max) > 0 || (reduce) > 0) { SMI_Get_ticks(&_mpid_smi_perf_tickref); }
#define PERF_BANDWIDTH_LIMIT(max, reduce, len) if ((max) > 0 || (reduce) > 0) { \
                /* get elapsed time and derive bandwidth (byte/ticks) from it */ \
                longlong_t cpy_ticks, bytes_per_Xtick, poll_ticks = 0; \
                SMI_Get_ticks(&cpy_ticks); \
                cpy_ticks -= _mpid_smi_perf_tickref; \
                bytes_per_Xtick = ((len) << PERF_TICKSCALE)/cpy_ticks; \
                if ((max) > 0 && bytes_per_Xtick > (max)) { \
		    /* wait to limit bandwidth to max */ \
                    poll_ticks = (((len) << PERF_TICKSCALE)/(max)) - cpy_ticks; \
            	    MPID_SMI_POLL(poll_ticks); \
                } \
            	if ((reduce) > 0 && poll_ticks <= 0) { \
            	   /* reduce bandwidth by polling for some time */ \
            	   MPID_SMI_POLL(cpy_ticks*(100/(double)reduce - 1)); \
                } \
	     } 
#if 0
fprintf(stderr, "[%d] cpy_ticks = %lld, bytes_per_Xtick = %lld, max = %d\n", MPID_SMI_myid, cpy_ticks, bytes_per_Xtick, max); \
fprintf(stderr, "[%d] polling for %lld ticks\n", MPID_SMI_myid, poll_ticks); \
            	    MPID_SMI_POLL(poll_ticks); \

#endif
#else
/* deactivate macros for release version */
#define PERF_GAP_LATENCY(ltncy)
#define PERF_SEND_LATENCY(ltncy)
#define PERF_RECV_LATENCY(ltncy)

#define PERF_BANDWIDTH_START(max, reduce)
#define PERF_BANDWIDTH_LIMIT(max, reduce, len) 
#endif


#endif
