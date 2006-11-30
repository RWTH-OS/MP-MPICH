/* $Id$ */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#define USE_MOVE64 0

/* this is a special memcpy version for x86 architectures
   which optimizes the use of the stream buffers on the
   SCI board 
   
   Thanks to Eric J. Korpela <korpela@ssl.berkeley.edu> for this
   improved version of our original assembler code.

   currently, the assembler code only compiles with gcc */

#ifdef WIN32
/* these macros for win32 are obsolete now, but I'll keep them here
   just for reference */
#define MOVE8_DEFINED
#define MOVE8(s, d) {\
         __asm { mov eax, dword ptr [s] } \
	 __asm { fld qword ptr [eax] } \
	 __asm { mov eax, dword ptr [d] } \
	 __asm { fstp qword ptr [eax] } }	

#define MOVE64(s,d) {\
        __asm { mov eax, dword ptr [s] } \
        __asm { fld qword ptr [eax]+56 } \
        __asm { fld qword ptr [eax]+48 } \
        __asm { fld qword ptr [eax]+40 } \
        __asm { fld qword ptr [eax]+32 } \
        __asm { fld qword ptr [eax]+24 } \
        __asm { fld qword ptr [eax]+16 } \
        __asm { fld qword ptr [eax]+8 } \
        __asm { fld qword ptr [eax] } \
        __asm { mov eax, dword ptr [d] } \
        __asm { fstp qword ptr [eax] } \
        __asm { fstp qword ptr [eax]+8 } \
        __asm { fstp qword ptr [eax]+16 } \
        __asm { fstp qword ptr [eax]+24 } \
        __asm { fstp qword ptr [eax]+32 } \
        __asm { fstp qword ptr [eax]+40 } \
        __asm { fstp qword ptr [eax]+48 } \
        __asm { fstp qword ptr [eax]+56 } }
#else
/* Unix on x86 - Sparc uses SMI_Memcpy */
#ifndef MPI_solaris
#ifdef __SUNPRO_C
#if 0
/* Solaris x86 with Sun cc */
#include "mov8.h"
#define MOVE8(s, d) mov8 ((char *)(s), (char *)(d))
#endif /* 0 */
#else
/* gcc */
#define MOVE8_DEFINED
#define MOVE8(s, d) asm volatile ("fildll (%0)\n\t" "fistpll (%1)" : \
                                  /*no output */ : \
                                  "r" ((char*) s), "r" ((char*) d) );

#define MOVE64(s, d) asm volatile ("fildll 56(%0)\n\t" "fildll 48(%0)\n\t" \
                                   "fildll 40(%0)\n\t" "fildll 32(%0)\n\t"  \
                                   "fildll 24(%0)\n\t" "fildll 16(%0)\n\t"  \
                                   "fildll 8(%0)\n\t" "fildll (%0)\n\t"  \
                                   "fistpll (%1)\n\t" "fistpll 8(%1)\n\t" \
                                   "fistpll 16(%1)\n\t" "fistpll 24(%1)\n\t"  \
                                   "fistpll 32(%1)\n\t" "fistpll 40(%1)\n\t"  \
                                   "fistpll 48(%1)\n\t" "fistpll 56(%1)" : \
                                   /*no output */ : \
                                   "r" ((char*) s), "r" ((char*) d) );
#endif

#endif 

void sci_memcpy(char *dst, char *src, int n) {
#ifdef WIN32
    /* is this an x86 architecture (the > 386 test is just for fun...) */
#if defined(_M_IX86) && (_M_IX86 > 300)
    __asm {
	mov edi, dword ptr [dst];
	mov esi, dword ptr [src];
	mov eax,[n];
	cmp eax,64    ; For small buffers a
	jl  dwall     ; DWORD move is faster
					
	mov ecx,eax;
        shr ecx,3;
        jz dwords;

stream: fld qword ptr [esi]
        fstp qword ptr [edi]
        add esi,8;
        add edi,8;
        loop stream;

dwords: and eax,7;
        jz end; 
dwall:  mov ecx,eax;
        shr ecx,2;
        jz bytes
        rep movsd;

bytes:  and eax,3;
        jz end;
        mov ecx,eax;
        rep movsb;
end:
    }
#else
    memcpy (dst, src, n);
#endif
#else
    unsigned int blocks, rest, i;
    char *d = dst;
    char *s = src;
    
    rest   = n; 
    blocks = n / 64;
    
#if USE_MOVE64
    /* move the 64 byte blocks */
    if (blocks) {
	for (i = 0; i < blocks; i++) {
	    MOVE64 (s, d);
	    s += 64;
	    d += 64;
	}
	rest = rest % 64; 
    }
#endif

    /* move the 8 byte blocks */
    blocks = rest / 8;
    if (blocks) {
	for (i = 0; i < blocks; i++) {
#ifdef MOVE8_DEFINED
	    MOVE8 (s, d);
#else
	    *(double *)d = *(double *)s;
#endif
	    s += 8;
	    d += 8;
	}
	rest = rest % 8;
    } 

    /* move the remaining bytes */
    if (rest) 
	memcpy(d, s, rest);
#endif
}
#endif /* MPI_solaris */

/* this is necessary if not linking with gcc 
void __eprintf (void) {
}
*/
