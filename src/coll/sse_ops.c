/*
 *  $Id$
 *
 */

#ifndef WIN32
#error sse_ops.c is for Win32 only
#else

#if defined(_M_IX86)

#include <mmintrin.h>
#include <xmmintrin.h>
#include "mpiimpl.h"
#include "x86_ops.h"
#include "mpiops.h"

/* This is used to align buffers to
   a given boundary. The algorithms assume that:
   1) Both buffers (in and inout) are equally aligned
   2) They are at least aligned to the size of the data type.
*/

/* IMPORTANT: Assumption 1 is by no means always true!
   Therefore, alternative code paths have to be added which can handle differently aligned buffers and hence are slower.
   It is impossible to realign the buffers when they are differently aligned without copying them. */

/* XXX: Why 32 bytes? 16 would be sufficient for the SSE operations? */
#define ALIGNEMENT 32

#define LN_DOUBLE 3
#define LN_INT 2
#define LN_FLOAT 2
#define LN_SHORT 1


/* ---------------------MPI_SUM----------------------------------*/

static void add_int_sse(int *inbuf,int*inoutbuf,unsigned int len);
static void add_short_sse(short *inbuf,short *inoutbuf,unsigned int len);
static void add_byte_sse(char *inbuf,char *inoutbuf,unsigned int len);
static void add_float_sse(float *inbuf,float* inoutbuf,unsigned int len);
static void add_double_sse(double *inbuf,double* inoutbuf,unsigned int len);
static void add_d_complex_sse(d_complex *inbuf,d_complex *inoutbuf,unsigned int len);
extern void add_error_func(void* d1,void*d2,unsigned int l) ;

void add_double_sse(double *inbuf,double* inoutbuf,unsigned int len) {
    unsigned int i,u;

    if(!len) return;
    _mm_prefetch((char*)(inbuf+4),_MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+4),_MM_HINT_NTA);
    u=len&(~3);   
    for(i=0;i<u;i+=4) {
		_mm_prefetch((char*)(inbuf+i+8),_MM_HINT_NTA);
		_mm_prefetch((char*)(inoutbuf+i+8),_MM_HINT_NTA);
		inoutbuf[i]+=inbuf[i];
		inoutbuf[i+1]+=inbuf[i+1];
		inoutbuf[i+2]+=inbuf[i+2];
		inoutbuf[i+3]+=inbuf[i+3];
    }
    for(;i<len;++i)
		inoutbuf[i]+=inbuf[i];
}

static void add_s_complex_sse(s_complex *inbuf,s_complex *inoutbuf,unsigned int len);/* {
    unsigned int i,u;

    if(!len) return;
    _mm_prefetch((char*)(inbuf+4),_MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+4),_MM_HINT_NTA);
    u=len&(~3);   
    for(i=0;i<u;i+=4) {
	_mm_prefetch((char*)(inbuf+i+8),_MM_HINT_NTA);
	_mm_prefetch((char*)(inoutbuf+i+8),_MM_HINT_NTA);
	inoutbuf[i].re+=inbuf[i].re;
	inoutbuf[i].im+=inbuf[i].im;
	
	inoutbuf[i+1].re+=inbuf[i+1].re;
	inoutbuf[i+1].im+=inbuf[i+1].im;

	inoutbuf[i+2].re+=inbuf[i+2].re;
	inoutbuf[i+2].im+=inbuf[i+2].im;

	inoutbuf[i+3].re+=inbuf[i+3].re;
	inoutbuf[i+3].im+=inbuf[i+3].im;
    }
    for(;i<len;++i) {
	inoutbuf[i].re+=inbuf[i].re;
	inoutbuf[i].im+=inbuf[i].im;
    }
}
*/
void add_d_complex_sse(d_complex *inbuf,d_complex *inoutbuf,unsigned int len) {
    unsigned int i,u;
    if(!len) return;
    _mm_prefetch((char*)(inbuf+2),_MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+2),_MM_HINT_NTA);
    u=len&(~3);   
    for(i=0;i<u;i+=2) {
	_mm_prefetch((char*)(inbuf+i+4),_MM_HINT_NTA);
	_mm_prefetch((char*)(inoutbuf+i+4),_MM_HINT_NTA);
	inoutbuf[i].re+=inbuf[i].re;
	inoutbuf[i].im+=inbuf[i].im;
	
	inoutbuf[i+1].re+=inbuf[i+1].re;
	inoutbuf[i+1].im+=inbuf[i+1].im;
    }
    for(;i<len;++i) {
	inoutbuf[i].re+=inbuf[i].re;
	inoutbuf[i].im+=inbuf[i].im;
    }
}




/*
    MPIR_INT=0, MPIR_FLOAT, MPIR_DOUBLE, MPIR_COMPLEX, MPIR_LONG, MPIR_SHORT,
    MPIR_CHAR, MPIR_BYTE, MPIR_UCHAR, MPIR_USHORT, MPIR_ULONG, MPIR_UINT,
    MPIR_CONTIG, MPIR_VECTOR, MPIR_HVECTOR, 
    MPIR_INDEXED,
    MPIR_HINDEXED, MPIR_STRUCT, MPIR_DOUBLE_COMPLEX, MPIR_PACKED, 
	MPIR_UB, MPIR_LB, MPIR_LONGDOUBLE, MPIR_LONGLONGINT, 
    MPIR_LOGICAL, MPIR_FORT_INT 
*/

static const proto_func jmp_add_sse[] = {
    add_int_sse,    /*int*/
    add_float_sse,    /*float*/
    add_double_sse,    /*double*/
    add_s_complex_sse,  /*complex*/
    add_int_sse,    /*long*/
    add_short_sse,  /*short*/
    add_byte_sse,   /*char*/
    add_byte_sse,   /*byte*/
    add_byte_sse,   /*uchar*/
    add_short_sse,  /*ushort*/
    add_int_sse,    /*ulong*/
    add_int_sse,    /*uint*/
    add_error_func, /*contig*/
    add_error_func, /*vector*/
    add_error_func, /*hvector*/
    add_error_func, /*indexed*/
    add_error_func, /*hindexed*/
    add_error_func, /*struct*/
    add_d_complex_sse,  /*double_complex*/
    add_error_func, /*packed*/
    add_error_func, /*ub*/
    add_error_func, /*lb*/
    add_double_sse,	    /*longdouble*/
    add_error_func, /*longlongint*/
    add_error_func, /*logical*/
    add_int_sse	    /*fort_int*/
};

void MPIR_add_sse(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_add_sse[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}

__declspec(naked)
void add_int_sse(int *inbuf,int*inoutbuf,unsigned int len) {
__asm{

	
	push	ebx

	mov	ecx,DWORD PTR [esp+16]		; Load length
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	
	mov	ebx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_INT			; to get aligned addresses
	
	mov	eax,ebx				; restore address of inoutbuf
	prefetchnta [eax+32]
	
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,1				; divide by 2
	jz	remain
	
align 4	
al_top:	
	movq	mm1, QWORD PTR [eax]		; Add integers
	movq	mm0, QWORD PTR [ecx+eax]	; 2 at a time
	
	paddd	mm0, mm1			;
	add	eax,8
	dec	edx
	movq	QWORD PTR [eax-8], mm0		;
	jne	al_top
remain:
	test	bl,1
	jz	startit

	mov	edx,DWORD PTR [ecx+eax]		; Add remaining
	add	edx,DWORD PTR [eax]		; integer
	mov	DWORD PTR [eax],edx
	add	eax,4
	jmp	startit

aligned:	
	mov	eax, ebx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;
startit:	
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]			; Prefetch the following cacheline
	prefetchnta [ecx+eax+32]		; 
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*2 integers.
	add	eax, 32				; 
	prefetchnta [eax+32]			; 
	prefetchnta [ecx+eax+32]		; 
	
	
	
	movq	mm2, QWORD PTR [ecx+eax-24]	; 
	movq	mm3, QWORD PTR [eax-24]		;	  
	paddd	mm0, mm1			;
	movq	QWORD PTR [eax-32], mm0		;
	
	

	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	paddd	mm2, mm3			;
	movq	QWORD PTR [eax-24], mm2
	
	

	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	paddd	mm0, mm1			;
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	paddd	mm2, mm3			;
	movq	QWORD PTR [eax-8], mm2		;
	
	jne	SHORT ltop			;
unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,7				; calculate remaining 
	shr	edx,1				; number of integers
	jz	norest				; 
lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; add two ints
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	paddd	mm0, mm1			;
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					;
	test	bl,1				;
	jz	end				; No--> we are finished
	mov	edx, DWORD PTR [eax]		; Add the remaining integer
	add	edx, DWORD PTR [ecx+eax]		;
	mov	[eax],edx			;
end:
	pop	ebx
	ret	
    }
}

__declspec(naked)
void add_short_sse(short *inbuf,short* inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf	
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT			; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	remain
		
al_top:	
	movq	mm1, QWORD PTR [eax]		; Add shorts
	movq	mm0, QWORD PTR [ecx+eax]	; 4 at a time
	add	eax,8
	paddw	mm0, mm1			;
	movq	QWORD PTR [eax-8], mm0		;
	
	dec	edx
	jne	al_top
remain:
	and	bl,3
	jz	startit
al_rest:
	mov	dx,WORD PTR [ecx+eax]		; Add remaining
	add	dx,WORD PTR [eax]		; shorts
	mov	WORD PTR [eax],dx
	add	eax,2
	dec	bl
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:

	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 4				; len /= 16
	jz	SHORT unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]			; Prefetch the following cacheline
	prefetchnta [ecx+eax+32]		; 	
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*4 shorts.
	    
	add	eax, 32				; 

	prefetchnta [eax+32]			; 
	prefetchnta [ecx+eax+32]		; 

						; 
	movq	mm2, QWORD PTR [ecx+eax-24]	; 
	movq	mm3, QWORD PTR [eax-24]		;
	  
	paddw	mm0, mm1			;
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	paddw	mm2, mm3			;
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	paddw	mm0, mm1			;
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	paddw	mm2, mm3			;
	movq	QWORD PTR [eax-8], mm2		;
	
	jne	SHORT ltop			;

unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,15				; calculate remaining 
	shr	edx,2				; number of shorts
	jz	norest				; 

lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; add 4 shorts
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	paddw	mm0, mm1			;
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					; restore FP state
	
	and	bl,3				; calculate remaining shorts
	jz	end				; 0 --> we are finished
lshort:
	mov	dx, WORD PTR [ecx+eax]		; Add the remaining integer
	add	dx, WORD PTR [eax]		;
	mov	WORD PTR [eax],dx		;
	add	eax,2
	dec	bl
	jnz	lshort

end:
	pop	ebx
	ret	
    }
}

__declspec(naked)
void add_byte_sse(char *inbuf,char *inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of bytes 
	sub	edx,eax				; that have to be handled	
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,3				; divide by 8
	jz	remain
		
al_top:	
	movq	mm1, QWORD PTR [eax]		; Add bytes
	movq	mm0, QWORD PTR [ecx+eax]	; 8 at a time
	add	eax,8
	paddb	mm0, mm1			;
	movq	QWORD PTR [eax-8], mm0		;
	
	dec	edx
	jne	al_top
remain:
	and	bl,7
	jz	startit

al_rest:
	mov	dl,BYTE PTR [ecx+eax]		; Add remaining
	add	dl,BYTE PTR [eax]		; bytes
	mov	byte PTR [eax],dl
	inc	eax
	dec	bl
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 5				; len /= 32
	jz	SHORT unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]			; 
	prefetchnta [ecx+eax+32]		; 
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*8 bytes.
	    
	add	eax, 32				; 
	prefetchnta [eax+32]			; 
	prefetchnta [ecx+eax+32]		; 

						; 
	movq	mm2, QWORD PTR [ecx+eax-24]	; 
	movq	mm3, QWORD PTR [eax-24]		;
	  
	paddb	mm0, mm1			;
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	paddb	mm2, mm3			;
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	paddb	mm0, mm1			;
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	paddb	mm2, mm3			;
	movq	QWORD PTR [eax-8], mm2		;
	
	jne	SHORT ltop			;

unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,31				; calculate remaining 
	shr	edx,3				; number of bytes
	jz	norest				; 

lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; add 8 bytes
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	paddb	mm0, mm1			;
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					; restore FP state
	
	and	bl,7				; calculate remaining bytes
	jz	end				; 0 --> we are finished
lbyte:
	mov	dl, BYTE PTR [ecx+eax]		; Add the remaining bytes
	add	dl, BYTE PTR [eax]		;
	mov	BYTE PTR [eax],dl		;
	inc	eax
	dec	bl
	jnz	lbyte

end:
	pop	ebx
	ret	
    }
}

__declspec(naked)
void add_float_sse(float *inbuf,float *inoutbuf,unsigned int len) {
__asm{	
	push	ebx
	
	mov	eax, DWORD PTR [esp+8]		; load inbuf
	mov	ecx, DWORD PTR [esp+12]		; load inoutbuf
		
	and eax, ALIGNEMENT-1		; eax = inbuf % ALIGNEMENT
	and ecx, ALIGNEMENT-1		; ecx = inoutbuf % ALIGNEMENT

	xor eax,ecx					; if inbuf and inoutbuf are equally (mis)aligned, eax will now be 0
	jne misaligned				; if not, use alternative code path (slower)
		
		
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax						; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_FLOAT			; to get aligned addresses
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]
	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	remain
	
al_top:	
	movups	xmm1, XMMWORD PTR [eax]		; Add floats
	movups	xmm0, XMMWORD PTR [ecx+eax]	; 4 at a time
	add	eax,16
	addps	xmm0, xmm1			;
	dec	edx
	movups	XMMWORD PTR [eax-16], xmm0	;
	jne	al_top
remain:
	and	bl,3
	jz	startit

al_rest:
	movss	xmm1,DWORD PTR [ecx+eax]	; Add remaining
	addss	xmm1,DWORD PTR [eax]		; floats
	add	eax,4
	dec	bl
	movss	DWORD PTR [eax-4],xmm1
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;
startit:
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]			; Prefetch the following cacheline
	prefetchnta [ecx+eax+32]		; 
	jmp	INTOLOOP
ALIGN 4
ltop:
	movaps	XMMWORD PTR [eax-16], xmm2		;

INTOLOOP:
	movaps	xmm1, XMMWORD PTR [ecx+eax]	; This is an unrolled loop
	movaps	xmm0, XMMWORD PTR [eax] 	; that adds 2*4 floats.
	add	eax, 32				
	prefetchnta [eax+32]			; 
	prefetchnta [ecx+eax+32]		; prefetch the cacheline after the next
	
	addps	xmm0, xmm1			;    
	movaps	xmm2, XMMWORD PTR [ecx+eax-16]	; 
	movaps	xmm3, XMMWORD PTR [eax-16]	;
	  	
	movaps	XMMWORD PTR [eax-32], xmm0	;
	dec	edx				;	
	addps	xmm2, xmm3			;
	jnz	SHORT ltop			;

	movaps	XMMWORD PTR [eax-16], xmm2	;
	
unroll_end:	
	test	bl, 4
	jz	norest				;
	movaps	xmm1, XMMWORD PTR [eax]		; Add floats
	movaps	xmm0, XMMWORD PTR [ecx+eax]	; 4 at a time
	add	eax,16
	addps	xmm0, xmm1			;
	movaps	XMMWORD PTR [eax-16], xmm0	;
norest:	
	

	and	bl,3				;
	jz	end				; No--> we are finished
align 4
lfloat:	
	movss	xmm1,DWORD PTR [ecx+eax]	; Add remaining
	addss	xmm1,DWORD PTR [eax]		; floats
	add	eax,4
	dec	bl
	movss	DWORD PTR [eax-4],xmm1
	jnz	short lfloat
end:
	pop	ebx
	ret

; ----- alternative code path (slower) for differently aligned buffers -----

misaligned:
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;
startit2:
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT unroll_end2		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]			; Prefetch the following cacheline
	prefetchnta [ecx+eax+32]		; 
	jmp	INTOLOOP2
ALIGN 4
ltop2:
	movups	XMMWORD PTR [eax-16], xmm2		;

INTOLOOP2:
	movups	xmm1, XMMWORD PTR [ecx+eax]	; This is an unrolled loop
	movups	xmm0, XMMWORD PTR [eax] 	; that adds 2*4 floats.
	add	eax, 32				
	prefetchnta [eax+32]			; 
	prefetchnta [ecx+eax+32]		; prefetch the cacheline after the next
	
	addps	xmm0, xmm1			;    
	movups	xmm2, XMMWORD PTR [ecx+eax-16]	; 
	movups	xmm3, XMMWORD PTR [eax-16]	;
	  	
	movups	XMMWORD PTR [eax-32], xmm0	;
	dec	edx				;	
	addps	xmm2, xmm3			;
	jnz	SHORT ltop2			;

	movups	XMMWORD PTR [eax-16], xmm2	;
	
unroll_end2:	
	test	bl, 4
	jz	norest2				;
	movups	xmm1, XMMWORD PTR [eax]		; Add floats
	movups	xmm0, XMMWORD PTR [ecx+eax]	; 4 at a time
	add	eax,16
	addps	xmm0, xmm1			;
	movups	XMMWORD PTR [eax-16], xmm0	;
norest2:	
	

	and	bl,3				;
	jz	end2				; No--> we are finished
align 4
lfloat2:	
	movss	xmm1,DWORD PTR [ecx+eax]	; Add remaining
	addss	xmm1,DWORD PTR [eax]		; floats
	add	eax,4
	dec	bl
	movss	DWORD PTR [eax-4],xmm1
	jnz	short lfloat2
end2:
	pop	ebx
	ret
    }
}

__declspec(naked)
void add_s_complex_sse(s_complex *inbuf,s_complex *inoutbuf,unsigned int len) {
__asm{	
	push	ebx
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of structs 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_DOUBLE			; to get aligned addresses
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]
	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,1				; divide by 2
	jz	remain
	
al_top:	
	movups	xmm1, XMMWORD PTR [eax]		; Add complex numbers
	movups	xmm0, XMMWORD PTR [ecx+eax]	; 2 at a time
	add	eax,16
	addps	xmm0, xmm1			;
	dec	edx
	movups	XMMWORD PTR [eax-16], xmm0	;
	jne	al_top
remain:
	test	bl,1
	jz	startit

al_rest:
	movss	xmm1,DWORD PTR [ecx+eax]	; Add remaining
	addss	xmm1,DWORD PTR [eax]		; complex number
	movss	DWORD PTR [eax],xmm1
	add	eax,4
	movss	xmm1,DWORD PTR [ecx+eax]	
	addss	xmm1,DWORD PTR [eax]		
	movss	DWORD PTR [eax],xmm1
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;
startit:
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 2				; len /= 4
	jz	SHORT unroll_end		; len==0 --> less than 4 numbers
	prefetchnta [eax+32]			; Prefetch the following cacheline
	prefetchnta [ecx+eax+32]		; 
	jmp	INTOLOOP
ALIGN 4
ltop:
	movaps	XMMWORD PTR [eax-16], xmm2		;

INTOLOOP:
	movaps	xmm1, XMMWORD PTR [ecx+eax]	; This is an unrolled loop
	movaps	xmm0, XMMWORD PTR [eax] 	; that adds 2*2 complex numbers.
	add	eax, 32				
	prefetchnta [eax+32]			; 
	prefetchnta [ecx+eax+32]		; prefetch the cacheline after the next
	
	addps	xmm0, xmm1			;    
	movaps	xmm2, XMMWORD PTR [ecx+eax-16]	; 
	movaps	xmm3, XMMWORD PTR [eax-16]	;
	  	
	movaps	XMMWORD PTR [eax-32], xmm0	;
	dec	edx				;	
	addps	xmm2, xmm3			;
	jnz	SHORT ltop			;

	movaps	XMMWORD PTR [eax-16], xmm2	;
	
unroll_end:	
	test	bl, 2
	jz	norest				;
	movaps	xmm1, XMMWORD PTR [eax]		; Add 2 complex
	movaps	xmm0, XMMWORD PTR [ecx+eax]	; numbers
	add	eax,16
	addps	xmm0, xmm1			;
	movaps	XMMWORD PTR [eax-16], xmm0	;

norest:	
	test	bl,1				;
	jz	end				; No--> we are finished
lfloat:	
	movss	xmm1,DWORD PTR [ecx+eax]	; Add remaining
	addss	xmm1,DWORD PTR [eax]		; number
	movss	DWORD PTR [eax],xmm1
	movss	xmm1,DWORD PTR [ecx+eax+4]	
	addss	xmm1,DWORD PTR [eax+4]		
	movss	DWORD PTR [eax+4],xmm1
end:
	pop	ebx
	ret	
    }
}

/* ---------------------MPI_PROD----------------------------------*/
extern void prod_error_func(void* d1,void*d2,unsigned int l) ;

static void prod_short_sse(short *inbuf,short *inoutbuf,unsigned int len);

static void prod_int_sse(int *inbuf,int*inoutbuf,unsigned int len) {
    unsigned int i,u;

    if(!len) return;
    _mm_prefetch((char*)(inbuf+8),_MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+8),_MM_HINT_NTA);
    u=len&(~7);   
    for(i=0;i<u;i+=8) {
	_mm_prefetch((char*)(inbuf+i+16),_MM_HINT_NTA);
	_mm_prefetch((char*)(inoutbuf+i+16),_MM_HINT_NTA);
	inoutbuf[i]*=inbuf[i];
	inoutbuf[i+1]*=inbuf[i+1];
	inoutbuf[i+2]*=inbuf[i+2];
	inoutbuf[i+3]*=inbuf[i+3];
	inoutbuf[i+4]*=inbuf[i+4];
	inoutbuf[i+5]*=inbuf[i+5];
	inoutbuf[i+6]*=inbuf[i+6];
	inoutbuf[i+7]*=inbuf[i+7];
    }
    for(;i<len;++i)
	inoutbuf[i]*=inbuf[i];
}

static void prod_byte_sse(char *inbuf,char *inoutbuf,unsigned int len) {
    unsigned int i,u;

    if(!len) return;
    _mm_prefetch((char*)(inbuf+32),_MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+32),_MM_HINT_NTA);
    u=len&(~31);   
    for(i=0;i<u;i+=32) {
	_mm_prefetch((char*)(inbuf+i+64),_MM_HINT_NTA);
	_mm_prefetch((char*)(inoutbuf+i+64),_MM_HINT_NTA);
	inoutbuf[i]*=inbuf[i];
	inoutbuf[i+1]*=inbuf[i+1];
	inoutbuf[i+2]*=inbuf[i+2];
	inoutbuf[i+3]*=inbuf[i+3];
	inoutbuf[i+4]*=inbuf[i+4];
	inoutbuf[i+5]*=inbuf[i+5];
	inoutbuf[i+6]*=inbuf[i+6];
	inoutbuf[i+7]*=inbuf[i+7];
	inoutbuf[i+8]*=inbuf[i+8];
	inoutbuf[i+9]*=inbuf[i+9];
	inoutbuf[i+10]*=inbuf[i+10];
	inoutbuf[i+11]*=inbuf[i+11];
	inoutbuf[i+12]*=inbuf[i+12];
	inoutbuf[i+13]*=inbuf[i+13];
	inoutbuf[i+14]*=inbuf[i+14];
	inoutbuf[i+15]*=inbuf[i+15];
	inoutbuf[i+16]*=inbuf[i+16];
	inoutbuf[i+17]*=inbuf[i+17];
	inoutbuf[i+18]*=inbuf[i+18];
	inoutbuf[i+19]*=inbuf[i+19];
	inoutbuf[i+20]*=inbuf[i+20];
	inoutbuf[i+21]*=inbuf[i+21];
	inoutbuf[i+22]*=inbuf[i+22];
	inoutbuf[i+23]*=inbuf[i+23];
	inoutbuf[i+24]*=inbuf[i+24];
	inoutbuf[i+25]*=inbuf[i+25];
	inoutbuf[i+26]*=inbuf[i+26];
	inoutbuf[i+27]*=inbuf[i+27];
	inoutbuf[i+28]*=inbuf[i+28];
	inoutbuf[i+29]*=inbuf[i+29];
	inoutbuf[i+30]*=inbuf[i+30];
	inoutbuf[i+31]*=inbuf[i+31];
    }
    for(;i<len;++i)
	inoutbuf[i]*=inbuf[i];
}


static void prod_float_sse(float *inbuf,float* inoutbuf,unsigned int len) ;
/*{
    unsigned int i,u;
    __m128 op1,op2;
    if(!len) return;
    _mm_prefetch(inbuf+8,_MM_HINT_NTA);
    _mm_prefetch(inoutbuf+8,_MM_HINT_NTA);
    u=len&(~7);   
    for(i=0;i<u;i+=8) {
	_mm_prefetch(inbuf+i+16,_MM_HINT_NTA);
	_mm_prefetch(inoutbuf+i+16,_MM_HINT_NTA);
	
	op1=_mm_loadu_ps(inbuf+i);
	op2=_mm_loadu_ps(inoutbuf+i);
	op1=_mm_mul_ps(op1,op2);
	_mm_storeu_ps(inoutbuf+i,op1);

	op1=_mm_loadu_ps(inbuf+i+4);
	op2=_mm_loadu_ps(inoutbuf+i+4);
	op1=_mm_mul_ps(op1,op2);
	_mm_storeu_ps(inoutbuf+i+4,op1);
    }
    for(;i<len;++i) {
	op1=_mm_load_ss(inbuf+i);
	op2=_mm_load_ss(inoutbuf+i);
	op1=_mm_mul_ss(op1,op2);
	_mm_store_ss(inoutbuf+i,op1);
    }
    
}
*/
static void prod_double_sse(double *inbuf,double* inoutbuf,unsigned int len) {
    unsigned int i,u;

    if(!len) return;
    _mm_prefetch((char*)(inbuf+4),_MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+4),_MM_HINT_NTA);
    u=len&(~3);   
    for(i=0;i<u;i+=4) {
	_mm_prefetch((char*)(inbuf+i+8),_MM_HINT_NTA);
	_mm_prefetch((char*)(inoutbuf+i+8),_MM_HINT_NTA);
	inoutbuf[i]*=inbuf[i];
	inoutbuf[i+1]*=inbuf[i+1];
	inoutbuf[i+2]*=inbuf[i+2];
	inoutbuf[i+3]*=inbuf[i+3];
    }
    for(;i<len;++i)
	inoutbuf[i]*=inbuf[i];
}

static void prod_s_complex_sse(s_complex *inbuf,s_complex *inoutbuf,unsigned int len) {
    unsigned int i,u;
    float c;

    if(!len) return;
    _mm_prefetch((char*)(inbuf+4),_MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+4),_MM_HINT_NTA);
    u=len&(~3);

    for(i=0;i<u;i+=4) {
	_mm_prefetch((char*)(inbuf+i+8),_MM_HINT_NTA);
	_mm_prefetch((char*)(inoutbuf+i+8),_MM_HINT_NTA);
	c = inoutbuf[i].re;
	inoutbuf[i].re = inoutbuf[i].re*inbuf[i].re - inoutbuf[i].im*inbuf[i].im;
	inoutbuf[i].im = c*inbuf[i].im+inoutbuf[i].im*inbuf[i].re ;
	
	c = inoutbuf[i+1].re;
	inoutbuf[i+1].re = inoutbuf[i+1].re*inbuf[i+1].re - inoutbuf[i+1].im*inbuf[i+1].im;
	inoutbuf[i+1].im = c*inbuf[i+1].im+inoutbuf[i+1].im*inbuf[i+1].re ;

	c = inoutbuf[i+2].re;
	inoutbuf[i+2].re = inoutbuf[i+2].re*inbuf[i+2].re - inoutbuf[i+2].im*inbuf[i+2].im;
	inoutbuf[i+2].im = c*inbuf[i+2].im+inoutbuf[i+2].im*inbuf[i+2].re ;

	c = inoutbuf[i+3].re;
	inoutbuf[i+3].re = inoutbuf[i+3].re*inbuf[i+3].re - inoutbuf[i+3].im*inbuf[i+3].im;
	inoutbuf[i+3].im = c*inbuf[i+3].im+inoutbuf[i+3].im*inbuf[i+3].re ;
    }
    for(;i<len;++i) {
	c = inoutbuf[i].re;
	inoutbuf[i].re = inoutbuf[i].re*inbuf[i].re - inoutbuf[i].im*inbuf[i].im;
	inoutbuf[i].im = c*inbuf[i].im+inoutbuf[i].im*inbuf[i].re ;
    }
}

static void prod_d_complex_sse(d_complex *inbuf,d_complex *inoutbuf,unsigned int len) {
    unsigned int i,u;
    double c;
    if(!len) return;
    _mm_prefetch((char*)(inbuf+2),_MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+2),_MM_HINT_NTA);
    u=len&(~2);
    for(i=0;i<u;i+=2) {
	_mm_prefetch((char*)(inbuf+i+4),_MM_HINT_NTA);
	_mm_prefetch((char*)(inoutbuf+i+4),_MM_HINT_NTA);
	c = inoutbuf[i].re;
	inoutbuf[i].re = inoutbuf[i].re*inbuf[i].re - inoutbuf[i].im*inbuf[i].im;
	inoutbuf[i].im = c*inbuf[i].im+inoutbuf[i].im*inbuf[i].re ;
	
	c = inoutbuf[i+1].re;
	inoutbuf[i+1].re = inoutbuf[i+1].re*inbuf[i+1].re - inoutbuf[i+1].im*inbuf[i+1].im;
	inoutbuf[i+1].im = c*inbuf[i+1].im+inoutbuf[i+1].im*inbuf[i+1].re ;
    }
    for(;i<len;++i) {
	c = inoutbuf[i].re;
	inoutbuf[i].re = inoutbuf[i].re*inbuf[i].re - inoutbuf[i].im*inbuf[i].im;
	inoutbuf[i].im = c*inbuf[i].im+inoutbuf[i].im*inbuf[i].re ;
    }
}

static  const proto_func jmp_prod_sse[] = {
    prod_int_sse,	    /*int*/
    prod_float_sse,	    /*float*/
    prod_double_sse,    /*double*/
    prod_s_complex_sse, /*complex*/
    prod_int_sse,	    /*long*/
    prod_short_sse, /*short*/
    prod_byte_sse,	    /*char*/
    prod_byte_sse,	    /*byte*/
    prod_byte_sse,	    /*uchar*/
    prod_short_sse, /*ushort*/
    prod_int_sse,	    /*ulong*/
    prod_int_sse,	    /*uint*/
    prod_error_func,/*contig*/
    prod_error_func,/*vector*/
    prod_error_func,/*hvector*/
    prod_error_func,/*indexed*/
    prod_error_func,/*hindexed*/
    prod_error_func,/*struct*/
    prod_d_complex_sse, /*double_complex*/
    prod_error_func,/*packed*/
    prod_error_func,/*ub*/
    prod_error_func,/*lb*/
    prod_double_sse,    /*longdouble*/
    prod_error_func,/*longlongint*/
    prod_error_func,/*logical*/
    prod_int_sse    /*fort_int*/
};

void MPIR_prod_sse(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_prod_sse[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}

__declspec(naked)
void prod_short_sse(short *inbuf,short* inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT			; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]
	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	remain
		
al_top:	
	movq	mm1, QWORD PTR [eax]		; Multiply shorts
	movq	mm0, QWORD PTR [ecx+eax]	; 4 at a time
	add	eax,8
	pmullw	mm0, mm1			;
	movq	QWORD PTR [eax-8], mm0		;
	
	dec	edx
	jne	al_top
remain:
	and	bl,3
	jz	startit
al_rest:
	mov	dx,WORD PTR [ecx+eax]		; Multiply remaining
	imul	dx,WORD PTR [eax]		; shorts
	mov	WORD PTR [eax],dx
	add	eax,2
	dec	bl
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 4				; len /= 16
	jz	SHORT unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*4 shorts.
	    
	add	eax, 32				; 
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
	
	movq	mm2, QWORD PTR [ecx+eax-24]	; 
	movq	mm3, QWORD PTR [eax-24]		;
	  
	pmullw	mm0, mm1			;
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	pmullw	mm2, mm3			;
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	pmullw	mm0, mm1			;
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	pmullw	mm2, mm3			;
	movq	QWORD PTR [eax-8], mm2		;
	
	jne	SHORT ltop			;

unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,15				; calculate remaining 
	shr	edx,2				; number of shorts
	jz	norest				; 

lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; add 4 shorts
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	pmullw	mm0, mm1			;
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					; restore FP state
	
	and	bl,3				; calculate remaining shorts
	jz	end				; 0 --> we are finished
lshort:
	mov	dx, WORD PTR [ecx+eax]		; Multiply the remaining shorts
	imul	dx, WORD PTR [eax]		;
	mov	WORD PTR [eax],dx		;
	add	eax,2
	dec	bl
	jnz	lshort

end:
	pop	ebx
	ret	
    }
}

__declspec(naked)
void prod_float_sse(float *inbuf,float *inoutbuf,unsigned int len) {
__asm{	
	push	ebx
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_FLOAT			; to get aligned addresses
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]
	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	remain
	
al_top:	
	movups	xmm1, XMMWORD PTR [eax]		; Add floats
	movups	xmm0, XMMWORD PTR [ecx+eax]	; 4 at a time
	add	eax,16
	mulps	xmm0, xmm1			;
	dec	edx
	movups	XMMWORD PTR [eax-16], xmm0	;
	jne	al_top
remain:
	and	bl,3
	jz	startit

al_rest:
	movss	xmm1,DWORD PTR [ecx+eax]	; Add remaining
	mulss	xmm1,DWORD PTR [eax]		; floats
	add	eax,4
	dec	bl
	movss	DWORD PTR [eax-4],xmm1
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;
startit:
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]			; Prefetch the following cacheline
	prefetchnta [ecx+eax+32]		; 
	jmp	INTOLOOP

ALIGN 4
ltop:
	movaps	XMMWORD PTR [eax-16], xmm2		;

INTOLOOP:
	movaps	xmm1, XMMWORD PTR [ecx+eax]	; This is an unrolled loop
	movaps	xmm0, XMMWORD PTR [eax] 	; that adds 2*4 floats.
	add	eax, 32				
	prefetchnta [eax+32]			; 
	prefetchnta [ecx+eax+32]		; prefetch the cacheline after the next
	
	mulps	xmm0, xmm1			;    
	movaps	xmm2, XMMWORD PTR [ecx+eax-16]	; 
	movaps	xmm3, XMMWORD PTR [eax-16]	;
	  	
	movaps	XMMWORD PTR [eax-32], xmm0	;
	dec	edx				;	
	mulps	xmm2, xmm3			;
	jnz	SHORT ltop			;

	movaps	XMMWORD PTR [eax-16], xmm2	;
	
unroll_end:	
	test	bl, 4
	jz	norest				;
	movaps	xmm1, XMMWORD PTR [eax]		; Add floats
	movaps	xmm0, XMMWORD PTR [ecx+eax]	; 4 at a time
	add	eax,16
	mulps	xmm0, xmm1			;
	movaps	XMMWORD PTR [eax-16], xmm0	;
norest:	
	

	and	bl,3				;
	jz	end				; No--> we are finished
align 4
lfloat:	
	movss	xmm1,DWORD PTR [ecx+eax]	; Add remaining
	mulss	xmm1,DWORD PTR [eax]		; floats
	add	eax,4
	dec	bl
	movss	DWORD PTR [eax-4],xmm1
	jnz	short lfloat
end:
	pop	ebx
	ret	
    }
}

/* ---------------------MPI_MAX----------------------------------*/
extern void max_error_func(void* d1,void*d2,unsigned int l);

static void max_sint_sse(int *inbuf,int*inoutbuf,unsigned int len);
static void max_sshort_sse(short *inbuf,short *inoutbuf,unsigned int len);
static void max_sbyte_sse(signed char *inbuf,signed char *inoutbuf,unsigned int len);

static void max_float_sse(float *inbuf,float *inoutbuf,unsigned int len);
static void max_double_cmov(double *inbuf,double *inoutbuf,unsigned int len);
static void max_uint_cmov(unsigned int *inbuf,unsigned int*inoutbuf,unsigned int len);
static void max_ushort_cmov(unsigned short *inbuf,unsigned short *inoutbuf,unsigned int len);
static void max_ubyte_sse(unsigned char *inbuf,unsigned char *inoutbuf,unsigned int len);


static  const proto_func jmp_max_sse_cmov[] = {
    max_sint_sse,   /*int*/
    max_float_sse, /*float*/
    max_double_cmov,/*double*/
    max_error_func, /*complex*/
    max_sint_sse,   /*long*/
    max_sshort_sse, /*short*/
    max_sbyte_sse,  /*char*/
    max_ubyte_sse, /*byte*/
    max_ubyte_sse, /*uchar*/
    max_ushort_cmov,/*ushort*/
    max_uint_cmov,  /*ulong*/
    max_uint_cmov,  /*uint*/
    max_error_func, /*contig*/
    max_error_func, /*vector*/
    max_error_func, /*hvector*/
    max_error_func, /*indexed*/
    max_error_func, /*hindexed*/
    max_error_func, /*struct*/
    max_error_func, /*double_complex*/
    max_error_func, /*packed*/
    max_error_func, /*ub*/
    max_error_func, /*lb*/
    max_double_cmov,/*longdouble*/
    max_error_func, /*longlongint*/
    max_error_func, /*logical*/
    max_sint_sse    /*fort_int*/
};

void MPIR_max_sse_cmov(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_max_sse_cmov[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}

__declspec(naked)
void max_sint_sse(int *inbuf,int*inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_INT			; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]
	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,1				; divide by 2
	jz	remain
		
al_top:	
	movq	mm1, QWORD PTR [eax]		; compare integers
	movq	mm0, QWORD PTR [ecx+eax]	; 2 at a time
	add	eax,8
	movq	mm4,mm0
	pcmpgtd	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
	movq	QWORD PTR [eax-8], mm0		;
	
	dec	edx
	jne	al_top
remain:
	test	bl,1
	jz	startit

	mov	edx,DWORD PTR [ecx+eax]		; compare remaining
	mov	ebx,DWORD PTR [eax]		; integer
	cmp	ebx,edx
	cmovg   edx,ebx
	mov	DWORD PTR [eax],edx
	add	eax,4
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that compares 4*2 integers.
	    
	add	eax, 32				; 
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
	  
	movq	mm4,mm0
	pcmpgtd	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4

	movq	mm2, QWORD PTR [ecx+eax-24]	; 
	movq	mm3, QWORD PTR [eax-24]		;
	movq	QWORD PTR [eax-32], mm0		;
		
	movq	mm4,mm2
	pcmpgtd	mm4,mm3
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4				;

	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	movq	QWORD PTR [eax-24], mm2
		
	movq	mm4,mm0
	pcmpgtd	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4

	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	movq	mm4,mm2
	pcmpgtd	mm4,mm3
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4
	movq	QWORD PTR [eax-8], mm2		;
	
	jne	SHORT ltop			;
unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,7				; calculate remaining 
	shr	edx,1				; number of integers
	jz	norest				; 
lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; add two ints
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	movq	mm4,mm0
	pcmpgtd	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4				;
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					;
	test	bl,1				;
	jz	end				; No--> we are finished
	mov	edx, DWORD PTR [ecx+eax]	; compare the remaining integer
	mov	ebx,DWORD PTR [eax]		; 
	cmp	ebx,edx
	cmovg   edx,ebx
	mov	DWORD PTR [eax],edx
end:
	pop	ebx
	ret	
    }
}

__declspec(naked)
void max_sshort_sse(short *inbuf,short* inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT			; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	remain
		
al_top:	
	movq	mm0, QWORD PTR [ecx+eax]	; compare shorts
	movq	mm1, QWORD PTR [eax]		; 4 at a time
	add	eax,8
	pmaxsw	mm0,mm1
	movq	QWORD PTR [eax-8], mm0		;
	
	dec	edx
	jne	al_top
remain:
	and	bl,3
	jz	startit
al_rest:
	mov	dx,WORD PTR [ecx+eax]		; compare remaining
	cmp	dx,WORD PTR [eax]		; shorts
	cmovl	dx,WORD PTR [eax]	
	mov	WORD PTR [eax],dx
	add	eax,2
	dec	bl
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 4				; len /= 16
	jz	SHORT unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that compares 4*4 shorts.
	    
	add	eax, 32				; 
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
	
	
	
	movq	mm2, QWORD PTR [ecx+eax-24]	; 
	movq	mm3, QWORD PTR [eax-24]		;	
	pmaxsw	mm0,mm1
	movq	QWORD PTR [eax-32], mm0		;
	
	pmaxsw	mm2,mm3
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;	
	movq	QWORD PTR [eax-24], mm2
	
	pmaxsw	mm0,mm1
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	pmaxsw	mm0,mm1
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	pmaxsw	mm2,mm3
	movq	QWORD PTR [eax-8], mm2		;
	
	jne	SHORT ltop			;

unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,15				; calculate remaining 
	shr	edx,2				; number of shorts
	jz	norest				; 

lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; compare 4 shorts
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	pmaxsw	mm0,mm1
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					; restore FP state
	
	and	bl,3				; calculate remaining shorts
	jz	end				; 0 --> we are finished
lshort:
	mov	dx, WORD PTR [ecx+eax]		; compare the remaining shorts
	add	eax,2
	cmp	dx, WORD PTR [eax-2]		;
	cmovl	dx, WORD PTR [eax-2]		;
	dec	bl
	mov	WORD PTR [eax-2],dx		;
	jnz	lshort

end:
	pop	ebx
	ret	
    }
}

__declspec(naked)
void max_sbyte_sse(char *inbuf,char *inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	push	esi
	mov	eax, DWORD PTR [esp+16]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of bytes 
	sub	edx,eax				; that have to be handled	
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+20]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+20],ecx		; store new length

	mov	esi, edx			; Save counter
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,3				; divide by 8
	jz	remain
		
al_top:	
	movq	mm0, QWORD PTR [ecx+eax]	; compare bytes
	movq	mm1, QWORD PTR [eax]		; 8 at a time
	add	eax,8
	dec	edx
	movq	mm4,mm0
	pcmpgtb	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
	movq	QWORD PTR [eax-8], mm0		;
	
	
	jne	al_top
remain:
	and	si,7
	jz	startit
	
al_rest:
	mov	dl,BYTE PTR [ecx+eax]		; compare remaining
	mov	bl,BYTE PTR [eax]
	inc	eax
	cmp	dl,bl				; bytes
	cmovl	dx,bx
	dec	si
	mov	BYTE PTR [eax-1],dl	
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	edx, DWORD PTR [esp+20]		; Load length
	mov	esi,edx				; Save it
	shr	edx, 5				; len /= 32
	jz	SHORT unroll_end		; len==0 --> less than 32 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that compares 4*8 bytes.
	    
	add	eax, 32		
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

	movq	mm4,mm0
	pcmpgtb	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4

	movq	mm2, QWORD PTR [ecx+eax-24]
	movq	mm3, QWORD PTR [eax-24]		;
	movq	QWORD PTR [eax-32], mm0		;
		
	movq	mm4,mm2
	pcmpgtb	mm4,mm3
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	movq	QWORD PTR [eax-24], mm2
		
	movq	mm4,mm0
	pcmpgtb	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4

	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;

	movq	mm4,mm2
	pcmpgtb	mm4,mm3
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4
	movq	QWORD PTR [eax-8], mm2		;
	
	jne	SHORT ltop			;

unroll_end:	
	mov	edx, esi			; load original length
	and	edx,31				; calculate remaining 
	shr	edx,3				; number of bytes
	jz	norest				; 

lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; compare 8 bytes
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	movq	mm4,mm0
	pcmpgtb	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					; restore FP state
	
	and	si,7				; calculate remaining bytes
	jz	end				; 0 --> we are finished
align 4
lbyte:
	mov	dl, BYTE PTR [ecx+eax]		; compare the remaining bytes
	mov	bl, BYTE PTR [eax]		;
	inc	eax
	cmp	dl, bl				;
	cmovl	dx,bx
	dec	si
	mov	BYTE PTR [eax-1],dl		;	
	jnz	lbyte

end:
	pop	esi
	pop	ebx
	ret	
    }
}

__declspec(naked)
void max_ubyte_sse(unsigned char *inbuf,unsigned char *inoutbuf,unsigned int len) {
__asm{	
	push	ebx
	push	esi
	mov	eax, DWORD PTR [esp+16]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of bytes 
	sub	edx,eax				; that have to be handled	
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+20]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+20],ecx		; store new length

	mov	esi, edx			; Save counter
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,3				; divide by 8
	jz	remain
		
al_top:	
	movq	mm0, QWORD PTR [ecx+eax]	; compare bytes
	movq	mm1, QWORD PTR [eax]		; 8 at a time
	add	eax,8
	dec	edx
	pmaxub	mm0,mm1
	movq	QWORD PTR [eax-8], mm0		;	
	jne	al_top
remain:
	and	si,7
	jz	startit
	
al_rest:
	mov	dl,BYTE PTR [ecx+eax]		; compare remaining
	mov	bl,BYTE PTR [eax]
	inc	eax
	cmp	dl,bl				; bytes
	cmovb	dx,bx
	dec	si
	mov	BYTE PTR [eax-1],dl	
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	edx, DWORD PTR [esp+20]		; Load length
	mov	esi,edx				; Save it
	shr	edx, 5				; len /= 32
	jz	SHORT unroll_end		; len==0 --> less than 32 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that compares 4*8 bytes.
	    
	add	eax, 32		
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

	movq	mm2, QWORD PTR [ecx+eax-24]
	movq	mm3, QWORD PTR [eax-24]		;
	  
	pmaxub	mm0,mm1
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	pmaxub	mm2,mm3
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	pmaxub	mm0,mm1
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;

	pmaxub	mm2,mm3
	movq	QWORD PTR [eax-8], mm2		;
	
	jne	SHORT ltop			;

unroll_end:	
	mov	edx, esi			; load original length
	and	edx,31				; calculate remaining 
	shr	edx,3				; number of bytes
	jz	norest				; 

lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; compare 8 bytes
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	pmaxub	mm0,mm1
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					; restore FP state
	
	and	si,7				; calculate remaining bytes
	jz	end				; 0 --> we are finished
align 4
lbyte:
	mov	dl, BYTE PTR [ecx+eax]		; compare the remaining bytes
	mov	bl, BYTE PTR [eax]		;
	inc	eax
	cmp	dl, bl				;
	cmovb	dx,bx
	dec	si
	mov	BYTE PTR [eax-1],dl		;	
	jnz	lbyte

end:
	pop	esi
	pop	ebx
	ret	
    }
}

__declspec(naked)
void max_uint_cmov(unsigned int *inbuf,unsigned int *inoutbuf,unsigned int len) {
__asm{
	push	ebx
	push	esi
	mov	ecx,DWORD PTR [esp+20]		; Load length
	test	ecx,ecx
	jz	end

	mov	eax, DWORD PTR [esp+16]		; load inoutbuf
	mov	ebx,eax				; Save address of inoutbuf
	

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_INT
	
	mov	eax,ebx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+20]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+20],ecx		; store new length

	mov	esi, edx			; Save counter
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
align 4			
al_top:	
	mov	edx,DWORD PTR [ecx+eax]		; 
	mov	ebx,DWORD PTR [eax]
	add	eax,4
	cmp	edx,ebx				; 
	cmovb	edx,ebx
	dec	si
	mov	DWORD PTR [eax-4],edx	
	jnz	al_top
	jmp	startit

aligned:	
	mov	eax, ebx
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	esi, DWORD PTR [esp+20]		; Load length
	shr	esi, 3				; len /= 8
	jz	SHORT unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:	
	mov	ebx, DWORD PTR [ecx+eax]	; This is an unrolled loop
	mov	edx, DWORD PTR [eax]		; that compares 8 integers.

	add	eax, 32	
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

	cmp	ebx, edx
	cmova	edx, ebx
	mov	DWORD PTR [eax-32],edx

	mov	ebx, DWORD PTR [ecx+eax-28]	
	mov	edx, DWORD PTR [eax-28]		
	cmp	ebx, edx
	cmova	edx, ebx
	mov	DWORD PTR [eax-28],edx

	mov	ebx, DWORD PTR [ecx+eax-24]	
	mov	edx, DWORD PTR [eax-24]		
	cmp	ebx, edx
	cmova	edx, ebx
	mov	DWORD PTR [eax-24],edx

	mov	ebx, DWORD PTR [ecx+eax-20]	
	mov	edx, DWORD PTR [eax-20]		
	cmp	ebx, edx
	cmova	edx, ebx
	mov	DWORD PTR [eax-20],edx

	mov	ebx, DWORD PTR [ecx+eax-16]	
	mov	edx, DWORD PTR [eax-16]	
	cmp	ebx, edx
	cmova	edx, ebx
	mov	DWORD PTR [eax-16],edx

	mov	ebx, DWORD PTR [ecx+eax-12]	
	mov	edx, DWORD PTR [eax-12]		
	cmp	ebx, edx
	cmova	edx, ebx
	mov	DWORD PTR [eax-12],edx

	mov	ebx, DWORD PTR [ecx+eax-8]	
	mov	edx, DWORD PTR [eax-8]		
	cmp	ebx, edx
	cmova	edx, ebx
	mov	DWORD PTR [eax-8],edx

	mov	ebx, DWORD PTR [ecx+eax-4]	
	mov	edx, DWORD PTR [eax-4]		
	cmp	ebx, edx
	cmova	edx, ebx
	dec	esi	
	mov	DWORD PTR [eax-4],edx

	jne	SHORT ltop

unroll_end:	
	mov	esi, DWORD PTR [esp+20]		; load original length
	and	si,7				; calculate remaining 
	jz	end
align 4
lrest:	
	mov	edx, DWORD PTR [ecx+eax]	; compare the remaining integers
	mov	ebx, DWORD PTR [eax]		; 
	add	eax, 4				
	cmp	ebx, edx
	cmova   edx, ebx
	dec	si
	mov	DWORD PTR [eax-4],edx	
	jnz	lrest
end:
	pop	esi
	pop	ebx
	ret	
    }
}

__declspec(naked)
void max_ushort_cmov(unsigned short *inbuf,unsigned short *inoutbuf,unsigned int len) {
__asm{
	push	ebx
	push	esi

	mov	ecx,DWORD PTR [esp+20]		; Load length
	test	ecx, ecx
	jz	end

	mov	eax, DWORD PTR [esp+16]		; load inoutbuf
	mov	ebx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT
	
	mov	eax,ebx				; restore address of inoutbuf
	prefetchnta [eax+32]

	
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length

	mov	DWORD PTR [esp+20],ecx		; store new length
	mov	esi, edx			; Save counter

	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
align 4			
al_top:	
	mov	dx,WORD PTR [ecx+eax]		; 
	mov	bx,WORD PTR [eax]
	add	eax,2
	cmp	dx,bx				; 
	cmova	bx,dx
	dec	si
	mov	WORD PTR [eax-2],bx
	jnz	al_top
	jmp	startit

aligned:	
	mov	eax, ebx
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	esi, DWORD PTR [esp+20]		; Load length
	shr	esi, 4				; len /= 16
	jz	SHORT unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:	
	mov	bx, WORD PTR [ecx+eax]		; This is an unrolled loop
	mov	dx, WORD PTR [eax]		; that compares 16 shorts.

	add	eax, 32	
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

	cmp	bx, dx
	cmova	dx, bx
	mov	WORD PTR [eax-32],dx

	mov	bx, WORD PTR [ecx+eax-30]	
	mov	dx, WORD PTR [eax-30]		
	cmp	bx, dx
	cmova	dx, bx
	mov	WORD PTR [eax-30],dx

	mov	bx, WORD PTR [ecx+eax-28]	
	mov	dx, WORD PTR [eax-28]		
	cmp	bx, dx
	cmova	dx, bx
	mov	WORD PTR [eax-28],dx

	mov	bx, WORD PTR [ecx+eax-26]	
	mov	dx, WORD PTR [eax-26]		
	cmp	bx, dx
	cmova	dx, bx
	mov	WORD PTR [eax-26],dx

	mov	bx, WORD PTR [ecx+eax-24]	
	mov	dx, WORD PTR [eax-24]	
	cmp	bx, dx
	cmova	dx, bx
	mov	WORD PTR [eax-24],dx

	mov	bx, WORD PTR [ecx+eax-22]	
	mov	dx, WORD PTR [eax-22]		
	cmp	bx, dx
	cmova	dx, bx
	mov	WORD PTR [eax-22],dx

	mov	bx, WORD PTR [ecx+eax-20]	
	mov	dx, WORD PTR [eax-20]		
	cmp	bx, dx
	cmova	dx, bx
	mov	WORD PTR [eax-20],dx

	mov	bx, WORD PTR [ecx+eax-18]	
	mov	dx, WORD PTR [eax-18]		
	cmp	bx, dx
	cmova	dx, bx	
	mov	WORD PTR [eax-18],dx

	mov	bx, WORD PTR [ecx+eax-16]		
	mov	dx, WORD PTR [eax-16]		
	cmp	bx, dx
	cmova	dx, bx
	mov	WORD PTR [eax-16],dx

	mov	bx, WORD PTR [ecx+eax-14]	
	mov	dx, WORD PTR [eax-14]		
	cmp	bx, dx
	cmova	dx, bx
	mov	WORD PTR [eax-14],dx

	mov	bx, WORD PTR [ecx+eax-12]	
	mov	dx, WORD PTR [eax-12]		
	cmp	bx, dx
	cmova	dx, bx
	mov	WORD PTR [eax-12],dx

	mov	bx, WORD PTR [ecx+eax-10]	
	mov	dx, WORD PTR [eax-10]		
	cmp	bx, dx
	cmova	dx, bx
	mov	WORD PTR [eax-10],dx

	mov	bx, WORD PTR [ecx+eax-8]	
	mov	dx, WORD PTR [eax-8]	
	cmp	bx, dx
	cmova	dx, bx
	mov	WORD PTR [eax-8],dx

	mov	bx, WORD PTR [ecx+eax-6]	
	mov	dx, WORD PTR [eax-6]		
	cmp	bx, dx
	cmova	dx, bx
	mov	WORD PTR [eax-6],dx

	mov	bx, WORD PTR [ecx+eax-4]	
	mov	dx, WORD PTR [eax-4]		
	cmp	bx, dx
	cmova	dx, bx
	mov	WORD PTR [eax-4],dx

	mov	bx, WORD PTR [ecx+eax-2]	
	mov	dx, WORD PTR [eax-2]		
	cmp	bx, dx
	cmova	dx, bx
	dec	esi	
	mov	WORD PTR [eax-2],dx
	jne	SHORT ltop

unroll_end:	
	mov	esi, DWORD PTR [esp+20]		; load original length
	and	si,15				; calculate remaining 
	jz	end
align 4
lrest:	
	mov	dx, WORD PTR [ecx+eax]	; compare the remaining shorts
	mov	bx, WORD PTR [eax]		; 
	add	eax, 2				
	cmp	bx, dx
	cmova   dx, bx
	dec	si
	mov	WORD PTR [eax-2],dx	
	jnz	lrest
end:
	pop	esi
	pop	ebx
	ret	
    }
}


__declspec(naked)
void max_float_sse(float *inbuf,float* inoutbuf,unsigned int len) {
__asm{	
	push	ebx
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_FLOAT			; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	remain
		
al_top:	
	movups	xmm0, XMMWORD PTR [ecx+eax]	; compare floats
	movups	xmm1, XMMWORD PTR [eax]
	maxps	xmm0, xmm1			; 4 at a time
	add	eax,16
	dec	edx
	movups	XMMWORD PTR [eax-16], xmm0	;
	jne	al_top
remain:
	and	bl,3
	jz	startit
al_rest:
	movss	xmm0,DWORD PTR [ecx+eax]
	maxss	xmm0,DWORD PTR [eax]
	add	eax,4
	movss	DWORD PTR [eax-4],xmm0
	dec	bl
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:
	movaps	xmm0, XMMWORD PTR [ecx+eax]	; This is an unrolled loop
	movaps	xmm1, XMMWORD PTR [eax]		; that compares 2*4 floats.
	    
	add	eax, 32				; 
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
	maxps	xmm0,xmm1

	movaps	xmm2, XMMWORD PTR [ecx+eax-16]	; 
	movaps	xmm3, XMMWORD PTR [eax-16]	;

	maxps	xmm2,xmm3
	movaps	XMMWORD PTR [eax-32], xmm0	;
	dec	edx			
	movaps	XMMWORD PTR [eax-16], xmm2
	jne	SHORT ltop			;

unroll_end:	
	test	bl, 4				; at least 4 numbers left?
	jz	norest				; 

	movaps	xmm0, XMMWORD PTR [ecx+eax]	; compare 4 floats
	movaps	xmm1, XMMWORD PTR [eax]		;
	add	eax,16
	maxps	xmm0,xmm1
	movaps	XMMWORD PTR [eax-16], xmm0

norest:	
	and	bl,3				; calculate remaining floats
	jz	end				; 0 --> we are finished
lfloat:
	movss	xmm0, DWORD PTR [ecx+eax]	; compare the remaining floats
	maxss	xmm0, DWORD PTR [eax]		;
	add	eax,4
	dec	bl
	movss	DWORD PTR [eax-4],xmm0		;
	jnz	lfloat

end:
	pop	ebx
	ret	
    }
}


/* WARNING:	doesn't check for 8-Byte Alignment. Alignment isn't
   needed for FPU instructions. May change later (SSE2...) */

__declspec(naked)
void max_double_cmov(double *inbuf,double* inoutbuf,unsigned int len) {
__asm{

	push	ebx

	mov	ecx,DWORD PTR [esp+16]		; Load length
	test	ecx,ecx
	jz	end

	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ebx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_DOUBLE			; to get aligned addresses
	
	mov	eax,ebx				; restore address of inoutbuf
	prefetchnta [eax+32]

	
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
		
align 4	
al_top:	
	fld	QWORD PTR [ecx+eax]		; 
	fld	QWORD PTR [eax]			; 
	add	eax, 8	
	fucomi	st(0),st(1)
	fcmovb	st(0),st(1)
	fstp	QWORD PTR [eax-8]
	dec	edx
	ffree	st(0)	
	jg	al_top                          ; jg allows him to work with "not 8-Byte aligned" memory
	jmp	startit
aligned:	
	mov	eax, ebx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 2				; len /= 4
	jz	SHORT unroll_end		; len==0 --> less than 4 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:
	fld	QWORD PTR [ecx+eax]		; This is an unrolled loop
	fld	QWORD PTR [eax]			; that compares 4 doubles.
	add	eax, 32	

	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

	fucomi	st(0),st(1)
	fcmovb	st(0),st(1)
	fstp	QWORD PTR [eax-32]
	ffree	st(0)

	fld	QWORD PTR [ecx+eax-24]
	fld	QWORD PTR [eax-24]
	fucomi	st(0),st(1)
	fcmovb	st(0),st(1)
	fstp	QWORD PTR [eax-24]
	ffree	st(0)

	fld	QWORD PTR [ecx+eax-16]
	fld	QWORD PTR [eax-16]
	fucomi	st(0),st(1)
	fcmovb	st(0),st(1)
	fstp	QWORD PTR [eax-16]
	ffree	st(0)

	fld	QWORD PTR [ecx+eax-8]
	fld	QWORD PTR [eax-8]
	fucomi	st(0),st(1)
	fcmovb	st(0),st(1)
	dec	edx
	fstp	QWORD PTR [eax-8]
	ffree	st(0)
	jne	SHORT ltop

unroll_end:	
	and	bl,3				; calculate remaining 
	jz	end

lrest:	
	fld	QWORD PTR [ecx+eax]		; compare the remaining numbers
	fld	QWORD PTR [eax]			; 
	add	eax, 8				
	fucomi	st(0),st(1)
	fcmovb	st(0),st(1)
	dec	bl
	fstp	QWORD PTR [eax-8]
	ffree	st(0)
	jnz	lrest
end:
	pop	ebx
	ret	
    }
}


/* ---------------------MPI_MIN----------------------------------*/
extern void min_error_func(void* d1,void*d2,unsigned int l);
static void min_sint_sse(int *inbuf,int*inoutbuf,unsigned int len);
static void min_sshort_sse(short *inbuf,short *inoutbuf,unsigned int len);
static void min_sbyte_sse(signed char *inbuf,signed char *inoutbuf,unsigned int len);

static void min_float_sse(float *inbuf,float *inoutbuf,unsigned int len);
static void min_double_cmov(double *inbuf,double *inoutbuf,unsigned int len);
static void min_uint_cmov(unsigned int *inbuf,unsigned int*inoutbuf,unsigned int len);
static void min_ushort_cmov(unsigned short *inbuf,unsigned short *inoutbuf,unsigned int len);
static void min_ubyte_sse(unsigned char *inbuf,unsigned char *inoutbuf,unsigned int len);


static  const proto_func jmp_min_sse_cmov[] = {
    min_sint_sse,   /*int*/
    min_float_sse, /*float*/
    min_double_cmov,/*double*/
    min_error_func, /*complex*/
    min_sint_sse,   /*long*/
    min_sshort_sse, /*short*/
    min_sbyte_sse,  /*char*/
    min_ubyte_sse, /*byte*/
    min_ubyte_sse, /*uchar*/
    min_ushort_cmov,/*ushort*/
    min_uint_cmov,  /*ulong*/
    min_uint_cmov,  /*uint*/
    min_error_func, /*contig*/
    min_error_func, /*vector*/
    min_error_func, /*hvector*/
    min_error_func, /*indexed*/
    min_error_func, /*hindexed*/
    min_error_func, /*struct*/
    min_error_func, /*double_complex*/
    min_error_func, /*packed*/
    min_error_func, /*ub*/
    min_error_func, /*lb*/
    min_double_cmov, /*longdouble*/
    min_error_func, /*longlongint*/
    min_error_func, /*logical*/
    min_sint_sse    /*fort_int*/
};

void MPIR_min_sse_cmov(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_min_sse_cmov[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}



__declspec(naked)
void min_sint_sse(int *inbuf,int*inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_INT			; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,1				; divide by 2
	jz	remain
	
	
al_top:	
	movq	mm1, QWORD PTR [eax]		; compare integers
	movq	mm0, QWORD PTR [ecx+eax]	; 2 at a time
	add	eax,8
	movq	mm4,mm1
	pcmpgtd	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
	movq	QWORD PTR [eax-8], mm0		;
	
	dec	edx
	jne	al_top
remain:
	test	bl,1
	jz	startit

	mov	edx,DWORD PTR [ecx+eax]		; compare remaining
	mov	ebx,DWORD PTR [eax]		; integer
	cmp	edx,ebx
	cmovg   edx,ebx
	mov	DWORD PTR [eax],edx
	add	eax,4
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that compares 4*2 integers.
	    
	add	eax, 32				; 
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
						; 	  
	movq	mm4,mm1
	pcmpgtd	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4

	movq	mm2, QWORD PTR [ecx+eax-24]	; 
	movq	mm3, QWORD PTR [eax-24]		;
	movq	QWORD PTR [eax-32], mm0		;
		
	movq	mm4,mm3
	pcmpgtd	mm4,mm2
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4				;

	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	movq	QWORD PTR [eax-24], mm2
		
	movq	mm4,mm1
	pcmpgtd	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4

	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	movq	mm4,mm3
	pcmpgtd	mm4,mm2
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4
	movq	QWORD PTR [eax-8], mm2		;
	
	jne	SHORT ltop			;
unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,7				; calculate remaining 
	shr	edx,1				; number of integers
	jz	norest				; 
lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; add two ints
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	movq	mm4,mm1
	pcmpgtd	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4				;
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					;
	test	bl,1				;
	jz	end				; No--> we are finished
	mov	edx, DWORD PTR [ecx+eax]	; compare the remaining integer
	mov	ebx,DWORD PTR [eax]		; 
	cmp	edx,ebx
	cmovg   edx,ebx
	mov	DWORD PTR [eax],edx
end:
	pop	ebx
	ret	
    }
}

__declspec(naked)
void min_sshort_sse(short *inbuf,short* inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT			; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	remain
		
al_top:	
	movq	mm0, QWORD PTR [ecx+eax]	; compare shorts
	movq	mm1, QWORD PTR [eax]		; 4 at a time
	add	eax,8
	pminsw	mm0,mm1
	movq	QWORD PTR [eax-8], mm0		;
	
	dec	edx
	jne	al_top
remain:
	and	bl,3
	jz	startit
al_rest:
	mov	dx,WORD PTR [ecx+eax]		; compare remaining
	cmp	dx,WORD PTR [eax]		; shorts
	cmovg	dx,WORD PTR [eax]	
	mov	WORD PTR [eax],dx
	add	eax,2
	dec	bl
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 4				; len /= 16
	jz	SHORT unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that compares 4*4 shorts.
	    
	add	eax, 32				; 
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
	
	pminsw	mm0,mm1
	
	movq	mm2, QWORD PTR [ecx+eax-24]	; 
	movq	mm3, QWORD PTR [eax-24]		;	
	movq	QWORD PTR [eax-32], mm0		;
	
	pminsw	mm2,mm3
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;	
	movq	QWORD PTR [eax-24], mm2
	
	pminsw	mm0,mm1
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	pminsw	mm2,mm3
	movq	QWORD PTR [eax-8], mm2		;
	
	jne	SHORT ltop			;

unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,15				; calculate remaining 
	shr	edx,2				; number of shorts
	jz	norest				; 

lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; compare 4 shorts
	movq	mm1, QWORD PTR [eax]		;
	pminsw	mm0,mm1
	add	eax,8
	dec	edx				;
	movq	QWORD PTR [eax-8], mm0		;	
	jne	lrest				;
norest:
	
	emms					; restore FP state
	
	and	bl,3				; calculate remaining shorts
	jz	end				; 0 --> we are finished
lshort:
	mov	dx, WORD PTR [ecx+eax]		; compare the remaining shorts
	add	eax,2
	cmp	dx, WORD PTR [eax-2]		;
	cmovg	dx, WORD PTR [eax-2]		;
	dec	bl
	mov	WORD PTR [eax-2],dx		;
	jnz	lshort

end:
	pop	ebx
	ret	
    }
}



__declspec(naked)
void min_sbyte_sse(char *inbuf,char *inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	push	esi
	mov	eax, DWORD PTR [esp+16]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of bytes 
	sub	edx,eax				; that have to be handled	
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+20]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+20],ecx		; store new length

	mov	esi, edx			; Save counter
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,3				; divide by 8
	jz	remain
		
al_top:	
	movq	mm0, QWORD PTR [ecx+eax]	; compare bytes
	movq	mm1, QWORD PTR [eax]		; 8 at a time
	add	eax,8
	dec	edx
	movq	mm4,mm1
	pcmpgtb	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
	movq	QWORD PTR [eax-8], mm0		;
	
	
	jne	al_top
remain:
	and	si,7
	jz	startit
	
al_rest:
	mov	dl,BYTE PTR [ecx+eax]		; compare remaining
	mov	bl,BYTE PTR [eax]
	inc	eax
	cmp	dl,bl				; bytes
	cmova	dx,bx
	dec	si
	mov	BYTE PTR [eax-1],dl	
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	edx, DWORD PTR [esp+20]		; Load length
	mov	esi,edx				; Save it
	shr	edx, 5				; len /= 32
	jz	SHORT unroll_end		; len==0 --> less than 32 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that compares 4*8 bytes.
	    
	add	eax, 32		
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

	movq	mm4,mm1
	pcmpgtb	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4

	movq	mm2, QWORD PTR [ecx+eax-24]
	movq	mm3, QWORD PTR [eax-24]		;
	movq	QWORD PTR [eax-32], mm0		;
		
	movq	mm4,mm3
	pcmpgtb	mm4,mm2
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	movq	QWORD PTR [eax-24], mm2
		
	movq	mm4,mm1
	pcmpgtb	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4

	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;

	movq	mm4,mm3
	pcmpgtb	mm4,mm2
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4
	movq	QWORD PTR [eax-8], mm2		;
	
	jne	SHORT ltop			;

unroll_end:	
	mov	edx, esi			; load original length
	and	edx,31				; calculate remaining 
	shr	edx,3				; number of bytes
	jz	norest				; 

lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; compare 8 bytes
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	movq	mm4,mm1
	pcmpgtb	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					; restore FP state
	
	and	si,7				; calculate remaining bytes
	jz	end				; 0 --> we are finished
align 4
lbyte:
	mov	dl, BYTE PTR [ecx+eax]		; compare the remaining bytes
	mov	bl, BYTE PTR [eax]		;
	inc	eax
	cmp	dl, bl				;
	cmova	dx,bx
	dec	si
	mov	BYTE PTR [eax-1],dl		;	
	jnz	lbyte

end:
	pop	esi
	pop	ebx
	ret	
    }
}

__declspec(naked)
void min_uint_cmov(unsigned int *inbuf,unsigned int*inoutbuf,unsigned int len) {
__asm{
	push	ebx
	push	esi
	mov	ecx,DWORD PTR [esp+20]		; Load length
	test	ecx,ecx
	jz	end

	mov	eax, DWORD PTR [esp+16]		; load inoutbuf
	mov	ebx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_INT
	
	mov	eax,ebx				; restore address of inoutbuf
	prefetchnta [eax+32]

	
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+20],ecx		; store new length

	mov	esi, edx			; Save counter
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
align 4			
al_top:	
	mov	edx,DWORD PTR [ecx+eax]		; 
	mov	ebx,DWORD PTR [eax]
	add	eax, 4
	cmp	ebx,edx				; 
	cmovb	edx,ebx
	dec	si
	mov	DWORD PTR [eax-4],edx	
	jnz	al_top
	jmp	startit

aligned:	
	mov	eax, ebx
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	esi, DWORD PTR [esp+20]		; Load length
	shr	esi, 3				; len /= 8
	jz	SHORT unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:	
	mov	ebx, DWORD PTR [ecx+eax]	; This is an unrolled loop
	mov	edx, DWORD PTR [eax]		; that compares 8 integers.

	add	eax, 32	
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

	cmp	ebx, edx
	cmovb	edx, ebx
	mov	DWORD PTR [eax-32],edx

	mov	ebx, DWORD PTR [ecx+eax-28]	
	mov	edx, DWORD PTR [eax-28]		
	cmp	ebx, edx
	cmovb	edx, ebx
	mov	DWORD PTR [eax-28],edx

	mov	ebx, DWORD PTR [ecx+eax-24]	
	mov	edx, DWORD PTR [eax-24]		
	cmp	ebx, edx
	cmovb	edx, ebx
	mov	DWORD PTR [eax-24],edx

	mov	ebx, DWORD PTR [ecx+eax-20]	
	mov	edx, DWORD PTR [eax-20]		
	cmp	ebx, edx
	cmovb	edx, ebx
	mov	DWORD PTR [eax-20],edx

	mov	ebx, DWORD PTR [ecx+eax-16]	
	mov	edx, DWORD PTR [eax-16]	
	cmp	ebx, edx
	cmovb	edx, ebx
	mov	DWORD PTR [eax-16],edx

	mov	ebx, DWORD PTR [ecx+eax-12]	
	mov	edx, DWORD PTR [eax-12]		
	cmp	ebx, edx
	cmovb	edx, ebx
	mov	DWORD PTR [eax-12],edx

	mov	ebx, DWORD PTR [ecx+eax-8]	
	mov	edx, DWORD PTR [eax-8]		
	cmp	ebx, edx
	cmovb	edx, ebx
	mov	DWORD PTR [eax-8],edx

	mov	ebx, DWORD PTR [ecx+eax-4]	
	mov	edx, DWORD PTR [eax-4]		
	cmp	ebx, edx
	cmovb	edx, ebx
	dec	esi	
	mov	DWORD PTR [eax-4],edx

	jne	SHORT ltop

unroll_end:	
	mov	esi, DWORD PTR [esp+20]		; load original length
	and	si,7				; calculate remaining 
	jz	end
align 4
lrest:	
	mov	edx, DWORD PTR [ecx+eax]	; compare the remaining integers
	mov	ebx, DWORD PTR [eax]		; 
	add	eax, 4				
	cmp	ebx, edx
	cmovb   edx, ebx
	dec	si
	mov	DWORD PTR [eax-4],edx	
	jnz	lrest
end:
	pop	esi
	pop	ebx
	ret	
    }
}

__declspec(naked)
void min_ushort_cmov(unsigned short *inbuf,unsigned short *inoutbuf,unsigned int len) {
__asm{
	push	ebx
	push	esi
	
	mov	ecx,DWORD PTR [esp+20]		; Load length
	test	ecx,ecx
	jz	end

	mov	eax, DWORD PTR [esp+16]		; load inoutbuf
	mov	ebx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT
	
	mov	eax,ebx				; restore address of inoutbuf
	prefetchnta [eax+32]

	cmp	ecx,edx				; Is length smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+20],ecx		; store new length
	mov	esi, edx

	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
align 4			
al_top:	
	mov	dx,WORD PTR [ecx+eax]		; 
	mov	bx,WORD PTR [eax]
	add	eax, 2
	cmp	bx,dx				; 
	cmovb	dx,bx
	dec	si
	mov	WORD PTR [eax-2],dx	
	jnz	al_top
	jmp	startit

aligned:	
	mov	eax, ebx
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	esi, DWORD PTR [esp+20]		; Load length
	shr	esi, 4				; len /= 16
	jz	SHORT unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:	
	mov	bx, WORD PTR [ecx+eax]		; This is an unrolled loop
	mov	dx, WORD PTR [eax]		; that compares 16 shorts.

	add	eax, 32	
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

	cmp	bx, dx
	cmovb	dx, bx
	mov	WORD PTR [eax-32],dx

	mov	bx, WORD PTR [ecx+eax-30]	
	mov	dx, WORD PTR [eax-30]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	WORD PTR [eax-30],dx

	mov	bx, WORD PTR [ecx+eax-28]	
	mov	dx, WORD PTR [eax-28]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	WORD PTR [eax-28],dx

	mov	bx, WORD PTR [ecx+eax-26]	
	mov	dx, WORD PTR [eax-26]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	WORD PTR [eax-26],dx

	mov	bx, WORD PTR [ecx+eax-24]	
	mov	dx, WORD PTR [eax-24]	
	cmp	bx, dx
	cmovb	dx, bx
	mov	WORD PTR [eax-24],dx

	mov	bx, WORD PTR [ecx+eax-22]	
	mov	dx, WORD PTR [eax-22]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	WORD PTR [eax-22],dx

	mov	bx, WORD PTR [ecx+eax-20]	
	mov	dx, WORD PTR [eax-20]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	WORD PTR [eax-20],dx

	mov	bx, WORD PTR [ecx+eax-18]	
	mov	dx, WORD PTR [eax-18]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	WORD PTR [eax-18],dx

	mov	bx, WORD PTR [ecx+eax-16]		
	mov	dx, WORD PTR [eax-16]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	WORD PTR [eax-16],dx

	mov	bx, WORD PTR [ecx+eax-14]	
	mov	dx, WORD PTR [eax-14]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	WORD PTR [eax-14],dx

	mov	bx, WORD PTR [ecx+eax-12]	
	mov	dx, WORD PTR [eax-12]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	WORD PTR [eax-12],dx

	mov	bx, WORD PTR [ecx+eax-10]	
	mov	dx, WORD PTR [eax-10]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	WORD PTR [eax-10],dx

	mov	bx, WORD PTR [ecx+eax-8]	
	mov	dx, WORD PTR [eax-8]	
	cmp	bx, dx
	cmovb	dx, bx
	mov	WORD PTR [eax-8],dx

	mov	bx, WORD PTR [ecx+eax-6]	
	mov	dx, WORD PTR [eax-6]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	WORD PTR [eax-6],dx

	mov	bx, WORD PTR [ecx+eax-4]	
	mov	dx, WORD PTR [eax-4]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	WORD PTR [eax-4],dx

	mov	bx, WORD PTR [ecx+eax-2]	
	mov	dx, WORD PTR [eax-2]		
	cmp	bx, dx
	cmovb	dx, bx
	dec	esi	
	mov	WORD PTR [eax-2],dx
	jne	SHORT ltop

unroll_end:	
	mov	esi, DWORD PTR [esp+20]		; load original length
	and	si,15				; calculate remaining 
	jz	end
align 4
lrest:	
	mov	dx, WORD PTR [ecx+eax]	; compare the remaining shorts
	mov	bx, WORD PTR [eax]		; 
	add	eax, 2				
	cmp	bx, dx
	cmovb   dx, bx
	dec	si
	mov	WORD PTR [eax-2],dx	
	jnz	lrest
end:
	pop	esi
	pop	ebx
	ret	
    }
}

__declspec(naked)
void min_ubyte_sse(unsigned char *inbuf,unsigned char *inoutbuf,unsigned int len) {
__asm{	
	push	ebx
	push	esi

	mov	ecx,DWORD PTR [esp+20]		; Load length
	test	ecx,ecx
	jz	end

	mov	eax, DWORD PTR [esp+16]		; load inoutbuf
	mov	ebx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of bytes 
	sub	edx,eax				; that have to be handled	
	
	mov	eax,ebx				; restore address of inoutbuf
	prefetchnta [eax+32]


	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+20],ecx		; store new length

	mov	esi, edx			; Save counter
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,3				; divide by 8
	jz	remain
		
al_top:	
	movq	mm0, QWORD PTR [ecx+eax]	; compare bytes
	movq	mm1, QWORD PTR [eax]		; 8 at a time
	add	eax,8
	dec	edx
	pminub	mm0,mm1
	movq	QWORD PTR [eax-8], mm0		;	
	jne	al_top
remain:
	and	si,7
	jz	startit
	
al_rest:
	mov	dl,BYTE PTR [ecx+eax]		; compare remaining
	mov	bl,BYTE PTR [eax]
	inc	eax
	cmp	dl,bl				; bytes
	cmova	dx,bx
	dec	si
	mov	BYTE PTR [eax-1],dl	
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ebx
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	edx, DWORD PTR [esp+20]		; Load length
	mov	esi,edx				; Save it
	shr	edx, 5				; len /= 32
	jz	SHORT unroll_end		; len==0 --> less than 32 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that compares 4*8 bytes.
	    
	add	eax, 32		
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

	movq	mm2, QWORD PTR [ecx+eax-24]
	movq	mm3, QWORD PTR [eax-24]		;
	  
	pminub	mm0,mm1
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	pminub	mm2,mm3
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	pminub	mm0,mm1
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;

	pminub	mm2,mm3
	movq	QWORD PTR [eax-8], mm2		;
	
	jne	SHORT ltop			;

unroll_end:	
	mov	edx, esi			; load original length
	and	edx,31				; calculate remaining 
	shr	edx,3				; number of bytes
	jz	norest				; 

lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; compare 8 bytes
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	pminub	mm0,mm1
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					; restore FP state
	
	and	si,7				; calculate remaining bytes
	jz	end				; 0 --> we are finished
align 4
lbyte:
	mov	dl, BYTE PTR [ecx+eax]		; compare the remaining bytes
	mov	bl, BYTE PTR [eax]		;
	inc	eax
	cmp	dl, bl				;
	cmova	dx,bx
	dec	si
	mov	BYTE PTR [eax-1],dl		;	
	jnz	lbyte

end:
	pop	esi
	pop	ebx
	ret	
    }
}

__declspec(naked)
void min_float_sse(float *inbuf,float* inoutbuf,unsigned int len) {
__asm{	
	push	ebx
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_FLOAT			; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	remain
		
al_top:	
	movups	xmm0, XMMWORD PTR [ecx+eax]	; compare floats
	movups	xmm1, XMMWORD PTR [eax]		; 4 at a time
	minps	xmm0, xmm1
	add	eax,16
	dec	edx
	movups	XMMWORD PTR [eax-16], xmm0	;
	jne	al_top
remain:
	and	bl,3
	jz	startit
al_rest:
	movss	xmm0,DWORD PTR [ecx+eax]
	minss	xmm0,DWORD PTR [eax]
	add	eax,4
	dec	bl
	movss	DWORD PTR [eax-4],xmm0
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:
	movaps	xmm0, XMMWORD PTR [ecx+eax]	; This is an unrolled loop
	movaps	xmm1, XMMWORD PTR [eax]		; that compares 2*4 floats.
	    
	add	eax, 32				; 
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
	movaps	xmm2, XMMWORD PTR [ecx+eax-16]	; 
	movaps	xmm3, XMMWORD PTR [eax-16]	;
	  
	minps	xmm0,xmm1
	movaps	XMMWORD PTR [eax-32], xmm0	;

	dec	edx			
	minps	xmm2,xmm3
	movaps	XMMWORD PTR [eax-16], xmm2
	jne	SHORT ltop			;

unroll_end:	
	test	bl, 4				; at least 4 numbers left?
	jz	norest				; 

	movaps	xmm0, XMMWORD PTR [ecx+eax]	; compare 4 floats
	movaps	xmm1, XMMWORD PTR [eax]		;
	add	eax,16
	minps	xmm0,xmm1
	movaps	XMMWORD PTR [eax-16], xmm0

norest:	
	and	bl,3				; calculate remaining floats
	jz	end				; 0 --> we are finished
lfloat:
	movss	xmm0, DWORD PTR [ecx+eax]	; compare the remaining floats
	add	eax,4
	minss	xmm0, DWORD PTR [eax-4]		;
	dec	bl
	movss	DWORD PTR [eax-4],xmm0		;
	jnz	lfloat

end:
	pop	ebx
	ret	
    }
}


/* WARNING:	doesn't check for 8-Byte Alignment. Alignment isn't
   needed for FPU instructions. May change later (SSE2...) */

__declspec(naked)
void min_double_cmov(double *inbuf,double* inoutbuf,unsigned int len) {
__asm{

	push	ebx

	mov	ecx,DWORD PTR [esp+16]		; Load length
	test	ecx,ecx
	jz	end

	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ebx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_DOUBLE			; to get aligned addresses
	
	mov	eax,ebx				; restore address of inoutbuf
	prefetchnta [eax+32]

	
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
		
align 4	
al_top:	
	fld	QWORD PTR [ecx+eax]		; 
	fld	QWORD PTR [eax]			; 
	add	eax, 8	
	fucomi	st(0),st(1)
	fcmovnbe	st(0),st(1)
	fstp	QWORD PTR [eax-8]
	dec	edx
	ffree	st(0)	
	jg	al_top                          ; jg allows him to work with "not 8-Byte aligned" memory
	jmp	startit
aligned:	
	mov	eax, ebx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 2				; len /= 4
	jz	SHORT unroll_end		; len==0 --> less than 4 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:
	fld	QWORD PTR [ecx+eax]		; This is an unrolled loop
	fld	QWORD PTR [eax]			; that compares 4 doubles.
	add	eax, 32	

	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

	fucomi	st(0),st(1)
	fcmovnbe	st(0),st(1)
	fstp	QWORD PTR [eax-32]
	ffree	st(0)

	fld	QWORD PTR [ecx+eax-24]
	fld	QWORD PTR [eax-24]
	fucomi	st(0),st(1)
	fcmovnbe	st(0),st(1)
	fstp	QWORD PTR [eax-24]
	ffree	st(0)

	fld	QWORD PTR [ecx+eax-16]
	fld	QWORD PTR [eax-16]
	fucomi	st(0),st(1)
	fcmovnbe	st(0),st(1)
	fstp	QWORD PTR [eax-16]
	ffree	st(0)

	fld	QWORD PTR [ecx+eax-8]
	fld	QWORD PTR [eax-8]
	fucomi	st(0),st(1)
	fcmovnbe	st(0),st(1)
	dec	edx
	fstp	QWORD PTR [eax-8]
	ffree	st(0)
	jne	SHORT ltop

unroll_end:	
	
	and	bl,3				; calculate remaining 
	jz	end

lrest:	
	fld	QWORD PTR [ecx+eax]		; compare the remaining numbers
	fld	QWORD PTR [eax]			; 
	add	eax, 8				
	fucomi	st(0),st(1)
	fcmovnbe	st(0),st(1)
	dec	bl
	fstp	QWORD PTR [eax-8]
	ffree	st(0)
	jnz	lrest
end:
	pop	ebx
	ret	
    }
}


/* ---------------------MPI_band----------------------------------*/
extern void band_error_func(void* d1,void*d2,unsigned int l);

static void band_int_sse(int *inbuf,int*inoutbuf,unsigned int len);
static void band_short_sse(short *inbuf,short *inoutbuf,unsigned int len);
static void band_byte_sse(signed char *inbuf,signed char *inoutbuf,unsigned int len);


static  const proto_func jmp_band_sse[] = {
    band_int_sse,   /*int*/
    band_error_func, /*float*/
    band_error_func,/*double*/
    band_error_func, /*complex*/
    band_int_sse,   /*long*/
    band_short_sse, /*short*/
    band_byte_sse,  /*char*/
    band_byte_sse, /*byte*/
    band_byte_sse, /*uchar*/
    band_short_sse,/*ushort*/
    band_int_sse,  /*ulong*/
    band_int_sse,  /*uint*/
    band_error_func, /*contig*/
    band_error_func, /*vector*/
    band_error_func, /*hvector*/
    band_error_func, /*indexed*/
    band_error_func, /*hindexed*/
    band_error_func, /*struct*/
    band_error_func, /*double_complex*/
    band_error_func, /*packed*/
    band_error_func, /*ub*/
    band_error_func, /*lb*/
    band_error_func, /*longdouble*/
    band_error_func, /*longlongint*/
    band_int_sse,   /*logical*/
    band_int_sse    /*fort_int*/
};

void MPIR_band_sse(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_band_sse[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}

__declspec(naked)
void band_int_sse(int *inbuf,int*inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_INT			; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,1				; divide by 2
	jz	remain
	
	
al_top:	
	movq	mm1, QWORD PTR [eax]		; handle integers
	movq	mm0, QWORD PTR [ecx+eax]	; 2 at a time
	add	eax,8
	pand	mm0, mm1			;
	movq	QWORD PTR [eax-8], mm0		;
	
	dec	edx
	jne	al_top
remain:
	test	bl,1
	jz	startit

	mov	edx,DWORD PTR [ecx+eax]		; handle remaining
	and	edx,DWORD PTR [eax]		; integer
	mov	DWORD PTR [eax],edx
	add	eax,4
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;
startit:

	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*2 integers.
	    
	add	eax, 32				; 
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
	
	pand	mm0, mm1			

	movq	mm2, QWORD PTR [ecx+eax-24]	
	movq	mm3, QWORD PTR [eax-24]		;
	movq	QWORD PTR [eax-32], mm0		;
	
	pand	mm2, mm3			;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	movq	QWORD PTR [eax-24], mm2
	
	pand	mm0, mm1			;
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	movq	QWORD PTR [eax-16], mm0		;

	pand	mm2, mm3			;
	
	dec	edx				;
	movq	QWORD PTR [eax-8], mm2		;
	jne	SHORT ltop			;
unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,7				; calculate remaining 
	shr	edx,1				; number of integers
	jz	norest				; 
lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; handle two ints
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	pand	mm0, mm1			;
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					;
	test	bl,1				;
	jz	end				; No--> we are finished
	mov	edx, DWORD PTR [ecx+eax]	; handle the remaining integer
	and	edx, DWORD PTR [eax]		;
	mov	[eax],edx			;
end:
	pop	ebx
	ret	
    }
}

__declspec(naked)
void band_short_sse(short *inbuf,short* inoutbuf,unsigned int len) {
__asm{

	push	ebx
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT			; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	remain
		
al_top:	
	movq	mm1, QWORD PTR [eax]		; Add shorts
	movq	mm0, QWORD PTR [ecx+eax]	; 4 at a time
	add	eax,8
	pand	mm0, mm1			;
	movq	QWORD PTR [eax-8], mm0		;
	
	dec	edx
	jne	al_top
remain:
	and	bl,3
	jz	startit
al_rest:
	mov	dx,WORD PTR [ecx+eax]		; Add remaining
	and	dx,WORD PTR [eax]		; shorts
	mov	WORD PTR [eax],dx
	add	eax,2
	dec	bl
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 4				; len /= 16
	jz	SHORT unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*4 shorts.
	    
	add	eax, 32				; We try to interleave some
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
	
	pand	mm0, mm1			;

	movq	mm2, QWORD PTR [ecx+eax-24]	
	movq	mm3, QWORD PTR [eax-24]		;
	movq	QWORD PTR [eax-32], mm0		;

	pand	mm2, mm3			;

	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	movq	QWORD PTR [eax-24], mm2

	pand	mm0, mm1			;	

	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	movq	QWORD PTR [eax-16], mm0		;

	pand	mm2, mm3			;

	dec	edx				;
	movq	QWORD PTR [eax-8], mm2		;
	
	jne	SHORT ltop			;

unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,15				; calculate remaining 
	shr	edx,2				; number of shorts
	jz	norest				; 

lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; add 4 shorts
	movq	mm1, QWORD PTR [eax]		;
	pand	mm0, mm1			;
	add	eax,8	
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					; restore FP state
	
	and	bl,3				; calculate remaining shorts
	jz	end				; 0 --> we are finished
lshort:
	mov	dx, WORD PTR [ecx+eax]		; Add the remaining integer
	and	dx, WORD PTR [eax]		;
	mov	WORD PTR [eax],dx		;
	add	eax,2
	dec	bl
	jnz	lshort

end:
	pop	ebx
	ret	
    }
}

__declspec(naked)
void band_byte_sse(char *inbuf,char *inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of bytes 
	sub	edx,eax				; that have to be handled	
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,3				; divide by 8
	jz	remain
		
al_top:	
	movq	mm1, QWORD PTR [eax]		; Add bytes
	movq	mm0, QWORD PTR [ecx+eax]	; 8 at a time
	add	eax,8
	pand	mm0, mm1			;
	movq	QWORD PTR [eax-8], mm0		;
	
	dec	edx
	jne	al_top
remain:
	and	bl,7
	jz	startit

al_rest:
	mov	dl,BYTE PTR [ecx+eax]		; Add remaining
	and	dl,BYTE PTR [eax]		; bytes
	mov	byte PTR [eax],dl
	inc	eax
	dec	bl
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 5				; len /= 32
	jz	SHORT unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*8 bytes.
	    
	add	eax, 32				
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
						
	pand	mm0, mm1			;
	
	movq	mm2, QWORD PTR [ecx+eax-24]	
	movq	mm3, QWORD PTR [eax-24]		;
	movq	QWORD PTR [eax-32], mm0		;
	
	pand	mm2, mm3			;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	movq	QWORD PTR [eax-24], mm2
	
	pand	mm0, mm1			;

	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	movq	QWORD PTR [eax-16], mm0		;
	
	pand	mm2, mm3			;

	dec	edx				;
	movq	QWORD PTR [eax-8], mm2		;
	
	jne	SHORT ltop			;

unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,31				; calculate remaining 
	shr	edx,3				; number of bytes
	jz	norest				; 

lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; add 8 bytes
	movq	mm1, QWORD PTR [eax]		;
	pand	mm0, mm1			;
	add	eax,8
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					; restore FP state
	
	and	bl,7				; calculate remaining bytes
	jz	end				; 0 --> we are finished
lbyte:
	mov	dl, BYTE PTR [ecx+eax]		; Add the remaining bytes
	and	dl, BYTE PTR [eax]		;
	mov	BYTE PTR [eax],dl		;
	inc	eax
	dec	bl
	jnz	lbyte

end:
	pop	ebx
	ret	
    }
}

/* ---------------------MPI_BOR----------------------------------*/
extern void bor_error_func(void* d1,void*d2,unsigned int l);

static void bor_int_sse(int *inbuf,int*inoutbuf,unsigned int len);
static void bor_short_sse(short *inbuf,short *inoutbuf,unsigned int len);
static void bor_byte_sse(signed char *inbuf,signed char *inoutbuf,unsigned int len);


static  const proto_func jmp_bor_sse[] = {
    bor_int_sse,   /*int*/
    bor_error_func, /*float*/
    bor_error_func,/*double*/
    bor_error_func, /*complex*/
    bor_int_sse,   /*long*/
    bor_short_sse, /*short*/
    bor_byte_sse,  /*char*/
    bor_byte_sse, /*byte*/
    bor_byte_sse, /*uchar*/
    bor_short_sse,/*ushort*/
    bor_int_sse,  /*ulong*/
    bor_int_sse,  /*uint*/
    bor_error_func, /*contig*/
    bor_error_func, /*vector*/
    bor_error_func, /*hvector*/
    bor_error_func, /*indexed*/
    bor_error_func, /*hindexed*/
    bor_error_func, /*struct*/
    bor_error_func, /*double_complex*/
    bor_error_func, /*packed*/
    bor_error_func, /*ub*/
    bor_error_func, /*lb*/
    bor_error_func, /*longdouble*/
    bor_error_func, /*longlongint*/
    bor_int_sse,   /*logical*/
    bor_int_sse    /*fort_int*/
};

void MPIR_bor_sse(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_bor_sse[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}

__declspec(naked)
void bor_int_sse(int *inbuf,int*inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_INT			; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,1				; divide by 2
	jz	remain
	
	
al_top:	
	movq	mm1, QWORD PTR [eax]		; handle integers
	movq	mm0, QWORD PTR [ecx+eax]	; 2 at a time
	por	mm0, mm1			;
	add	eax,8
	movq	QWORD PTR [eax-8], mm0		;
	
	dec	edx
	jne	al_top
remain:
	test	bl,1
	jz	startit

	mov	edx,DWORD PTR [ecx+eax]		; handle remaining
	or	edx,DWORD PTR [eax]		; integer
	mov	DWORD PTR [eax],edx
	add	eax,4
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;
startit:

	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*2 integers.
	    
	add	eax, 32			
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
	por	mm0, mm1			;
	
	movq	mm2, QWORD PTR [ecx+eax-24]
	movq	mm3, QWORD PTR [eax-24]		;
	movq	QWORD PTR [eax-32], mm0		;
	
	por	mm2, mm3			;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	movq	QWORD PTR [eax-24], mm2
	
	por	mm0, mm1			;
	    
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	movq	QWORD PTR [eax-16], mm0		;
	
	por	mm2, mm3			;
	dec	edx				;
	movq	QWORD PTR [eax-8], mm2		;
	
	jne	SHORT ltop			;
unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,7				; calculate remaining 
	shr	edx,1				; number of integers
	jz	norest				; 
lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; handle two ints
	movq	mm1, QWORD PTR [eax]		;
	por	mm0, mm1			;
	add	eax,8
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					;
	test	bl,1				;
	jz	end				; No--> we are finished
	mov	edx, DWORD PTR [ecx+eax]	; handle the remaining integer
	or	edx, DWORD PTR [eax]		;
	mov	DWORD PTR [eax],edx			;
end:
	pop	ebx
	ret	
    }
}

__declspec(naked)
void bor_short_sse(short *inbuf,short* inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT			; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	remain
		
al_top:	
	movq	mm1, QWORD PTR [eax]		; Add shorts
	movq	mm0, QWORD PTR [ecx+eax]	; 4 at a time
	add	eax,8
	por	mm0, mm1			;
	movq	QWORD PTR [eax-8], mm0		;
	
	dec	edx
	jne	al_top
remain:
	and	bl,3
	jz	startit
al_rest:
	mov	dx,WORD PTR [ecx+eax]		; Add remaining
	or	dx,WORD PTR [eax]		; shorts
	mov	WORD PTR [eax],dx
	add	eax,2
	dec	bl
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 4				; len /= 16
	jz	SHORT unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*4 shorts.
	    
	add	eax, 32		
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
	
	por	mm0, mm1			;

	movq	mm2, QWORD PTR [ecx+eax-24]
	movq	mm3, QWORD PTR [eax-24]		;
	movq	QWORD PTR [eax-32], mm0		;

	por	mm2, mm3			;

	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	movq	QWORD PTR [eax-24], mm2
	
	por	mm0, mm1			;

	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	movq	QWORD PTR [eax-16], mm0		;
	
	por	mm2, mm3			;
	dec	edx				;
	movq	QWORD PTR [eax-8], mm2		;
	
	jne	SHORT ltop			;

unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,15				; calculate remaining 
	shr	edx,2				; number of shorts
	jz	norest				; 

lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; add 4 shorts
	movq	mm1, QWORD PTR [eax]		;
	por	mm0, mm1			;
	add	eax,8
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					; restore FP state
	
	and	bl,3				; calculate remaining shorts
	jz	end				; 0 --> we are finished
lshort:
	mov	dx, WORD PTR [ecx+eax]		; Add the remaining integer
	or	dx, WORD PTR [eax]		;
	mov	WORD PTR [eax],dx		;
	add	eax,2
	dec	bl
	jnz	lshort

end:
	pop	ebx
	ret	
    }
}

__declspec(naked)
void bor_byte_sse(char *inbuf,char *inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of bytes 
	sub	edx,eax				; that have to be handled	
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,3				; divide by 8
	jz	remain
		
al_top:	
	movq	mm1, QWORD PTR [eax]		; handle bytes
	movq	mm0, QWORD PTR [ecx+eax]	; 8 at a time
	add	eax,8
	por	mm0, mm1			;
	movq	QWORD PTR [eax-8], mm0		;
	
	dec	edx
	jne	al_top
remain:
	and	bl,7
	jz	startit

al_rest:
	mov	dl,BYTE PTR [ecx+eax]		; Add remaining
	or	dl,BYTE PTR [eax]		; bytes
	mov	byte PTR [eax],dl
	inc	eax
	dec	bl
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 5				; len /= 32
	jz	SHORT unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*8 bytes.
	    
	add	eax, 32		
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	por	mm0, mm1			;
						
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	movq	QWORD PTR [eax-32], mm0		;
	
	por	mm2, mm3			;

	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	movq	QWORD PTR [eax-24], mm2
    
	por	mm0, mm1			;

	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	movq	QWORD PTR [eax-16], mm0		;

	por	mm2, mm3			;
	dec	edx				;
	movq	QWORD PTR [eax-8], mm2		;
	
	jne	SHORT ltop			;

unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,31				; calculate remaining 
	shr	edx,3				; number of bytes
	jz	norest				; 

lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; add 8 bytes
	movq	mm1, QWORD PTR [eax]		;
	por	mm0, mm1			;
	add	eax,8
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					; restore FP state
	
	and	bl,7				; calculate remaining bytes
	jz	end				; 0 --> we are finished
lbyte:
	mov	dl, BYTE PTR [ecx+eax]		; Add the remaining bytes
	or	dl, BYTE PTR [eax]		;
	mov	BYTE PTR [eax],dl		;
	inc	eax
	dec	bl
	jnz	lbyte

end:
	pop	ebx
	ret	
    }
}

/* ---------------------MPI_BXOR----------------------------------*/
extern void bxor_error_func(void* d1,void*d2,unsigned int l);

static void bxor_int_sse(int *inbuf,int*inoutbuf,unsigned int len);
static void bxor_short_sse(short *inbuf,short *inoutbuf,unsigned int len);
static void bxor_byte_sse(signed char *inbuf,signed char *inoutbuf,unsigned int len);


static  const proto_func jmp_bxor_sse[] = {
    bxor_int_sse,   /*int*/
    bxor_error_func, /*float*/
    bxor_error_func,/*double*/
    bxor_error_func, /*complex*/
    bxor_int_sse,   /*long*/
    bxor_short_sse, /*short*/
    bxor_byte_sse,  /*char*/
    bxor_byte_sse, /*byte*/
    bxor_byte_sse, /*uchar*/
    bxor_short_sse,/*ushort*/
    bxor_int_sse,  /*ulong*/
    bxor_int_sse,  /*uint*/
    bxor_error_func, /*contig*/
    bxor_error_func, /*vector*/
    bxor_error_func, /*hvector*/
    bxor_error_func, /*indexed*/
    bxor_error_func, /*hindexed*/
    bxor_error_func, /*struct*/
    bxor_error_func, /*double_complex*/
    bxor_error_func, /*packed*/
    bxor_error_func, /*ub*/
    bxor_error_func, /*lb*/
    bxor_error_func, /*longdouble*/
    bxor_error_func, /*longlongint*/
    bxor_int_sse,   /*logical*/
    bxor_int_sse    /*fort_int*/
};

void MPIR_bxor_sse(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_bxor_sse[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}

__declspec(naked)
void bxor_int_sse(int *inbuf,int*inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_INT			; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,1				; divide by 2
	jz	remain
	
	
al_top:	
	movq	mm1, QWORD PTR [eax]		; handle integers
	movq	mm0, QWORD PTR [ecx+eax]	; 2 at a time
	add	eax,8
	pxor	mm0, mm1			;
	movq	QWORD PTR [eax-8], mm0		;
	
	dec	edx
	jne	al_top
remain:
	test	bl,1
	jz	startit

	mov	edx,DWORD PTR [ecx+eax]		; handle remaining
	xor	edx,DWORD PTR [eax]		; integer
	mov	DWORD PTR [eax],edx
	add	eax,4
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;
startit:

	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*2 integers.
	    
	add	eax, 32				
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	pxor	mm0, mm1			;
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	pxor	mm2, mm3			;
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	pxor	mm0, mm1			;
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	pxor	mm2, mm3			;
	movq	QWORD PTR [eax-8], mm2		;
	
	jne	SHORT ltop			;
unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,7				; calculate remaining 
	shr	edx,1				; number of integers
	jz	norest				; 
lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; handle two ints
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	pxor	mm0, mm1			;
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					;
	test	bl,1				;
	jz	end				; No--> we are finished
	mov	edx, DWORD PTR [ecx+eax]	; handle the remaining integer
	xor	edx, DWORD PTR [eax]		;
	mov	[eax],edx			;
end:
	pop	ebx
	ret	
    }
}

__declspec(naked)
void bxor_short_sse(short *inbuf,short* inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT			; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	remain
		
al_top:	
	movq	mm1, QWORD PTR [eax]		; Add shorts
	movq	mm0, QWORD PTR [ecx+eax]	; 4 at a time
	add	eax,8
	pxor	mm0, mm1			;
	movq	QWORD PTR [eax-8], mm0		;
	
	dec	edx
	jne	al_top
remain:
	and	bl,3
	jz	startit
al_rest:
	mov	dx,WORD PTR [ecx+eax]		; Add remaining
	xor	dx,WORD PTR [eax]		; shorts
	mov	WORD PTR [eax],dx
	add	eax,2
	dec	bl
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 4				; len /= 16
	jz	SHORT unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*4 shorts.
	    
	add	eax, 32				
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	pxor	mm0, mm1			;
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	pxor	mm2, mm3			;
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	pxor	mm0, mm1			;
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	pxor	mm2, mm3			;
	movq	QWORD PTR [eax-8], mm2		;
	
	jne	SHORT ltop			;

unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,15				; calculate remaining 
	shr	edx,2				; number of shorts
	jz	norest				; 

lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; add 4 shorts
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	pxor	mm0, mm1			;
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					; restore FP state
	
	and	bl,3				; calculate remaining shorts
	jz	end				; 0 --> we are finished
lshort:
	mov	dx, WORD PTR [ecx+eax]		; Add the remaining integer
	xor	dx, WORD PTR [eax]		;
	mov	WORD PTR [eax],dx		;
	add	eax,2
	dec	bl
	jnz	lshort

end:
	pop	ebx
	ret	
    }
}

__declspec(naked)
void bxor_byte_sse(char *inbuf,char *inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of bytes 
	sub	edx,eax				; that have to be handled	
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,3				; divide by 8
	jz	remain
		
al_top:	
	movq	mm1, QWORD PTR [eax]		; handle bytes
	movq	mm0, QWORD PTR [ecx+eax]	; 8 at a time
	add	eax,8
	pxor	mm0, mm1			;
	movq	QWORD PTR [eax-8], mm0		;
	
	dec	edx
	jne	al_top
remain:
	and	bl,7
	jz	startit

al_rest:
	mov	dl,BYTE PTR [ecx+eax]		; Add remaining
	xor	dl,BYTE PTR [eax]		; bytes
	mov	byte PTR [eax],dl
	inc	eax
	dec	bl
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 5				; len /= 32
	jz	SHORT unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*8 bytes.
	    
	add	eax, 32				
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	pxor	mm0, mm1			;
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	pxor	mm2, mm3			;
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	pxor	mm0, mm1			;
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	pxor	mm2, mm3			;
	movq	QWORD PTR [eax-8], mm2		;
	
	jne	SHORT ltop			;

unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,31				; calculate remaining 
	shr	edx,3				; number of bytes
	jz	norest				; 

lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; add 8 bytes
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	pxor	mm0, mm1			;
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					; restore FP state
	
	and	bl,7				; calculate remaining bytes
	jz	end				; 0 --> we are finished
lbyte:
	mov	dl, BYTE PTR [ecx+eax]		; Add the remaining bytes
	xor	dl, BYTE PTR [eax]		;
	mov	BYTE PTR [eax],dl		;
	inc	eax
	dec	bl
	jnz	lbyte

end:
	pop	ebx
	ret	
    }
}

__declspec(align(8))
static const int oneInt[2] = {1,1};

__declspec(align(8))
static const short oneShort[4] = {1,1,1,1};

__declspec(align(8))
static const char oneByte[8] = {1,1,1,1,1,1,1,1};

/* ---------------------MPI_LXOR----------------------------------*/
extern void lxor_error_func(void* d1,void*d2,unsigned int l);

static void lxor_int_sse(int *inbuf,int*inoutbuf,unsigned int len);
static void lxor_short_sse(short *inbuf,short *inoutbuf,unsigned int len);
static void lxor_byte_sse(signed char *inbuf,signed char *inoutbuf,unsigned int len);


static  const proto_func jmp_lxor_sse[] = {
    lxor_int_sse,   /*int*/
    lxor_error_func, /*float*/
    lxor_error_func,/*double*/
    lxor_error_func, /*complex*/
    lxor_int_sse,   /*long*/
    lxor_short_sse, /*short*/
    lxor_byte_sse,  /*char*/
    lxor_byte_sse, /*byte*/
    lxor_byte_sse, /*uchar*/
    lxor_short_sse,/*ushort*/
    lxor_int_sse,  /*ulong*/
    lxor_int_sse,  /*uint*/
    lxor_error_func, /*contig*/
    lxor_error_func, /*vector*/
    lxor_error_func, /*hvector*/
    lxor_error_func, /*indexed*/
    lxor_error_func, /*hindexed*/
    lxor_error_func, /*struct*/
    lxor_error_func, /*double_complex*/
    lxor_error_func, /*packed*/
    lxor_error_func, /*ub*/
    lxor_error_func, /*lb*/
    lxor_error_func, /*longdouble*/
    lxor_error_func, /*longlongint*/
    lxor_int_sse,   /*logical*/
    lxor_int_sse    /*fort_int*/
};

void MPIR_lxor_sse(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_lxor_sse[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}

__declspec(naked)
void lxor_int_sse(int *inbuf,int*inoutbuf,unsigned int len) {
__asm{
	push	ebx
	
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_INT			; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value

match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]

	sub	ecx, eax			;
	shr	edx,1				; divide by 2
	jz	remain
	movq	mm6,[oneInt]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	
al_top:	
	movq	mm1, QWORD PTR [eax]		; handle integers
	movq	mm0, QWORD PTR [ecx+eax]	; 2 at a time
	add	eax,8
	pcmpeqd mm0,mm4				; mm0 == 0?
	pcmpeqd mm1,mm4				; mm1 == 0?
	pxor	mm0,mm1				; res1 ^ res2
	pand	mm0,mm6				; reset all bits except first
	movq	QWORD PTR [eax-8], mm0		;
	
	dec	edx
	jne	al_top

remain:
	test	bl,1
	jz	startit

	mov	edx,DWORD PTR [ecx+eax]		; handle remaining
	test	edx,edx				; integer
	je	aiszero
	cmp	DWORD PTR [eax],0
	jz	restrue
	xor	edx,edx
	jmp	result				; false
aiszero:
	cmp	DWORD PTR [eax],0
	je	nomove				; false, since edx contains 0
restrue:
	mov	edx,1
result:
	mov	DWORD PTR [eax],edx
nomove:	
	add	eax,4
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	movq	mm6,[oneInt]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:

	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*2 integers.
		
	add	eax, 32				
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	pcmpeqd mm0,mm4
	pcmpeqd mm1,mm4
	pxor	mm0,mm1
	pand	mm0,mm6;
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	pcmpeqd mm2,mm4
	pcmpeqd mm3,mm4
	pxor	mm2,mm3
	pand	mm2,mm6
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	pcmpeqd mm0,mm4
	pcmpeqd mm1,mm4
	pxor	mm0,mm1
	pand	mm0,mm6
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	pcmpeqd mm2,mm4
	pcmpeqd mm3,mm4
	pxor	mm2,mm3
	pand	mm2,mm6
	movq	QWORD PTR [eax-8], mm2		;	
	jne	SHORT ltop			;

unroll_end:	
	mov	dl, bl				; load original length
	and	dl,7				; calculate remaining 
	shr	dl,1				; number of integers
	jz	norest				; 
align 4
lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; handle two ints
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	pcmpeqd mm0,mm4
	pcmpeqd mm1,mm4
	pxor	mm0,mm1
	dec	dl
	pand	mm0,mm6
	movq	QWORD PTR [eax-8], mm0		;
	jne	lrest				;
norest:
	
	emms					;
	test	bl,1				;
	jz	end				; No--> we are finished
	
	mov	edx, DWORD PTR [ecx+eax]	; handle the remaining integer
	test	edx,edx				
	je	zero
	cmp	DWORD PTR [eax],0
	jz	restrue1
	xor	edx,edx				; edx = 0
	jmp	result1				; false
zero:
	cmp	DWORD PTR [eax],0
	je	end				; false, since inoutbuf already contains 0
restrue1:
	mov	edx,1
result1:
	mov	DWORD PTR [eax],edx
end:
	pop	ebx
	ret	
    }
}

__declspec(naked)
void lxor_short_sse(short *inbuf,short *inoutbuf,unsigned int len) {
__asm{
	push	ebx
	
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT			; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value

match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	remain
	movq	mm6,[oneShort]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	
al_top:	
	movq	mm1, QWORD PTR [eax]		; handle shorts
	movq	mm0, QWORD PTR [ecx+eax]	; 4 at a time
	add	eax,8
	pcmpeqw mm0,mm4				; mm0 == 0?
	pcmpeqw mm1,mm4				; mm1 == 0?
	pxor	mm0,mm1				; res1 ^ res2
	pand	mm0,mm6				; reset all bits except first
	movq	QWORD PTR [eax-8], mm0		;
	
	dec	edx
	jne	al_top

remain:
	and	bl,3
	jz	startit

al_rest:
	mov	dx,WORD PTR [ecx+eax]		; handle remaining
	test	dx,dx				; integer
	je	aiszero
	cmp	WORD PTR [eax],0
	jz	restrue
	xor	dx,dx
	jmp	result				; false
aiszero:
	cmp	WORD PTR [eax],0
	je	nomove				; false, since inoutbuf already contains 0
restrue:
	mov	dx,1
result:
	mov	WORD PTR [eax],dx
nomove:	
	add	eax,2
	dec	bl
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	movq	mm6,[oneShort]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 4				; len /= 16
	jz	SHORT unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:

	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that handles 4*4 shorts.
		
	add	eax, 32
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	pcmpeqw mm0,mm4
	pcmpeqw mm1,mm4
	pxor	mm0,mm1
	pand	mm0,mm6;
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	pcmpeqw mm2,mm4
	pcmpeqw mm3,mm4
	pxor	mm2,mm3
	pand	mm2,mm6
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	pcmpeqw mm0,mm4
	pcmpeqw mm1,mm4
	pxor	mm0,mm1
	pand	mm0,mm6
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	pcmpeqw mm2,mm4
	pcmpeqw mm3,mm4
	pxor	mm2,mm3
	pand	mm2,mm6
	movq	QWORD PTR [eax-8], mm2		;	
	jne	SHORT ltop			;

unroll_end:	
	mov	dl, bl				; load original length
	and	dl,15				; calculate remaining 
	shr	dl,2				; number of shorts
	jz	norest				; 
lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; handle 4 shorts
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	pcmpeqw mm0,mm4
	pcmpeqw mm1,mm4
	pxor	mm0,mm1
	dec	dl
	pand	mm0,mm6
	movq	QWORD PTR [eax-8], mm0		;
	jne	lrest				;
norest:
	
	emms					;
	and	bl,3				; Anything left?
	jz	end				; No--> we are finished
align 4
lrest1:
	mov	dx, WORD PTR [ecx+eax]		; handle the remaining shorts
	test	dx,dx				
	je	zero
	cmp	WORD PTR [eax],0
	jz	restrue1
	xor	dx,dx				; edx = 0
	jmp	result1				; false
zero:
	cmp	WORD PTR [eax],0
	je	nomove1				; false, since inoutbuf already contains 0
restrue1:
	mov	dx,1
result1:
	mov	WORD PTR [eax],dx
nomove1:
	add	eax,2
	dec	bl
	jnz	lrest1

end:
	pop	ebx
	ret	
    }
}

__declspec(naked)
void lxor_byte_sse(char *inbuf,char *inoutbuf,unsigned int len) {
__asm{
	push	ebx
	
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
						; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value

match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,3				; divide by 8
	jz	remain
	movq	mm6,[oneByte]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
align 4	
al_top:	
	movq	mm1, QWORD PTR [eax]		; handle bytes
	movq	mm0, QWORD PTR [ecx+eax]	; 8 at a time
	add	eax,8
	pcmpeqb mm0,mm4				; mm0 == 0?
	pcmpeqb mm1,mm4				; mm1 == 0?
	pxor	mm0,mm1				; res1 ^ res2
	pand	mm0,mm6				; reset all bits except first
	movq	QWORD PTR [eax-8], mm0		;
	
	dec	edx
	jne	al_top

remain:
	and	bl,7
	jz	startit

al_rest:
	mov	dl,BYTE PTR [ecx+eax]		; handle remaining
	test	dl,dl				; integer
	je	aiszero
	cmp	BYTE PTR [eax],0
	jz	restrue
	xor	dl,dl
	jmp	result				; false
aiszero:
	cmp	BYTE PTR [eax],0
	je	nomove				; false, since inoutbuf already 0
restrue:
	mov	dl,1
result:
	mov	BYTE PTR [eax],dl
nomove:	
	inc	eax
	dec	bl
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	movq	mm6,[oneByte]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 5				; len /= 32
	jz	SHORT unroll_end		; len==0 --> less than 32 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:

	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that handles 4*8 bytes.
		
	add	eax, 32
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	pcmpeqb mm0,mm4
	pcmpeqb mm1,mm4
	pxor	mm0,mm1
	pand	mm0,mm6;
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	pcmpeqb mm2,mm4
	pcmpeqb mm3,mm4
	pxor	mm2,mm3
	pand	mm2,mm6
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	pcmpeqb mm0,mm4
	pcmpeqb mm1,mm4
	pxor	mm0,mm1
	pand	mm0,mm6
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	pcmpeqb mm2,mm4
	pcmpeqb mm3,mm4
	pxor	mm2,mm3
	pand	mm2,mm6
	movq	QWORD PTR [eax-8], mm2		;	
	jne	SHORT ltop			;

unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,31				; calculate remaining 
	shr	edx,3				; number of shorts
	jz	norest				; 
lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; handle 8 bytes
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	pcmpeqb mm0,mm4
	pcmpeqb mm1,mm4
	pxor	mm0,mm1
	pand	mm0,mm6
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					;
	and	bl,7				; Anything left?
	jz	end				; No--> we are finished
align 4
lrest1:
	mov	dl, BYTE PTR [ecx+eax]		; handle the remaining bytes
	test	dl,dl				
	je	zero
	cmp	BYTE PTR [eax],0
	jz	restrue1
	xor	dl,dl				; edx = 0
	jmp	result1				; false
zero:
	cmp	BYTE PTR [eax],0
	je	nomove1				; false, since inoutbuf already contains 0
restrue1:
	mov	dl,1
result1:	
	mov	BYTE PTR [eax],dl
nomove1:
	inc	eax
	dec	bl
	jnz	lrest1

end:
	pop	ebx
	ret	
    }
}

/* ---------------------MPI_LAND----------------------------------*/
extern void land_error_func(void* d1,void*d2,unsigned int l);

static void land_int_sse(int *inbuf,int*inoutbuf,unsigned int len);
static void land_short_sse(short *inbuf,short *inoutbuf,unsigned int len);
static void land_byte_sse(signed char *inbuf,signed char *inoutbuf,unsigned int len);


static  const proto_func jmp_land_sse[] = {
    land_int_sse,   /*int*/
    land_error_func, /*float*/
    land_error_func,/*double*/
    land_error_func, /*complex*/
    land_int_sse,   /*long*/
    land_short_sse, /*short*/
    land_byte_sse,  /*char*/
    land_byte_sse, /*byte*/
    land_byte_sse, /*uchar*/
    land_short_sse,/*ushort*/
    land_int_sse,  /*ulong*/
    land_int_sse,  /*uint*/
    land_error_func, /*contig*/
    land_error_func, /*vector*/
    land_error_func, /*hvector*/
    land_error_func, /*indexed*/
    land_error_func, /*hindexed*/
    land_error_func, /*struct*/
    land_error_func, /*double_complex*/
    land_error_func, /*packed*/
    land_error_func, /*ub*/
    land_error_func, /*lb*/
    land_error_func, /*longdouble*/
    land_error_func, /*longlongint*/
    land_int_sse,   /*logical*/
    land_int_sse    /*fort_int*/
};

void MPIR_land_sse(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_land_sse[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}

__declspec(naked)
void land_int_sse(int *inbuf,int*inoutbuf,unsigned int len) {
__asm{
	push	ebx
	
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_INT			; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value

match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]

	sub	ecx, eax			;
	shr	edx,1				; divide by 2
	jz	remain
	movq	mm6,[oneInt]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	
al_top:	
	movq	mm1, QWORD PTR [eax]		; handle integers
	movq	mm0, QWORD PTR [ecx+eax]	; 2 at a time
	add	eax,8
	pcmpeqd mm0,mm4				; mm0 == 0?
	pcmpeqd mm1,mm4				; mm1 == 0?
	pandn	mm1,mm6
	pandn	mm0,mm1				; res1 ^ res2
	dec	edx
	
	movq	QWORD PTR [eax-8], mm0		;	
	jne	al_top

remain:
	test	bl,1
	jz	startit

	mov	edx,DWORD PTR [ecx+eax]		; handle remaining
	test	edx,edx				; integer
	je	result				; false, since edx contains zero
	cmp	DWORD PTR [eax],0
	je	nomove				; false
	mov	edx,1
result:
	mov	DWORD PTR [eax],edx	
nomove:
	add	eax,4
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	movq	mm6,[oneInt]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

align 4
ltop:

	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*2 integers.
		
	add	eax, 32		
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	pcmpeqd mm0,mm4
	pcmpeqd mm1,mm4
	pandn	mm1,mm6	
	pandn	mm0,mm1
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	pcmpeqd mm2,mm4
	pcmpeqd mm3,mm4
	pandn	mm3,mm6
	pandn	mm2,mm3
	
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	pcmpeqd mm0,mm4
	pcmpeqd mm1,mm4
	pandn	mm1,mm6
	pandn	mm0,mm1
	
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	pcmpeqd mm2,mm4
	pcmpeqd mm3,mm4
	pandn	mm3,mm6
	pandn	mm2,mm3
	movq	QWORD PTR [eax-8], mm2		;	
	jne	SHORT ltop			;

unroll_end:	
	mov	dl, bl				; load original length
	and	dl,7				; calculate remaining 
	shr	dl,1				; number of integers
	jz	norest				; 
align 4
lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; handle two ints
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	pcmpeqd mm0,mm4
	pcmpeqd mm1,mm4
	pandn	mm1,mm6	
	pandn	mm0,mm1
	dec	dl
	
	movq	QWORD PTR [eax-8], mm0		;
	jne	lrest				;
norest:
	
	emms					;
	test	bl,1				;
	jz	end				; No--> we are finished
	
	mov	edx, DWORD PTR [ecx+eax]	; handle the remaining integer
	test	edx,edx				
	je	result1				; false
	cmp	DWORD PTR [eax],0
	je	end				; false
	mov	edx,1
result1:
	mov	DWORD PTR [eax],edx
end:
	pop	ebx
	ret	
    }
}

__declspec(naked)
void land_short_sse(short *inbuf,short *inoutbuf,unsigned int len) {
__asm{
	push	ebx
	
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT			; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value

match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	remain
	movq	mm6,[oneShort]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	
al_top:	
	movq	mm1, QWORD PTR [eax]		; handle shorts
	movq	mm0, QWORD PTR [ecx+eax]	; 4 at a time
	add	eax,8
	pcmpeqw mm0,mm4				; mm0 == 0?
	pcmpeqw mm1,mm4				; mm1 == 0?
	pandn	mm1,mm6
	dec	edx
	pandn	mm0,mm1				; 
	movq	QWORD PTR [eax-8], mm0		;
	
	jne	al_top

remain:
	and	bl,3
	jz	startit

al_rest:
	mov	dx,WORD PTR [ecx+eax]		; handle remaining
	test	dx,dx				; integer
	je	result				; false
	cmp	WORD PTR [eax],0
	je	nomove				; false
	mov	dx,1				; true
result:
	mov	WORD PTR [eax],dx	
nomove:
	add	eax,2
	dec	bl
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	movq	mm6,[oneShort]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 4				; len /= 16
	jz	SHORT unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:

	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that handles 4*4 shorts.
		
	add	eax, 32				
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	pcmpeqw mm0,mm4
	pcmpeqw mm1,mm4
	pandn	mm1,mm6
	pandn	mm0,mm1
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	pcmpeqw mm2,mm4
	pcmpeqw mm3,mm4
	pandn	mm3,mm6
	pandn	mm2,mm3
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	pcmpeqw mm0,mm4
	pcmpeqw mm1,mm4
	pandn	mm1,mm6
	pandn	mm0,mm1
	
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	pcmpeqw mm3,mm4
	pcmpeqw mm2,mm4
	pandn	mm3,mm6
	pandn	mm2,mm3
	movq	QWORD PTR [eax-8], mm2		;	
	jne	SHORT ltop			;

unroll_end:	
	mov	dl, bl				; load original length
	and	dl,15				; calculate remaining 
	shr	dl,2				; number of shorts
	jz	norest				; 
lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; handle 4 shorts
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	pcmpeqw mm0,mm4
	pcmpeqw mm1,mm4
	pandn	mm1,mm6

	dec	dl
	pandn	mm0,mm1	
	movq	QWORD PTR [eax-8], mm0		;
	jne	lrest				;
norest:
	
	emms					;
	and	bl,3				; Anything left?
	jz	end				; No--> we are finished
align 4
lrest1:
	mov	dx, WORD PTR [ecx+eax]		; handle the remaining shorts
	test	dx,dx				
	je	result1				; false
	cmp	WORD PTR [eax],0
	je	nomove1
	mov	dx,1
result1:
	mov	WORD PTR [eax],dx
nomove1:
	add	eax,2
	dec	bl
	jnz	lrest1

end:
	pop	ebx
	ret	
    }
}

__declspec(naked)
void land_byte_sse(char *inbuf,char *inoutbuf,unsigned int len) {
__asm{
	push	ebx
	
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
						; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value

match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,3				; divide by 8
	jz	remain
	movq	mm6, QWORD PTR [oneByte]	; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
align 4	
al_top:	
	movq	mm1, QWORD PTR [eax]		; handle bytes
	movq	mm0, QWORD PTR [ecx+eax]	; 8 at a time
	add	eax,8
	pcmpeqb mm1,mm4				; mm1 == 0?
	pcmpeqb mm0,mm4				; mm0 == 0?	
			
	pandn	mm1,mm6				; invert mm1
	pandn	mm0,mm1				; 
	dec	edx
	movq	QWORD PTR [eax-8], mm0		;	

	jne	al_top

remain:
	and	bl,7
	jz	startit

al_rest:
	mov	dl,BYTE PTR [ecx+eax]		; handle remaining
	test	dl,dl				; integer
	je	result				; false
	cmp	BYTE PTR [eax],0
	je	nomove				; false
	mov	dl,1
result:
	mov	BYTE PTR [eax],dl
nomove:	
	inc	eax
	dec	bl
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	movq	mm6, QWORD PTR [oneByte]	; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 5				; len /= 32
	jz	SHORT unroll_end		; len==0 --> less than 32 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
ltop:

	movq	mm1, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm0, QWORD PTR [eax]		; that handles 4*8 bytes.
		
	add	eax, 32
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	pcmpeqb mm1,mm4
	pcmpeqb mm0,mm4
	pandn	mm1,mm6
	pandn	mm0,mm1
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	pcmpeqb mm3,mm4
	pcmpeqb mm2,mm4
	pandn	mm3,mm6
	pandn	mm2,mm3
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	pcmpeqb mm1,mm4
	pcmpeqb mm0,mm4
	pandn	mm1,mm6
	pandn	mm0,mm1	
	movq	QWORD PTR [eax-16], mm0		;
	
	pcmpeqb mm2,mm4
	pcmpeqb mm3,mm4
	dec	edx				;
	pandn	mm3,mm6
	pandn	mm2,mm3
	movq	QWORD PTR [eax-8], mm2		;	
	jne	SHORT ltop			;

unroll_end:	
	mov	dl, bl				; load original length
	and	dl,31				; calculate remaining 
	shr	dl,3				; number of shorts
	jz	norest				; 
lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; handle 8 bytes
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	pcmpeqb mm0,mm4	
	pcmpeqb mm1,mm4
	dec	dl				;
	pandn	mm1,mm6
	pandn	mm0,mm1
	movq	QWORD PTR [eax-8], mm0		;

	jne	lrest				;
norest:
	
	emms					;
	and	bl,7				; Anything left?
	jz	end				; No--> we are finished
align 4
lrest1:
	mov	dl, BYTE PTR [ecx+eax]		; handle the remaining bytes
	test	dl,dl				
	je	result1				; false
	cmp	BYTE PTR [eax],0
	je	nomove1				; false
	mov	dl,1
result1:
	mov	BYTE PTR [eax],dl
nomove1:	
	inc	eax
	dec	bl
	jnz	lrest1

end:
	pop	ebx
	ret	
    }
}

/* ---------------------MPI_LOR----------------------------------*/
extern void lor_error_func(void* d1,void*d2,unsigned int l);

static void lor_int_sse(int *inbuf,int*inoutbuf,unsigned int len);
static void lor_short_sse(short *inbuf,short *inoutbuf,unsigned int len);
static void lor_byte_sse(signed char *inbuf,signed char *inoutbuf,unsigned int len);


static  const proto_func jmp_lor_sse[] = {
    lor_int_sse,   /*int*/
    lor_error_func, /*float*/
    lor_error_func,/*double*/
    lor_error_func, /*complex*/
    lor_int_sse,   /*long*/
    lor_short_sse, /*short*/
    lor_byte_sse,  /*char*/
    lor_byte_sse, /*byte*/
    lor_byte_sse, /*uchar*/
    lor_short_sse,/*ushort*/
    lor_int_sse,  /*ulong*/
    lor_int_sse,  /*uint*/
    lor_error_func, /*contig*/
    lor_error_func, /*vector*/
    lor_error_func, /*hvector*/
    lor_error_func, /*indexed*/
    lor_error_func, /*hindexed*/
    lor_error_func, /*struct*/
    lor_error_func, /*double_complex*/
    lor_error_func, /*packed*/
    lor_error_func, /*ub*/
    lor_error_func, /*lb*/
    lor_error_func, /*longdouble*/
    lor_error_func, /*longlongint*/
    lor_int_sse,   /*logical*/
    lor_int_sse    /*fort_int*/
};

void MPIR_lor_sse(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_lor_sse[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}

__declspec(naked)
void lor_int_sse(int *inbuf,int*inoutbuf,unsigned int len) {
__asm{
	push	ebx
	
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_INT			; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value

match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]

	sub	ecx, eax			;
	shr	edx,1				; divide by 2
	jz	remain
	movq	mm6,[oneInt]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	
al_top:	
	movq	mm1, QWORD PTR [eax]		; handle integers
	movq	mm0, QWORD PTR [ecx+eax]	; 2 at a time
	add	eax,8
	pcmpeqd mm0,mm4				; mm0 == 0?
	pcmpeqd mm1,mm4				; mm1 == 0?
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1				; res1 || res2
	dec	edx
	movq	QWORD PTR [eax-8], mm0		;	
	jne	al_top

remain:
	test	bl,1
	jz	startit

	mov	edx,DWORD PTR [ecx+eax]		; handle remaining
	test	edx,edx				; integer
	jne	restrue				; true
	cmp	DWORD PTR [eax],0
	je	nomove				; false
restrue:
	mov	DWORD PTR [eax],1	
nomove:
	add	eax,4
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	movq	mm6,[oneInt]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		
align 4
ltop:

	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*2 integers.
		
	add	eax, 32				
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	pcmpeqd mm0,mm4
	pcmpeqd mm1,mm4
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	pcmpeqd mm2,mm4
	pcmpeqd mm3,mm4
	pandn	mm2,mm6				; invert mm2
	pandn	mm3,mm6				; and mm3
	por	mm2,mm3
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	pcmpeqd mm0,mm4
	pcmpeqd mm1,mm4
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	pcmpeqd mm2,mm4
	pcmpeqd mm3,mm4
	pandn	mm2,mm6
	pandn	mm3,mm6
	por	mm2,mm3
	movq	QWORD PTR [eax-8], mm2		;	
	jne	SHORT ltop			;

unroll_end:	
	mov	dl, bl				; load original length
	and	dl,7				; calculate remaining 
	shr	dl,1				; number of integers
	jz	norest				; 
align 4
lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; handle two ints
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	pcmpeqd mm0,mm4
	pcmpeqd mm1,mm4
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1
	dec	dl
	movq	QWORD PTR [eax-8], mm0		;
	jne	lrest				;
norest:
	
	emms					;
	test	bl,1				;
	jz	end				; No--> we are finished
	
	mov	edx, DWORD PTR [ecx+eax]	; handle the remaining integer
	test	edx,edx				
	jne	restrue1
	cmp	DWORD PTR [eax],0
	je	end				; false
restrue1:
	mov	DWORD PTR [eax],1
end:
	pop	ebx
	ret	
    }
}

__declspec(naked)
void lor_short_sse(short *inbuf,short *inoutbuf,unsigned int len) {
__asm{
	push	ebx
	
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT			; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value

match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]

	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	remain
	movq	mm6,[oneShort]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	
al_top:	
	movq	mm1, QWORD PTR [eax]		; handle shorts
	movq	mm0, QWORD PTR [ecx+eax]	; 4 at a time
	add	eax,8
	pcmpeqw mm0,mm4				; mm0 == 0?
	pcmpeqw mm1,mm4				; mm1 == 0?
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1				; res1 || res2
	dec	edx
	movq	QWORD PTR [eax-8], mm0		;
	
	jne	al_top

remain:
	and	bl,3
	jz	startit

al_rest:
	mov	dx,WORD PTR [ecx+eax]		; handle remaining
	test	dx,dx				; shorts
	jne	restrue
	cmp	WORD PTR [eax],0
	je	nomove				; false
restrue:
	mov	WORD PTR [eax],1	
nomove:
	add	eax,2
	dec	bl
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	movq	mm6,[oneShort]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 4				; len /= 16
	jz	SHORT unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		
align 4
ltop:

	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that handles 4*4 shorts.
		
	add	eax, 32				
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	pcmpeqw mm0,mm4
	pcmpeqw mm1,mm4
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	pcmpeqw mm2,mm4
	pcmpeqw mm3,mm4
	pandn	mm2,mm6				; invert mm2
	pandn	mm3,mm6				; and mm3
	por	mm2,mm3
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	pcmpeqw mm0,mm4
	pcmpeqw mm1,mm4
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	pcmpeqw mm2,mm4
	pcmpeqw mm3,mm4
	pandn	mm2,mm6				; invert mm2
	pandn	mm3,mm6				; and mm3
	por	mm2,mm3
	
	movq	QWORD PTR [eax-8], mm2		;	
	jne	SHORT ltop			;

unroll_end:	
	mov	dl, bl				; load original length
	and	dl,15				; calculate remaining 
	shr	dl,2				; number of shorts
	jz	norest				; 
lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; handle 4 shorts
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	pcmpeqw mm0,mm4
	pcmpeqw mm1,mm4
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1
	dec	dl
	movq	QWORD PTR [eax-8], mm0		;
	jne	lrest				;
norest:
	
	emms					;
	and	bl,3				; Anything left?
	jz	end				; No--> we are finished
align 4
lrest1:
	mov	dx, WORD PTR [ecx+eax]		; handle the remaining shorts
	test	dx,dx				
	jne	result1				; false
	cmp	WORD PTR [eax],0
	je	nomove1
result1:
	mov	WORD PTR [eax],1
nomove1:
	add	eax,2
	dec	bl
	jnz	lrest1

end:
	pop	ebx
	ret	
    }
}

__declspec(naked)
void lor_byte_sse(char *inbuf,char *inoutbuf,unsigned int len) {
__asm{
	push	ebx
	
	mov	eax, DWORD PTR [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	aligned				; 0 --> Already aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
						; to get aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value

match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	prefetchnta [ecx+32]

	sub	ecx, eax			;
	shr	edx,3				; divide by 8
	jz	remain
	movq	mm6,[oneByte]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
align 4	
al_top:	
	movq	mm1, QWORD PTR [eax]		; handle bytes
	movq	mm0, QWORD PTR [ecx+eax]	; 8 at a time
	add	eax,8
	pcmpeqb mm0,mm4				; mm0 == 0?
	pcmpeqb mm1,mm4				; mm1 == 0?
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1				; res1 ^ res2
	dec	edx
	movq	QWORD PTR [eax-8], mm0		;
	
	jne	al_top

remain:
	and	bl,7
	jz	startit

al_rest:
	mov	dl,BYTE PTR [ecx+eax]		; handle remaining
	test	dl,dl				; integer
	jne	restrue
	cmp	BYTE PTR [eax],0
	je	nomove				; false	
restrue:
	mov	BYTE PTR [eax],1
nomove:	
	inc	eax
	dec	bl
	jnz	al_rest
	jmp	startit

aligned:	
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;

startit:
	movq	mm6,[oneByte]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 5				; len /= 32
	jz	SHORT unroll_end		; len==0 --> less than 32 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		
align 4
ltop:

	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that handles 4*8 bytes.
		
	add	eax, 32				
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	pcmpeqb mm0,mm4
	pcmpeqb mm1,mm4
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	pcmpeqb mm2,mm4
	pcmpeqb mm3,mm4
	pandn	mm2,mm6				; invert mm2
	pandn	mm3,mm6				; and mm3
	por	mm2,mm3
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	pcmpeqb mm0,mm4
	pcmpeqb mm1,mm4
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	pcmpeqb mm2,mm4
	pcmpeqb mm3,mm4
	pandn	mm2,mm6				; invert mm2
	pandn	mm3,mm6				; and mm3
	por	mm2,mm3
	movq	QWORD PTR [eax-8], mm2		;	
	jne	SHORT ltop			;

unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,31				; calculate remaining 
	shr	edx,3				; number of shorts
	jz	norest				; 
lrest:
	movq	mm0, QWORD PTR [ecx+eax]	; handle 8 bytes
	movq	mm1, QWORD PTR [eax]		;
	add	eax,8
	pcmpeqb mm0,mm4
	pcmpeqb mm1,mm4
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					;
	and	bl,7				; Anything left?
	jz	end				; No--> we are finished
align 4
lrest1:
	mov	dl, BYTE PTR [ecx+eax]		; handle the remaining bytes
	test	dl,dl				
	jne	result1				; true
	cmp	BYTE PTR [eax],0
	je	nomove1				; false
result1:
	mov	BYTE PTR [eax],1
nomove1:	
	inc	eax
	dec	bl
	jnz	lrest1

end:
	pop	ebx
	ret	
    }
}

#elif defined(_M_AMD64) 

#include <mmintrin.h>
#include <emmintrin.h>
#include "mpiimpl.h"
#include "x86_ops.h"
#include "mpiops.h"

/* ---------------------MPI_SUM----------------------------------*/

static void add_int_sse(int *inbuf, int* inoutbuf, unsigned int len);
static void add_short_sse(short *inbuf, short *inoutbuf, unsigned int len);
static void add_byte_sse(char *inbuf, char *inoutbuf, unsigned int len);
static void add_float_sse(float *inbuf, float* inoutbuf, unsigned int len);
static void add_double_sse(double *inbuf, double* inoutbuf, unsigned int len);
static void add_d_complex_sse(d_complex *inbuf, d_complex *inoutbuf, unsigned int len);
extern void add_error_func(void* d1, void*d2, unsigned int l) ;

static void add_byte_sse(char* inbuf, char* inoutbuf, unsigned int len) 
{
	__m128i			xmm[8];
    unsigned int	i, u;

    if(!len) 
		return;

	_mm_prefetch((char*)(inbuf), _MM_HINT_NTA);
	_mm_prefetch((char*)(inbuf+64), _MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf), _MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+64), _MM_HINT_NTA);
    
	u = 128 / (2*sizeof(char));
	for(i=0; (i+u)<len; i+=u) 
	{
		_mm_prefetch((char*)(inbuf+(i+128)), _MM_HINT_NTA);
		_mm_prefetch((char*)(inbuf+(i+192)), _MM_HINT_NTA);
		_mm_prefetch((char*)(inoutbuf+(i+128)), _MM_HINT_NTA);
		_mm_prefetch((char*)(inoutbuf+(i+192)), _MM_HINT_NTA);

		/* load 128 Byte into xmm register */
		xmm[0] = _mm_load_si128((__m128i*) (inbuf+(i+0)));
		xmm[1] = _mm_load_si128((__m128i*) (inbuf+(i+16)));
		xmm[2] = _mm_load_si128((__m128i*) (inbuf+(i+32)));
		xmm[3] = _mm_load_si128((__m128i*) (inbuf+(i+48)));
		xmm[4] = _mm_load_si128((__m128i*) (inoutbuf+(i+0)));
		xmm[5] = _mm_load_si128((__m128i*) (inoutbuf+(i+16)));
		xmm[6] = _mm_load_si128((__m128i*) (inoutbuf+(i+32)));
		xmm[7] = _mm_load_si128((__m128i*) (inoutbuf+(i+48)));

		/* Adds the byte values */
		xmm[0] = _mm_add_epi8(xmm[0], xmm[4]);
		xmm[1] = _mm_add_epi8(xmm[1], xmm[5]);
		xmm[2] = _mm_add_epi8(xmm[2], xmm[6]);
		xmm[3] = _mm_add_epi8(xmm[3], xmm[7]);

		/* stores the results */
		_mm_stream_si128((__m128i*) (inoutbuf+(i+0)), xmm[0]);
		_mm_stream_si128((__m128i*) (inoutbuf+(i+16)), xmm[1]);
		_mm_stream_si128((__m128i*) (inoutbuf+(i+32)), xmm[2]);
		_mm_stream_si128((__m128i*) (inoutbuf+(i+48)), xmm[3]);
    }

    for(;i<len;i++)
		inoutbuf[i]+=inbuf[i];
}

static void add_short_sse(short* inbuf, short* inoutbuf, unsigned int len) 
{
	__m128i			xmm[8];
    unsigned int	i, u;

    if(!len) 
		return;

	_mm_prefetch((char*)(inbuf), _MM_HINT_NTA);
	_mm_prefetch((char*)(inbuf+32), _MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf), _MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+32),_MM_HINT_NTA);
    
	u = 128 / (2*sizeof(short));
	for(i=0; (i+u)<len; i+=u) 
	{
		_mm_prefetch((char*)(inbuf+(i+64)), _MM_HINT_NTA);
		_mm_prefetch((char*)(inbuf+(i+96)), _MM_HINT_NTA);
		_mm_prefetch((char*)(inoutbuf+(i+64)), _MM_HINT_NTA);
		_mm_prefetch((char*)(inoutbuf+(i+96)), _MM_HINT_NTA);

		/* load 128 Byte into xmm register */
		xmm[0] = _mm_load_si128((__m128i*) (inbuf+(i+0)));
		xmm[1] = _mm_load_si128((__m128i*) (inbuf+(i+8)));
		xmm[2] = _mm_load_si128((__m128i*) (inbuf+(i+16)));
		xmm[3] = _mm_load_si128((__m128i*) (inbuf+(i+32)));
		xmm[4] = _mm_load_si128((__m128i*) (inoutbuf+(i+0)));
		xmm[5] = _mm_load_si128((__m128i*) (inoutbuf+(i+8)));
		xmm[6] = _mm_load_si128((__m128i*) (inoutbuf+(i+16)));
		xmm[7] = _mm_load_si128((__m128i*) (inoutbuf+(i+32)));

		/* Adds the short values */
		xmm[0] = _mm_add_epi16(xmm[0], xmm[4]);
		xmm[1] = _mm_add_epi16(xmm[1], xmm[5]);
		xmm[2] = _mm_add_epi16(xmm[2], xmm[6]);
		xmm[3] = _mm_add_epi16(xmm[3], xmm[7]);

		/* stores the results */
		_mm_stream_si128((__m128i*) (inoutbuf+(i+0)), xmm[0]);
		_mm_stream_si128((__m128i*) (inoutbuf+(i+8)), xmm[1]);
		_mm_stream_si128((__m128i*) (inoutbuf+(i+16)), xmm[2]);
		_mm_stream_si128((__m128i*) (inoutbuf+(i+32)), xmm[3]);
    }

    for(;i<len;i++)
		inoutbuf[i]+=inbuf[i];
}

static void add_int_sse(int* inbuf, int* inoutbuf, unsigned int len) 
{
	__m128i			xmm[8];
    unsigned int	i, u;

    if(!len) 
		return;

	_mm_prefetch((char*)(inbuf), _MM_HINT_NTA);
	_mm_prefetch((char*)(inbuf+16), _MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf), _MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+16), _MM_HINT_NTA);
    
	u = 128 / (2*sizeof(int));
	for(i=0; (i+u)<len; i+=u) 
	{
		_mm_prefetch((char*)(inbuf+(i+32)), _MM_HINT_NTA);
		_mm_prefetch((char*)(inbuf+(i+48)), _MM_HINT_NTA);
		_mm_prefetch((char*)(inoutbuf+(i+32)), _MM_HINT_NTA);
		_mm_prefetch((char*)(inoutbuf+(i+48)), _MM_HINT_NTA);

		/* load 128 Byte into xmm register */
		xmm[0] = _mm_load_si128((__m128i*) (inbuf+(i+0)));
		xmm[1] = _mm_load_si128((__m128i*) (inbuf+(i+4)));
		xmm[2] = _mm_load_si128((__m128i*) (inbuf+(i+8)));
		xmm[3] = _mm_load_si128((__m128i*) (inbuf+(i+12)));
		xmm[4] = _mm_load_si128((__m128i*) (inoutbuf+(i+0)));
		xmm[5] = _mm_load_si128((__m128i*) (inoutbuf+(i+4)));
		xmm[6] = _mm_load_si128((__m128i*) (inoutbuf+(i+8)));
		xmm[7] = _mm_load_si128((__m128i*) (inoutbuf+(i+12)));

		/* Adds the int values */
		xmm[0] = _mm_add_epi32(xmm[0], xmm[4]);
		xmm[1] = _mm_add_epi32(xmm[1], xmm[5]);
		xmm[2] = _mm_add_epi32(xmm[2], xmm[6]);
		xmm[3] = _mm_add_epi32(xmm[3], xmm[7]);

		/* stores the results */
		_mm_stream_si128((__m128i*) (inoutbuf+(i+0)), xmm[0]);
		_mm_stream_si128((__m128i*) (inoutbuf+(i+4)), xmm[1]);
		_mm_stream_si128((__m128i*) (inoutbuf+(i+8)), xmm[2]);
		_mm_stream_si128((__m128i*) (inoutbuf+(i+12)), xmm[3]);
    }

    for(;i<len;i++)
		inoutbuf[i]+=inbuf[i];
}

static void add_double_sse(double *inbuf, double* inoutbuf, unsigned int len) 
{
	__m128d			xmm[8];
    unsigned int	i, u;

    if(!len) 
		return;

	_mm_prefetch((char*)(inbuf), _MM_HINT_NTA);
	_mm_prefetch((char*)(inbuf+8), _MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf), _MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+8), _MM_HINT_NTA);
    
	u = 128 / (2*sizeof(double));
	for(i=0; (i+u)<len; i+=u) 
	{
		_mm_prefetch((char*)(inbuf+(i+16)), _MM_HINT_NTA);
		_mm_prefetch((char*)(inbuf+(i+24)), _MM_HINT_NTA);
		_mm_prefetch((char*)(inoutbuf+(i+16)), _MM_HINT_NTA);
		_mm_prefetch((char*)(inoutbuf+(i+24)), _MM_HINT_NTA);

		/* load 128 Byte into xmm register */
		xmm[0] = _mm_load_pd(inbuf+(i+0));
		xmm[1] = _mm_load_pd(inbuf+(i+2));
		xmm[2] = _mm_load_pd(inbuf+(i+4));
		xmm[3] = _mm_load_pd(inbuf+(i+6));
		xmm[4] = _mm_load_pd(inoutbuf+(i+0));
		xmm[5] = _mm_load_pd(inoutbuf+(i+2));
		xmm[6] = _mm_load_pd(inoutbuf+(i+4));
		xmm[7] = _mm_load_pd(inoutbuf+(i+6));

		/* Adds the double-precision, floating-point values */
		xmm[0] = _mm_add_pd(xmm[0], xmm[4]);
		xmm[1] = _mm_add_pd(xmm[1], xmm[5]);
		xmm[2] = _mm_add_pd(xmm[2], xmm[6]);
		xmm[3] = _mm_add_pd(xmm[3], xmm[7]);

		/* stores the results */
		_mm_stream_pd(inoutbuf+(i+0), xmm[0]);
		_mm_stream_pd(inoutbuf+(i+2), xmm[1]);
		_mm_stream_pd(inoutbuf+(i+4), xmm[2]);
		_mm_stream_pd(inoutbuf+(i+6), xmm[3]);
    }

    for(;i<len;i++)
		inoutbuf[i]+=inbuf[i];
}

static void add_float_sse(float* inbuf, float* inoutbuf, unsigned int len) 
{
	__m128			xmm[8];
    unsigned int	i, u;

    if(!len) 
		return;

	_mm_prefetch((char*)(inbuf), _MM_HINT_NTA);
	_mm_prefetch((char*)(inbuf+16), _MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf), _MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+16), _MM_HINT_NTA);
    
	u = 128 / (2*sizeof(float));
	for(i=0; (i+u)<len; i+=u) 
	{
		_mm_prefetch((char*)(inbuf+(i+32)), _MM_HINT_NTA);
		_mm_prefetch((char*)(inbuf+(i+48)), _MM_HINT_NTA);
		_mm_prefetch((char*)(inoutbuf+(i+32)), _MM_HINT_NTA);
		_mm_prefetch((char*)(inoutbuf+(i+48)), _MM_HINT_NTA);

		/* load 128 Byte into xmm register */
		xmm[0] = _mm_load_ps(inbuf+(i+0));
		xmm[1] = _mm_load_ps(inbuf+(i+4));
		xmm[2] = _mm_load_ps(inbuf+(i+8));
		xmm[3] = _mm_load_ps(inbuf+(i+12));
		xmm[4] = _mm_load_ps(inoutbuf+(i+0));
		xmm[5] = _mm_load_ps(inoutbuf+(i+4));
		xmm[6] = _mm_load_ps(inoutbuf+(i+8));
		xmm[7] = _mm_load_ps(inoutbuf+(i+12));

		/* Adds the single-precision, floating-point values */
		xmm[0] = _mm_add_ps(xmm[0], xmm[4]);
		xmm[1] = _mm_add_ps(xmm[1], xmm[5]);
		xmm[2] = _mm_add_ps(xmm[2], xmm[6]);
		xmm[3] = _mm_add_ps(xmm[3], xmm[7]);

		/* stores the results */
		_mm_stream_ps(inoutbuf+(i+0), xmm[0]);
		_mm_stream_ps(inoutbuf+(i+4), xmm[1]);
		_mm_stream_ps(inoutbuf+(i+8), xmm[2]);
		_mm_stream_ps(inoutbuf+(i+12), xmm[3]);
    }

    for(;i<len;i++)
		inoutbuf[i]+=inbuf[i];
}

static void add_s_complex_sse(s_complex *inbuf, s_complex *inoutbuf, unsigned int len) 
{
	add_float_sse((float*) inbuf, (float*) inoutbuf, 2*len); 
}

static void add_d_complex_sse(d_complex *inbuf, d_complex *inoutbuf, unsigned int len) 
{
	add_double_sse((double*) inbuf, (double*) inoutbuf, 2*len); 
}

static const proto_func jmp_add_sse[] = {
    add_int_sse,    /*int*/
    add_float_sse,    /*float*/
    add_double_sse,    /*double*/
    add_s_complex_sse,  /*complex*/
    add_int_sse,    /*long*/
    add_short_sse,  /*short*/
    add_byte_sse,   /*char*/
    add_byte_sse,   /*byte*/
    add_byte_sse,   /*uchar*/
    add_short_sse,  /*ushort*/
    add_int_sse,    /*ulong*/
    add_int_sse,    /*uint*/
    add_error_func, /*contig*/
    add_error_func, /*vector*/
    add_error_func, /*hvector*/
    add_error_func, /*indexed*/
    add_error_func, /*hindexed*/
    add_error_func, /*struct*/
    add_d_complex_sse,  /*double_complex*/
    add_error_func, /*packed*/
    add_error_func, /*ub*/
    add_error_func, /*lb*/
    add_double_sse,	    /*longdouble*/
    add_error_func, /*longlongint*/
    add_error_func, /*logical*/
    add_int_sse	    /*fort_int*/
};

void MPIR_add_sse(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_add_sse[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}

/* ---------------------MPI_PROD----------------------------------*/
extern void prod_error_func(void* d1,void*d2,unsigned int l);

static void prod_byte_sse(char *inbuf,char *inoutbuf,unsigned int len) 
{
	unsigned int i, u;

	if(!len) 
		return;

	_mm_prefetch((char*)(inbuf+32), _MM_HINT_NTA);
	_mm_prefetch((char*)(inoutbuf+32), _MM_HINT_NTA);

	u=len&(~31);   
	for(i=0; i<u; i+=32) 
	{
		_mm_prefetch((char*)(inbuf+i+64), _MM_HINT_NTA);
		_mm_prefetch((char*)(inoutbuf+i+64), _MM_HINT_NTA);

		inoutbuf[i]*=inbuf[i];
		inoutbuf[i+1]*=inbuf[i+1];
		inoutbuf[i+2]*=inbuf[i+2];
		inoutbuf[i+3]*=inbuf[i+3];
		inoutbuf[i+4]*=inbuf[i+4];
		inoutbuf[i+5]*=inbuf[i+5];
		inoutbuf[i+6]*=inbuf[i+6];
		inoutbuf[i+7]*=inbuf[i+7];
		inoutbuf[i+8]*=inbuf[i+8];
		inoutbuf[i+9]*=inbuf[i+9];
		inoutbuf[i+10]*=inbuf[i+10];
		inoutbuf[i+11]*=inbuf[i+11];
		inoutbuf[i+12]*=inbuf[i+12];
		inoutbuf[i+13]*=inbuf[i+13];
		inoutbuf[i+14]*=inbuf[i+14];
		inoutbuf[i+15]*=inbuf[i+15];
		inoutbuf[i+16]*=inbuf[i+16];
		inoutbuf[i+17]*=inbuf[i+17];
		inoutbuf[i+18]*=inbuf[i+18];
		inoutbuf[i+19]*=inbuf[i+19];
		inoutbuf[i+20]*=inbuf[i+20];
		inoutbuf[i+21]*=inbuf[i+21];
		inoutbuf[i+22]*=inbuf[i+22];
		inoutbuf[i+23]*=inbuf[i+23];
		inoutbuf[i+24]*=inbuf[i+24];
		inoutbuf[i+25]*=inbuf[i+25];
		inoutbuf[i+26]*=inbuf[i+26];
		inoutbuf[i+27]*=inbuf[i+27];
		inoutbuf[i+28]*=inbuf[i+28];
		inoutbuf[i+29]*=inbuf[i+29];
		inoutbuf[i+30]*=inbuf[i+30];
		inoutbuf[i+31]*=inbuf[i+31];
	}

	for(;i<len;++i)
		inoutbuf[i]*=inbuf[i];
}

static void prod_double_sse(double *inbuf, double* inoutbuf, unsigned int len) 
{
	__m128d			xmm[8];
    unsigned int	i, u;

    if(!len) 
		return;

	_mm_prefetch((char*)(inbuf), _MM_HINT_NTA);
	_mm_prefetch((char*)(inbuf+8), _MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf), _MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+8), _MM_HINT_NTA);
    
	u = 128 / (2*sizeof(double));
	for(i=0; (i+u)<len; i+=u) 
	{
		_mm_prefetch((char*)(inbuf+(i+16)), _MM_HINT_NTA);
		_mm_prefetch((char*)(inbuf+(i+24)), _MM_HINT_NTA);
		_mm_prefetch((char*)(inoutbuf+(i+16)), _MM_HINT_NTA);
		_mm_prefetch((char*)(inoutbuf+(i+24)), _MM_HINT_NTA);

		/* load 128 Byte into xmm register */
		xmm[0] = _mm_load_pd(inbuf+(i+0));
		xmm[1] = _mm_load_pd(inbuf+(i+2));
		xmm[2] = _mm_load_pd(inbuf+(i+4));
		xmm[3] = _mm_load_pd(inbuf+(i+6));
		xmm[4] = _mm_load_pd(inoutbuf+(i+0));
		xmm[5] = _mm_load_pd(inoutbuf+(i+2));
		xmm[6] = _mm_load_pd(inoutbuf+(i+4));
		xmm[7] = _mm_load_pd(inoutbuf+(i+6));

		/* Multiplies the double-precision, floating-point values */
		xmm[0] = _mm_mul_pd(xmm[0], xmm[4]);
		xmm[1] = _mm_mul_pd(xmm[1], xmm[5]);
		xmm[2] = _mm_mul_pd(xmm[2], xmm[6]);
		xmm[3] = _mm_mul_pd(xmm[3], xmm[7]);

		/* stores the results */
		_mm_stream_pd(inoutbuf+(i+0), xmm[0]);
		_mm_stream_pd(inoutbuf+(i+2), xmm[1]);
		_mm_stream_pd(inoutbuf+(i+4), xmm[2]);
		_mm_stream_pd(inoutbuf+(i+6), xmm[3]);
    }

    for(;i<len;i++)
		inoutbuf[i]+=inbuf[i];
}

static void prod_float_sse(float* inbuf, float* inoutbuf, unsigned int len) 
{
	__m128			xmm[8];
    unsigned int	i, u;

    if(!len) 
		return;

	_mm_prefetch((char*)(inbuf), _MM_HINT_NTA);
	_mm_prefetch((char*)(inbuf+16), _MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf), _MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+16), _MM_HINT_NTA);
    
	u = 128 / (2*sizeof(float));
	for(i=0; (i+u)<len; i+=u) 
	{
		_mm_prefetch((char*)(inbuf+(i+32)), _MM_HINT_NTA);
		_mm_prefetch((char*)(inbuf+(i+48)), _MM_HINT_NTA);
		_mm_prefetch((char*)(inoutbuf+(i+32)), _MM_HINT_NTA);
		_mm_prefetch((char*)(inoutbuf+(i+48)), _MM_HINT_NTA);

		/* load 128 Byte into xmm register */
		xmm[0] = _mm_load_ps(inbuf+(i+0));
		xmm[1] = _mm_load_ps(inbuf+(i+4));
		xmm[2] = _mm_load_ps(inbuf+(i+8));
		xmm[3] = _mm_load_ps(inbuf+(i+12));
		xmm[4] = _mm_load_ps(inoutbuf+(i+0));
		xmm[5] = _mm_load_ps(inoutbuf+(i+4));
		xmm[6] = _mm_load_ps(inoutbuf+(i+8));
		xmm[7] = _mm_load_ps(inoutbuf+(i+12));

		/* Multiplies the double-precision, floating-point values */
		xmm[0] = _mm_mul_ps(xmm[0], xmm[4]);
		xmm[1] = _mm_mul_ps(xmm[1], xmm[5]);
		xmm[2] = _mm_mul_ps(xmm[2], xmm[6]);
		xmm[3] = _mm_mul_ps(xmm[3], xmm[7]);

		/* stores the results */
		_mm_stream_ps(inoutbuf+(i+0), xmm[0]);
		_mm_stream_ps(inoutbuf+(i+4), xmm[1]);
		_mm_stream_ps(inoutbuf+(i+8), xmm[2]);
		_mm_stream_ps(inoutbuf+(i+12), xmm[3]);
    }

    for(;i<len;i++)
		inoutbuf[i]+=inbuf[i];
}

/* static void prod_s_complex_sse(s_complex *inbuf,s_complex *inoutbuf,unsigned int len) {
    unsigned int i,u;
    float c;

    if(!len) return;
    _mm_prefetch((char*)(inbuf+4),_MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+4),_MM_HINT_NTA);
    u=len&(~3);

    for(i=0;i<u;i+=4) {
	_mm_prefetch((char*)(inbuf+i+8),_MM_HINT_NTA);
	_mm_prefetch((char*)(inoutbuf+i+8),_MM_HINT_NTA);
	c = inoutbuf[i].re;
	inoutbuf[i].re = inoutbuf[i].re*inbuf[i].re - inoutbuf[i].im*inbuf[i].im;
	inoutbuf[i].im = c*inbuf[i].im+inoutbuf[i].im*inbuf[i].re ;
	
	c = inoutbuf[i+1].re;
	inoutbuf[i+1].re = inoutbuf[i+1].re*inbuf[i+1].re - inoutbuf[i+1].im*inbuf[i+1].im;
	inoutbuf[i+1].im = c*inbuf[i+1].im+inoutbuf[i+1].im*inbuf[i+1].re ;

	c = inoutbuf[i+2].re;
	inoutbuf[i+2].re = inoutbuf[i+2].re*inbuf[i+2].re - inoutbuf[i+2].im*inbuf[i+2].im;
	inoutbuf[i+2].im = c*inbuf[i+2].im+inoutbuf[i+2].im*inbuf[i+2].re ;

	c = inoutbuf[i+3].re;
	inoutbuf[i+3].re = inoutbuf[i+3].re*inbuf[i+3].re - inoutbuf[i+3].im*inbuf[i+3].im;
	inoutbuf[i+3].im = c*inbuf[i+3].im+inoutbuf[i+3].im*inbuf[i+3].re ;
    }
    for(;i<len;++i) {
	c = inoutbuf[i].re;
	inoutbuf[i].re = inoutbuf[i].re*inbuf[i].re - inoutbuf[i].im*inbuf[i].im;
	inoutbuf[i].im = c*inbuf[i].im+inoutbuf[i].im*inbuf[i].re ;
    }
}

static void prod_d_complex_sse(d_complex *inbuf,d_complex *inoutbuf,unsigned int len) {
    unsigned int i,u;
    double c;
    if(!len) return;
    _mm_prefetch((char*)(inbuf+2),_MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+2),_MM_HINT_NTA);
    u=len&(~2);
    for(i=0;i<u;i+=2) {
	_mm_prefetch((char*)(inbuf+i+4),_MM_HINT_NTA);
	_mm_prefetch((char*)(inoutbuf+i+4),_MM_HINT_NTA);
	c = inoutbuf[i].re;
	inoutbuf[i].re = inoutbuf[i].re*inbuf[i].re - inoutbuf[i].im*inbuf[i].im;
	inoutbuf[i].im = c*inbuf[i].im+inoutbuf[i].im*inbuf[i].re ;
	
	c = inoutbuf[i+1].re;
	inoutbuf[i+1].re = inoutbuf[i+1].re*inbuf[i+1].re - inoutbuf[i+1].im*inbuf[i+1].im;
	inoutbuf[i+1].im = c*inbuf[i+1].im+inoutbuf[i+1].im*inbuf[i+1].re ;
    }
    for(;i<len;++i) {
	c = inoutbuf[i].re;
	inoutbuf[i].re = inoutbuf[i].re*inbuf[i].re - inoutbuf[i].im*inbuf[i].im;
	inoutbuf[i].im = c*inbuf[i].im+inoutbuf[i].im*inbuf[i].re ;
    }
} */

#endif 

#endif /* WIN32 */
