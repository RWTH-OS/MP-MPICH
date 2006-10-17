/*
 *  $Id$
 *
 */

/* Code to initialize the optimized reduce operations on IA-32
   machines. We try to use the most sophisticated features like mmx, sse and sse2.

   Optimized reduce operations for other CPU platforms could be plugged in here, too.
*/

#include <memory.h>

#include "mpiimpl.h"
#include "x86_ops.h"

#if CPU_ARCH_IS_X86
X86_CPU_feature_t MPIR_X86Features;

#ifdef WIN32
#ifdef _M_AMD64

extern void __cpuid(int* CPUInfo, int InfoType);
#pragma intrinsic(__cpuid)

void MPIR_test_cpu(X86_CPU_feature_t *f) 
{
	int				CPUInfo[4] = {-1}; /* eax, ebx, ecx, edx */

	memset(f, 0, sizeof(X86_CPU_feature_t));

	__cpuid(CPUInfo, 1);
	
	
	/* Remember, 64-bit assembly code for Windows cannot use the older MMX, 3D Now! and 
	   x87 instruction extensions; they have been superseded by SSE/SSE2. */
	/*if (CPUInfo[3] & 0x80000)
		f->mmx = 1;*/
	
	if (CPUInfo[3] & 0x4000)
		f->cmov = 1;
	if (CPUInfo[3] & 0x2000000)
		f->sse = 1;
	if (CPUInfo[3] & 0x4000000)
		f->sse2 = 1;

	/* Remember, 64-bit assembly code for Windows cannot use the older MMX, 3D Now! and 
	   x87 instruction extensions; they have been superseded by SSE/SSE2. */
    /*__cpuid(CPUInfo, 0x80000000);
	if (CPUInfo[0] > 0x80000000)
	{
		__cpuid(CPUInfo, 0x80000001);
		if (CPUInfo[3] & 0x80000000)
			f->_3dnow = 1;
	}*/
}

#else

void MPIR_test_cpu(X86_CPU_feature_t *f) 
{

    __asm {
	    mov		ecx,f
	    mov		DWORD PTR [ecx],0
	    mov		DWORD PTR [ecx+4],0
	    mov		DWORD PTR [ecx+8],0
	    mov		DWORD PTR [ecx+12],0
	    mov		DWORD PTR [ecx+16],0

	    ;; check whether CPUID is supported
	    ;; (bit 21 of Eflags can be toggled)
	    pushfd			;save Eflags
	    pop		eax		;transfer Eflags into EAX
	    mov		edx, eax	;save original Eflags
	    xor		eax, 00200000h	;toggle bit 21
	    push	eax		;put new value of stack
	    popfd			;transfer new value to Eflags
	    pushfd			;save updated Eflags
	    pop		eax		;transfer Eflags to EAX
	    xor		eax, edx	;updated Eflags and original differ?
	    jz		NO_3DNow	;no diff, bit 21 can’t be toggled
	    
	    
	    mov		eax,1
	    cpuid			
	    mov		ecx,f
	  

	    test	edx, 4000h	; Is cmov bit (Bit 15 of EDX)
					; in feature flags set?
	    jz		nocmov
	    mov		DWORD PTR [ecx],1
nocmov:
	    test	edx, 00800000h	; Is IA MMX technology bit (Bit 23 of EDX)
					; in feature flags set?
	    jz		short NoMMX     
	    mov		DWORD PTR [ecx+4],1
NoMMX:	    test	edx, 02000000h  ; Is IA SSE technology bit (Bit 25 of EDX)
					; in feature flags set?
	    jz		NoSSE;
	    mov		DWORD PTR [ecx+8],1
	    test	edx, 04000000h  ; Is IA SSE2 technology bit (Bit 26 of EDX)
					; in feature flags set?
	    jz		NoSSE		;
	    mov		DWORD PTR [ecx+12],1
NoSSE:	
	    ;;test whether extended function 80000001h is supported
	    mov		eax, 80000000h	;call extended function 80000000h
	    cpuid			;reports back highest supported ext.function
	    cmp		eax, 80000000h	;supports functions > 80000000h?
	    jbe		NO_3DNow	;no 3DNow! support, either
	    
	    ;;test if function 80000001h indicates 3DNow! support
	    mov		eax, 80000001h	;call extended function 80000001h
	    cpuid			;reports back extended feature flags
	    test	edx, 80000000h	;bit 31 in extended features
	    jz		NO_3DNow	;if set, 3DNow! is supported
	    mov		ecx,f
	    mov		DWORD PTR [ecx+16],1
NO_3DNow: 
    }
}
#endif 
#endif /* WIN32 */

#endif

void MPIR_Setup_x86_reduce_ops(void) {
#if CPU_ARCH_IS_X86
    MPIR_test_cpu(&MPIR_X86Features);
#ifdef FEATURE_DEBUG
    DEBUG(fprintf(stderr,"Detected features: CMOVcc:%d MMX:%d SSE:%d SSE2:%d 3DNOW:%d\n",
		  MPIR_X86Features.cmov,MPIR_X86Features.mmx,MPIR_X86Features.sse,
		  MPIR_X86Features.sse2,MPIR_X86Features._3dnow);
	  fflush(stderr);)
#endif
#if 0
#if defined(WIN32) || defined(MPI_LINUX) || defined(MPI_solaris86)
#ifndef NOSSE
    if (MPIR_X86Features.sse || MPIR_X86Features.sse2) {
	if (MPIR_X86Features.cmov) {  
	    MPIR_Op_setup( MPIR_max_sse_cmov,   1, 1, MPI_MAX ); 
	    MPIR_Op_setup( MPIR_min_sse_cmov,   1, 1, MPI_MIN ); 
	} else {                                                 
	    MPIR_Op_setup( MPIR_MAXF,   1, 1, MPI_MAX );
	    MPIR_Op_setup( MPIR_MINF,   1, 1, MPI_MIN );
	}                                                    
	MPIR_Op_setup( MPIR_add_sse,    1, 1, MPI_SUM );
	MPIR_Op_setup( MPIR_prod_sse,   1, 1, MPI_PROD );
	MPIR_Op_setup( MPIR_land_sse,   1, 1, MPI_LAND );
	MPIR_Op_setup( MPIR_band_sse,   1, 1, MPI_BAND );
	MPIR_Op_setup( MPIR_lor_sse,    1, 1, MPI_LOR );
	MPIR_Op_setup( MPIR_bor_sse,    1, 1, MPI_BOR );
	MPIR_Op_setup( MPIR_lxor_sse,   1, 1, MPI_LXOR );
	MPIR_Op_setup( MPIR_bxor_sse,   1, 1, MPI_BXOR );
    }       
    else
#endif /* NOSSE */
/* Remember, 64-bit assembly code for Windows cannot use the older MMX, 3D Now! and 
   x87 instruction extensions; they have been superseded by SSE/SSE2. */
#if !(defined(WIN32) && defined(_M_AMD64))
	if (MPIR_X86Features.mmx) {
	if (MPIR_X86Features.cmov) {
	    MPIR_Op_setup( MPIR_max_mmx_cmov,   1, 1, MPI_MAX );
	    MPIR_Op_setup( MPIR_min_mmx_cmov,   1, 1, MPI_MIN );
	} else {
	    MPIR_Op_setup( MPIR_MAXF,   1, 1, MPI_MAX );
	    MPIR_Op_setup( MPIR_MINF,   1, 1, MPI_MIN );
	}
	MPIR_Op_setup( MPIR_add_mmx,    1, 1, MPI_SUM );
	MPIR_Op_setup( MPIR_prod_mmx,   1, 1, MPI_PROD );
	MPIR_Op_setup( MPIR_land_mmx,   1, 1, MPI_LAND );
	MPIR_Op_setup( MPIR_band_mmx,   1, 1, MPI_BAND );
	MPIR_Op_setup( MPIR_lor_mmx,    1, 1, MPI_LOR );
	MPIR_Op_setup( MPIR_bor_mmx,    1, 1, MPI_BOR );
	MPIR_Op_setup( MPIR_lxor_mmx,   1, 1, MPI_LXOR );
	MPIR_Op_setup( MPIR_bxor_mmx,   1, 1, MPI_BXOR );
    } else
#endif /* !(defined(WIN32) && !defined(_M_AMD64)) */
#endif
#endif /* if 0 */
#endif 
    {
	/* these are the standard Ops in C */
	MPIR_Op_setup( MPIR_MAXF,   1, 1, MPI_MAX );
	MPIR_Op_setup( MPIR_MINF,   1, 1, MPI_MIN );
	MPIR_Op_setup( MPIR_SUM,    1, 1, MPI_SUM );
	MPIR_Op_setup( MPIR_PROD,   1, 1, MPI_PROD );
	MPIR_Op_setup( MPIR_LAND,   1, 1, MPI_LAND );
	MPIR_Op_setup( MPIR_BAND,   1, 1, MPI_BAND );
	MPIR_Op_setup( MPIR_LOR,    1, 1, MPI_LOR );
	MPIR_Op_setup( MPIR_BOR,    1, 1, MPI_BOR );
	MPIR_Op_setup( MPIR_LXOR,   1, 1, MPI_LXOR );
	MPIR_Op_setup( MPIR_BXOR,   1, 1, MPI_BXOR );
    }
    MPIR_Op_setup( MPIR_MAXLOC, 1, 1, MPI_MAXLOC );
    MPIR_Op_setup( MPIR_MINLOC, 1, 1, MPI_MINLOC );
}

