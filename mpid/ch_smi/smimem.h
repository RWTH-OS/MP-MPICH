/* $Id$ */

#ifndef _MPID_SMI_MEM_H
#define _MPID_SMI_MEM_H

/* Some macros for memory allocation and for optimized memcpy
   functions (for remote memory access via SCI) */
   
#include <stdlib.h>

#include "mpimem.h"
#include "smidef.h"
#include "smiperf.h"

/* checked memory allocation */
#define ALLOCATE(ptr,type,size) \
    if ((ptr = (type) MALLOC(size)) == NULL) { \
        fprintf(stderr,"[%d] ch_smi ERROR (%s:%d) : out of local memory (requested %d bytes)\n",\
                MPID_SMI_myid,__FILE__,__LINE__, size); \
        fflush(stderr); \
    	MPID_ABORT("Can't continue with insufficient local memory."); \
    } 
#define ZALLOCATE(ptr,type,size) \
    if ((ptr = (type) MALLOC(size)) == NULL) { \
        fprintf(stderr,"[%d] ch_smi ERROR (%s:%d) : out of local memory (requested %d bytes)\n",\
                MPID_SMI_myid,__FILE__,__LINE__, size); \
        fflush(stderr); \
    	MPID_ABORT("Can't continue with insufficient local memory."); \
    } \
    memset ((void *)ptr, 0, size);

/* ALIGNEDALLOCATE does the same as ALLOCATE, except that the allocated memory is aligned
   on a [alignment]-byte boundary. This is needed for some SSE assembler instructions used in SSE
   optimized functions (16 bytes for SSE).
   
   ALIGNEDALLOCATE should work on all platforms.
   
   ALIGNEDFREE _must_ be called to free memory allocated by ALIGNEDALLOCATE. */

#define ALIGNEDALLOCATE(ptr,type,amount,alignment) \
{	\
	void* mem_real; \
	void* mem_aligned = 0; \
	size_t mem_size = amount + alignment-1 + sizeof(void*); \
	\
	if (alignment==0)  { \
        fprintf(stderr,"[%d] ch_smi ERROR (%s:%d) : alignment to 0-byte boundary is impossible\n",\
                MPID_SMI_myid,__FILE__,__LINE__ ); \
        fflush(stderr); \
    	MPID_ABORT("Can't continue because of illegal use of ALIGNEDALLOCATE."); \
    } \
		\
	mem_real = malloc( mem_size ); \
	if (mem_real!=0) \
	{\
		mem_aligned = (void*) ( (size_t)mem_real + \
			sizeof(void*) + ( alignment - ( (size_t)mem_real+sizeof(void*) ) % alignment ) % alignment ); \
		*( (size_t*) ((size_t)mem_aligned - sizeof(void*))) = (size_t) mem_real; \
	} else { \
        fprintf(stderr,"[%d] ch_smi ERROR (%s:%d) : out of local memory (requested %d bytes)\n",\
                MPID_SMI_myid,__FILE__,__LINE__, mem_size ); \
        fflush(stderr); \
    	MPID_ABORT("Can't continue with insufficient local memory."); \
    } \
	\
	ptr = (type) mem_aligned; \
}

#define ALIGNEDFREE(mem) free((void*) *((size_t*) ((size_t)mem - sizeof(void*))) );

/* flushing write-combined data to memory / PCI-bus is different on 
   the various x86-CPUs */
#if defined AMD_ATHLON
#define MPID_SMI_WCSIZE 32
#define WC_FLUSH asm volatile ("sfence"::)
#elif defined INTEL_PENTIUM_III
#define MPID_SMI_WCSIZE 0
#define WC_FLUSH 
#elif defined INTEL_PENTIUM_4
#define MPID_SMI_WCSIZE 0
#define WC_FLUSH 
#else 
#define MPID_SMI_WCSIZE 0
#define WC_FLUSH 
#endif

/* define these memcpy-macros to gain optimal peformance */
#if defined (MPI_solaris86) || defined (MPI_solaris) || defined (MPI_LINUX) ||  (defined MPI_LINUX_ALPHA) || defined (WIN32) || defined(MPI_LINUX_IA64) || defined(MPI_LINUX_X86_64)
/* local to local memcpy (standard) */

#ifdef MEMCPY
#undef MEMCPY
#endif
#define MEMCPY(d,s,c) MPID_SMI_DEBUG_PRINT_COPY_MSG("MEMCPY",d,s,c); \
                      memcpy( d, s, c );
/* MEMCPY_S is for "small" amounts of data (in the short protocol) */
#ifdef MPI_LINUX_ALPHA
/* memcpy() on Alpha is bad for sizes < 16. Alignent to 8 byte is done in
   higher code layers; here we tweak only for 8-byte memcpy() calls. */
#define MEMCPY_S(d,s,c) if ((c) == 8) { \
             MPID_SMI_DEBUG_PRINT_COPY_MSG("LONG-assign",d,s,c); \
            ((long *)(d))[0] = ((long *)(s))[0]; \
         } else {\
             MPID_SMI_DEBUG_PRINT_COPY_MSG("MEMCPY_S",d,s,c); \
             memcpy( d, s, c ); }
#else 
#define MEMCPY_S(d,s,c) MPID_SMI_DEBUG_PRINT_COPY_MSG("MEMCPY_S",d,s,c); \
                      if (c < 256) memcpy( d, s, c ); \
                      else MPID_SMI_memcpy (d,s,c);
#endif

typedef void (* mpid_smi_memcpy_fcn_t)(void *, const void*, size_t);

/* low-latency-memcpy for small entities (smipriv.c) */
void mpid_smi_peelcpy_l (char *dest, char *src, ulong len);

mpid_smi_memcpy_fcn_t MPID_SMI_memcpy;
void _mpid_smi_smi_memcpy(void *d, const void* s, size_t l);
void _mpid_smi_mmx_memcpy(void *d, const void* s, size_t l);
void _mpid_smi_mmx32_memcpy(void *d, const void* s, size_t l);
void _mpid_smi_mmx64_memcpy(void *d, const void* s, size_t l);
void _mpid_smi_sse32_memcpy(void *d, const void* s, size_t l);
void _mpid_smi_sse64_memcpy(void *d, const void* s, size_t l);
void _mpid_smi_wc32_memcpy(void *d, const void* s, size_t l);
void _mpid_smi_wc64_memcpy(void *d, const void* s, size_t l);
void _mpid_smi_alpha_memcpy(void *d, const void* s, size_t l);
void _mpid_smi_mmx_prefetchnta_memcpy(void *d, const void* s, size_t l);

/* the optimal copy function is determined dynamically by querying the system */
#define REMOTE_COPY(d,s,l) MPID_SMI_memcpy(d,s,l)

/* If we don't want to use the SMI copy functions (to reduce the overhead),
   we have to decide between write-combine-copy and MMX-copy for writing to remote 
   SCI memory. Until the related query in the SCI driver works, we use MMX for Solaris
   and WC for Linux/Win32. */

/* memory consistency for remote SCI memory */
#define SCI_SYNC_WRITE(d)  if (MPID_SMI_is_remote[d]) { MPID_STAT_CALL(sci_flush);\
                            SMI_Flush_write(d); MPID_STAT_RETURN(sci_flush); }
#define SCI_SYNC_READ(d) if (MPID_SMI_is_remote[d]) { MPID_STAT_CALL(sci_flush); \
                            SMI_Flush_read(d); MPID_STAT_RETURN(sci_flush); }


#define WRITE_RMT_PTR(addr, value, rank) *(addr) = value; while (MPID_SMI_is_remote[rank] && MPID_SMI_cfg.DO_VERIFY \
                         && SMI_Check_transfer_proc(rank, CHECK_MODE) != SMI_SUCCESS) {\
                                MPID_STAT_COUNT(sci_transm_error);\
                                *(addr) = value; }

#define READ_RMT_PTR(addr, value, rank) SMI_Flush_read(rank); value = *(addr); \
                        while (MPID_SMI_is_remote[rank] && MPID_SMI_cfg.DO_VERIFY \
                                && SMI_Check_transfer_proc(rank, CHECK_MODE) != SMI_SUCCESS) {\
                                MPID_STAT_COUNT(sci_transm_error);\
                                SMI_Flush_read(rank); value = *(addr); }

#define READ_LOCAL_SHARED_PTR(addr, value, limit) do { value = *(addr); } while (value > limit);


/* use SMI for memory copy operations ? defined in smidef.h */
#if MPID_SMI_USE_SMI_MEMCPY
/* write-copy: from my send buffer towards other procs recv buffer ("Write") */
#if MPID_SMI_BUFFERCPY_VERIFY
#define MEMCPY_W(d,s,l,p) MPID_SMI_DEBUG_PRINT_COPY_MSG("MEMCPY_W",d,s,l); \
                              if (!MPID_SMI_is_remote[p]) \
                                  memcpy (d,s,l);\
                              else \
                                  if (MPID_SMI_cfg.DO_VERIFY) \
                                     SMI_Memcpy(d, s, l, SMI_MEMCPY_LP_RS|SMI_MEMCPY_ALIGN|SMI_MEMCPY_NOBARRIER); \
                                  else \
                                     SMI_Memcpy(d, s, l, SMI_MEMCPY_LP_RS|SMI_MEMCPY_ALIGN|SMI_MEMCPY_NOBARRIER|SMI_MEMCPY_NOVERIFY); 
#else
#define MEMCPY_W(d,s,l,p) MPID_SMI_DEBUG_PRINT_COPY_MSG("MEMCPY_W",d,s,l); \
                              if (!MPID_SMI_is_remote[p]) \
                                  memcpy (d,s,l);\
                              else \
                                  SMI_Memcpy(d, s, l, SMI_MEMCPY_LP_RS|SMI_MEMCPY_ALIGN|SMI_MEMCPY_NOBARRIER|SMI_MEMCPY_NOVERIFY);
#endif
/* from incoming buffer to user buffer ("Read") */
#define MEMCPY_R(d,s,l) MPID_SMI_DEBUG_PRINT_COPY_MSG("MEMCPY_R",d,s,l); \
                          memcpy (d,s,l);
#else
/* do not use SMI routines - recommended for performance reasons! */
#if MPID_SMI_BUFFERCPY_VERIFY
#define MEMCPY_W(d,s,l,p) MPID_SMI_DEBUG_PRINT_COPY_MSG("MEMCPY_W",d,s,l); \
                          PERF_BANDWIDTH_START(MPID_SMI_cfg.PERF_BW_LIMIT, MPID_SMI_cfg.PERF_BW_REDUCE); \
                          if (!MPID_SMI_is_remote[p]) { \
                              memcpy (d,s,l);\
                          } else { \
                              REMOTE_COPY (d,s,l);\
                              while (MPID_SMI_cfg.DO_VERIFY && SMI_Check_transfer_proc(p, CHECK_MODE) != SMI_SUCCESS) {\
                                MPID_STAT_COUNT(sci_transm_error);\
                                REMOTE_COPY (d,s,l);\
                            } \
                          } \
                          PERF_BANDWIDTH_LIMIT(MPID_SMI_cfg.PERF_BW_LIMIT, MPID_SMI_cfg.PERF_BW_REDUCE, l);

#define MEMCPY_W_FAST(d,s,l,p) MPID_SMI_DEBUG_PRINT_COPY_MSG("MEMCPY_W_FAST",d,s,l); \
                          PERF_BANDWIDTH_START(MPID_SMI_cfg.PERF_BW_LIMIT, MPID_SMI_cfg.PERF_BW_REDUCE); \
                          if (!MPID_SMI_is_remote[p]) { \
                              memcpy (d,s,l);\
                          } else { \
                              REMOTE_COPY (d,s,l);\
                          } \
                          PERF_BANDWIDTH_LIMIT(MPID_SMI_cfg.PERF_BW_LIMIT, MPID_SMI_cfg.PERF_BW_REDUCE, l);
#else
#define MEMCPY_W(d,s,l,p) MPID_SMI_DEBUG_PRINT_COPY_MSG("MEMCPY_W",d,s,l); \
                          PERF_BANDWIDTH_START(MPID_SMI_cfg.PERF_BW_LIMIT, MPID_SMI_cfg.PERF_BW_REDUCE); \
                            if (!MPID_SMI_is_remote[p]) \
                                memcpy (d,s,l);\
                            else \
                                REMOTE_COPY (d,s,l); \
                          PERF_BANDWIDTH_LIMIT(MPID_SMI_cfg.PERF_BW_LIMIT, MPID_SMI_cfg.PERF_BW_REDUCE, l); 

#endif
#define MEMCPY_R(d,s,l) MPID_SMI_DEBUG_PRINT_COPY_MSG("MEMCPY_R",d,s,l); \
                          memcpy (d,s,l);
#endif /* USE_SMI_MEMCPY */

#else

#undef MEMCPY
#define MEMCPY memcpy
#define LOCSCI_MEMCPY memcpy
#define LOCSCI_MEMCPY memcpy

#endif /* MPI_... */


#endif /* _MPID_SMI_MEM_H_ */
