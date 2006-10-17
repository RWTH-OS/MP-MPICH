/*
 *  $Id$
 *
 */


#ifndef WIN32
#error sse_ops.c is for Win32 only
#else

#include "mpiimpl.h"
#include "x86_ops.h"
#include "mpiops.h"
#include <mmintrin.h>

void add_error_func(void* d1,void*d2,unsigned int l) 
{
    MPIR_Op_errno = MPIR_ERRCLASS_TO_CODE(MPI_ERR_OP,MPIR_ERR_NOT_DEFINED);
    MPIR_ERROR(MPIR_COMM_WORLD,MPIR_Op_errno, "MPI_SUM" );
}

#if defined(_M_IX86)

/* This is used to align buffers to
   a given boundary. The algorithms assume that:
   1) Both buffers (in and inout) are equally aligned
   2) They are at least aligned to the size of the data type.
*/

#define ALIGNEMENT 32

#define LN_DOUBLE 3
#define LN_INT 2
#define LN_FLOAT 2
#define LN_SHORT 1


/* ---------------------MPI_SUM----------------------------------*/
extern void add_error_func(void* d1,void*d2,unsigned int l);
static void add_int_mmx(int *inbuf,int*inoutbuf,unsigned int len);
static void add_short_mmx(short *inbuf,short *inoutbuf,unsigned int len);
static void add_byte_mmx(char *inbuf,char *inoutbuf,unsigned int len);

static void add_float(float *inbuf,float* inoutbuf,unsigned int len) {
    unsigned int i,u;
    u=len&(~3);   
    for(i=0;i<u;i+=4) {
	inoutbuf[i]+=inbuf[i];
	inoutbuf[i+1]+=inbuf[i+1];
	inoutbuf[i+2]+=inbuf[i+2];
	inoutbuf[i+3]+=inbuf[i+3];
    }
    for(;i<len;++i)
	inoutbuf[i]+=inbuf[i];
}

void add_double(double *inbuf,double* inoutbuf,unsigned int len) {
    unsigned int i,u;

    u=len&(~3);   
    for(i=0;i<u;i+=4) {
	inoutbuf[i]+=inbuf[i];
	inoutbuf[i+1]+=inbuf[i+1];
	inoutbuf[i+2]+=inbuf[i+2];
	inoutbuf[i+3]+=inbuf[i+3];
    }
    for(;i<len;++i)
	inoutbuf[i]+=inbuf[i];
}

static void add_s_complex(s_complex *inbuf,s_complex *inoutbuf,unsigned int len) {
    unsigned int i,u;
    u=len&(~3);   
    for(i=0;i<u;i+=4) {
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

void add_d_complex(d_complex *inbuf,d_complex *inoutbuf,unsigned int len) {
    unsigned int i,u;
    u=len&(~3);   
    for(i=0;i<u;i+=4) {
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

/*
    MPIR_INT=0, MPIR_FLOAT, MPIR_DOUBLE, MPIR_COMPLEX, MPIR_LONG, MPIR_SHORT,
    MPIR_CHAR, MPIR_BYTE, MPIR_UCHAR, MPIR_USHORT, MPIR_ULONG, MPIR_UINT,
    MPIR_CONTIG, MPIR_VECTOR, MPIR_HVECTOR, 
    MPIR_INDEXED,
    MPIR_HINDEXED, MPIR_STRUCT, MPIR_DOUBLE_COMPLEX, MPIR_PACKED, 
	MPIR_UB, MPIR_LB, MPIR_LONGDOUBLE, MPIR_LONGLONGINT, 
    MPIR_LOGICAL, MPIR_FORT_INT 
*/

static const proto_func jmp_add_mmx[] = {
    add_int_mmx,    /*int*/
    add_float,	    /*float*/
    add_double,	    /*double*/
    add_s_complex,  /*complex*/
    add_int_mmx,    /*long*/
    add_short_mmx,  /*short*/
    add_byte_mmx,   /*char*/
    add_byte_mmx,   /*byte*/
    add_byte_mmx,   /*uchar*/
    add_short_mmx,  /*ushort*/
    add_int_mmx,    /*ulong*/
    add_int_mmx,    /*uint*/
    add_error_func, /*contig*/
    add_error_func, /*vector*/
    add_error_func, /*hvector*/
    add_error_func, /*indexed*/
    add_error_func, /*hindexed*/
    add_error_func, /*struct*/
    add_d_complex,  /*double_complex*/
    add_error_func, /*packed*/
    add_error_func, /*ub*/
    add_error_func, /*lb*/
    add_double,	    /*longdouble*/
    add_error_func, /*longlongint*/
    add_error_func, /*logical*/
    add_int_mmx	    /*fort_int*/
};

void MPIR_add_mmx(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_add_mmx[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}

__declspec(naked)
void add_int_mmx(int *inbuf,int*inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;
	shr	edx,1				; divide by 2
	jz	remain
	
	
al_top:	
	movq	mm1, QWORD PTR [eax]		; Add integers
	movq	mm0, QWORD PTR [ecx+eax]	; 2 at a time
	add	eax,8
	paddd	mm0, mm1			;
	movq	QWORD PTR [eax-8], mm0		;
	
	dec	edx
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
	mov	eax, ecx
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;
startit:

	mov	edx, DWORD PTR [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT unroll_end		; len==0 --> less than 8 numbers
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*2 integers.
	    
	add	eax, 32				; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
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
	mov	edx, DWORD PTR [ecx+eax]	; Add the remaining integer
	add	edx, DWORD PTR [eax]		;
	mov	[eax],edx			;
end:
	pop	ebx
	ret	
    }
}

__declspec(naked)
void add_short_mmx(short *inbuf,short* inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
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
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*4 shorts.
	    
	add	eax, 32				; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
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
void add_byte_mmx(char *inbuf,char *inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
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
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*8 bytes.
	    
	add	eax, 32				; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
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


/* ---------------------MPI_PROD----------------------------------*/
void prod_error_func(void* d1,void*d2,unsigned int l) {
    MPIR_Op_errno = MPIR_ERRCLASS_TO_CODE(MPI_ERR_OP,MPIR_ERR_NOT_DEFINED);
    MPIR_ERROR(MPIR_COMM_WORLD,MPIR_Op_errno, "MPI_PROD" );
}

static void prod_short_mmx(short *inbuf,short *inoutbuf,unsigned int len);

static void prod_int(int *inbuf,int*inoutbuf,unsigned int len) {
    unsigned int i,u;
    u=len&(~3);   
    for(i=0;i<u;i+=4) {
	inoutbuf[i]*=inbuf[i];
	inoutbuf[i+1]*=inbuf[i+1];
	inoutbuf[i+2]*=inbuf[i+2];
	inoutbuf[i+3]*=inbuf[i+3];
    }
    for(;i<len;++i)
	inoutbuf[i]*=inbuf[i];
}

static void prod_byte(char *inbuf,char *inoutbuf,unsigned int len) {
    unsigned int i,u;
    u=len&(~3);   
    for(i=0;i<u;i+=4) {
	inoutbuf[i]*=inbuf[i];
	inoutbuf[i+1]*=inbuf[i+1];
	inoutbuf[i+2]*=inbuf[i+2];
	inoutbuf[i+3]*=inbuf[i+3];
    }
    for(;i<len;++i)
	inoutbuf[i]*=inbuf[i];
}

static void prod_float(float *inbuf,float* inoutbuf,unsigned int len) {
    unsigned int i,u;
    u=len&(~3);   
    for(i=0;i<u;i+=4) {
	inoutbuf[i]*=inbuf[i];
	inoutbuf[i+1]*=inbuf[i+1];
	inoutbuf[i+2]*=inbuf[i+2];
	inoutbuf[i+3]*=inbuf[i+3];
    }
    for(;i<len;++i)
	inoutbuf[i]*=inbuf[i];
}

static void prod_double(double *inbuf,double* inoutbuf,unsigned int len) {
    unsigned int i,u;

    u=len&(~3);   
    for(i=0;i<u;i+=4) {
	inoutbuf[i]*=inbuf[i];
	inoutbuf[i+1]*=inbuf[i+1];
	inoutbuf[i+2]*=inbuf[i+2];
	inoutbuf[i+3]*=inbuf[i+3];
    }
    for(;i<len;++i)
	inoutbuf[i]*=inbuf[i];
}

static void prod_s_complex(s_complex *inbuf,s_complex *inoutbuf,unsigned int len) {
    unsigned int i,u;
    float c;
    u=len&(~3);

    for(i=0;i<u;i+=4) {
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

static void prod_d_complex(d_complex *inbuf,d_complex *inoutbuf,unsigned int len) {
    unsigned int i,u;
    double c;
    
    u=len&(~3);
    for(i=0;i<u;i+=4) {
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

static  const proto_func jmp_prod_mmx[] = {
    prod_int,	    /*int*/
    prod_float,	    /*float*/
    prod_double,    /*double*/
    prod_s_complex, /*complex*/
    prod_int,	    /*long*/
    prod_short_mmx, /*short*/
    prod_byte,	    /*char*/
    prod_byte,	    /*byte*/
    prod_byte,	    /*uchar*/
    prod_short_mmx, /*ushort*/
    prod_int,	    /*ulong*/
    prod_int,	    /*uint*/
    prod_error_func,/*contig*/
    prod_error_func,/*vector*/
    prod_error_func,/*hvector*/
    prod_error_func,/*indexed*/
    prod_error_func,/*hindexed*/
    prod_error_func,/*struct*/
    prod_d_complex, /*double_complex*/
    prod_error_func,/*packed*/
    prod_error_func,/*ub*/
    prod_error_func,/*lb*/
    prod_double,    /*longdouble*/
    prod_error_func,/*longlongint*/
    prod_error_func,/*logical*/
    prod_int	    /*fort_int*/
};

void MPIR_prod_mmx(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_prod_mmx[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}

__declspec(naked)
void prod_short_mmx(short *inbuf,short* inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
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
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*4 shorts.
	    
	add	eax, 32				; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
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


/* ---------------------MPI_MAX----------------------------------*/
void max_error_func(void* d1,void*d2,unsigned int l) {
    MPIR_Op_errno = MPIR_ERRCLASS_TO_CODE(MPI_ERR_OP,MPIR_ERR_NOT_DEFINED);
    MPIR_ERROR(MPIR_COMM_WORLD,MPIR_Op_errno, "MPI_MAX" );
}

static void max_sint_mmx(int *inbuf,int*inoutbuf,unsigned int len);
static void max_sshort_mmx(short *inbuf,short *inoutbuf,unsigned int len);
static void max_sbyte_mmx(signed char *inbuf,signed char *inoutbuf,unsigned int len);

static void max_float_cmov(float *inbuf,float *inoutbuf,unsigned int len);
static void max_double_cmov(double *inbuf,double *inoutbuf,unsigned int len);
static void max_uint_cmov(unsigned int *inbuf,unsigned int*inoutbuf,unsigned int len);
static void max_ushort_cmov(unsigned short *inbuf,unsigned short *inoutbuf,unsigned int len);
static void max_ubyte_cmov(unsigned char *inbuf,unsigned char *inoutbuf,unsigned int len);


static  const proto_func jmp_max_mmx_cmov[] = {
    max_sint_mmx,   /*int*/
    max_float_cmov, /*float*/
    max_double_cmov,/*double*/
    max_error_func, /*complex*/
    max_sint_mmx,   /*long*/
    max_sshort_mmx, /*short*/
    max_sbyte_mmx,  /*char*/
    max_ubyte_cmov, /*byte*/
    max_ubyte_cmov, /*uchar*/
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
    max_sint_mmx    /*fort_int*/
};

void MPIR_max_mmx_cmov(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_max_mmx_cmov[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}



__declspec(naked)
void max_sint_mmx(int *inbuf,int*inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
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
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that compares 4*2 integers.
	    
	add	eax, 32				; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	movq	mm4,mm0
	pcmpgtd	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	movq	mm4,mm2
	pcmpgtd	mm4,mm3
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4				;
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	movq	mm4,mm0
	pcmpgtd	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
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
void max_sshort_mmx(short *inbuf,short* inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	remain
		
al_top:	
	movq	mm0, QWORD PTR [ecx+eax]	; compare shorts
	movq	mm1, QWORD PTR [eax]		; 4 at a time
	add	eax,8
	movq	mm4,mm0
	pcmpgtw	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
	movq	QWORD PTR [eax-8], mm0		;
	
	dec	edx
	jne	al_top
remain:
	and	bl,3
	jz	startit
al_rest:
	mov	dx,WORD PTR [ecx+eax]		; Add remaining
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
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that compares 4*4 shorts.
	    
	add	eax, 32				; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	movq	mm4,mm0
	pcmpgtw	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	movq	mm4,mm2
	pcmpgtw	mm4,mm3
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	movq	mm4,mm0
	pcmpgtw	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	movq	mm4,mm2
	pcmpgtw	mm4,mm3
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4
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
	movq	mm4,mm0
	pcmpgtw	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
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
void max_sbyte_mmx(char *inbuf,char *inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+20]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+20],ecx		; store new length

	mov	esi, edx			; Save counter
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
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
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that compares 4*8 bytes.
	    
	add	eax, 32				; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	movq	mm4,mm0
	pcmpgtb	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	movq	mm4,mm2
	pcmpgtb	mm4,mm3
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	movq	mm4,mm0
	pcmpgtb	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
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
void max_uint_cmov(unsigned int *inbuf,unsigned int*inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	push	esi
	mov	esi, DWORD PTR [esp+20]		; Load length
	test	esi, esi
	jz	end
	mov	eax, DWORD PTR [esp+16]		; load inoutbuf
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	sub	ecx, eax			;	
	shr	esi, 2				; len /= 4
	jz	SHORT unroll_end		; len==0 --> less than 4 numbers

align 4
ltop:
	mov	ebx, DWORD PTR [ecx+eax]	; This is an unrolled loop
	mov	edx, DWORD PTR [eax]		; that compares 4 integers.
	add	eax, 16	
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
	and	si,3				; calculate remaining 
	jz	end

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
	mov	esi, DWORD PTR [esp+20]		; Load length
	test	esi, esi
	jz	end
	mov	eax, DWORD PTR [esp+16]		; load inoutbuf
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	sub	ecx, eax			;	
	shr	esi, 2				; len /= 4
	jz	SHORT unroll_end		; len==0 --> less than 4 numbers

align 4
ltop:
	mov	bx, WORD PTR [ecx+eax]		; This is an unrolled loop
	mov	dx, WORD PTR [eax]		; that compares 4 shorts.
	add	eax, 8	
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

	jnz	SHORT ltop

unroll_end:	
	mov	esi, DWORD PTR [esp+20]		; load original length
	and	si,3				; calculate remaining 
	jz	end

lrest:	
	mov	dx, WORD PTR [ecx+eax]		; compare the remaining shorts
	mov	bx, WORD PTR [eax]		; 
	add	eax, 2				
	cmp	bx, dx
	cmova   dx, bx
	dec	si
	mov	WORD PTR [eax-2],dx	
	jnz	SHORT lrest
end:
	pop	esi
	pop	ebx
	ret	
    }
}

__declspec(naked)
void max_ubyte_cmov(unsigned char *inbuf,unsigned char* inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	push	esi
	mov	esi, DWORD PTR [esp+20]		; Load length
	test	esi, esi
	jz	end
	mov	eax, DWORD PTR [esp+16]		; load inoutbuf
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	sub	ecx, eax			;	
	shr	esi, 2				; len /= 4
	jz	SHORT unroll_end		; len==0 --> less than 4 numbers

align 4
ltop:
	mov	bl, BYTE PTR [ecx+eax]		; This is an unrolled loop
	mov	dl, BYTE PTR [eax]		; that compares 4 bytes.
	add	eax, 4
	cmp	bl, dl
	cmova	dx, bx
	mov	BYTE PTR [eax-4],dl

	mov	bl, BYTE PTR [ecx+eax-3]	
	mov	dl, BYTE PTR [eax-3]		
	cmp	bl, dl
	cmova	dx, bx
	mov	BYTE PTR [eax-3],dl

	mov	bl, BYTE PTR [ecx+eax-2]	
	mov	dl, BYTE PTR [eax-2]		
	cmp	bl, dl
	cmova	dx, bx
	mov	BYTE PTR [eax-2],dl

	mov	bl, BYTE PTR [ecx+eax-1]	
	mov	dl, BYTE PTR [eax-1]		
	cmp	bl, dl
	cmova	dx, bx
	dec	esi	
	mov	BYTE PTR [eax-1],dl

	jnz	SHORT ltop

unroll_end:	
	mov	esi, DWORD PTR [esp+20]		; load original length
	and	esi,3				; calculate remaining 
	jz	end

lrest:	
	mov	dl, BYTE PTR [ecx+eax]		; compare the remaining shorts
	mov	bl, BYTE PTR [eax]		; 
	inc	eax
	cmp	bl, dl
	cmova   dx, bx
	dec	si
	mov	BYTE PTR [eax-1],dl	
	jnz	SHORT lrest
end:
	pop	esi
	pop	ebx
	ret	
    }
}

__declspec(naked)
void max_float_cmov(float *inbuf,float* inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	push	esi
	mov	esi, DWORD PTR [esp+20]		; Load length
	test	esi, esi
	jz	end
	mov	eax, DWORD PTR [esp+16]		; load inoutbuf
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	sub	ecx, eax			;	
	shr	esi, 2				; len /= 4
	jz	SHORT unroll_end		; len==0 --> less than 4 numbers

align 4
ltop:
	fld	DWORD PTR [ecx+eax]		; This is an unrolled loop
	fld	DWORD PTR [eax]			; that compares 4 floats.
	add	eax, 16	
	fucomi	st(0),st(1)
	fcmovb	st(0),st(1)
	fstp	DWORD PTR [eax-16]
	ffree	st(0)

	fld	DWORD PTR [ecx+eax-12]
	fld	DWORD PTR [eax-12]
	fucomi	st(0),st(1)
	fcmovb	st(0),st(1)
	fstp	DWORD PTR [eax-12]
	ffree	st(0)

	fld	DWORD PTR [ecx+eax-8]
	fld	DWORD PTR [eax-8]
	fucomi	st(0),st(1)
	fcmovb	st(0),st(1)
	fstp	DWORD PTR [eax-8]
	ffree	st(0)

	fld	DWORD PTR [ecx+eax-4]
	fld	DWORD PTR [eax-4]
	fucomi	st(0),st(1)
	fcmovb	st(0),st(1)
	dec	esi
	fstp	DWORD PTR [eax-4]
	ffree	st(0)
	jne	SHORT ltop

unroll_end:	
	mov	esi, DWORD PTR [esp+20]		; load original length
	and	si,3				; calculate remaining 
	jz	end

lrest:	
	fld	DWORD PTR [ecx+eax]		; compare the remaining numbers
	fld	DWORD PTR [eax]			; 
	add	eax, 4				
	fucomi	st(0),st(1)
	fcmovb	st(0),st(1)
	dec	si
	fstp	DWORD PTR [eax-4]
	ffree	st(0)
	jnz	lrest
end:
	pop	esi
	pop	ebx
	ret	
    }
}

__declspec(naked)
void max_double_cmov(double *inbuf,double* inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	push	esi
	mov	esi, DWORD PTR [esp+20]		; Load length
	test	esi, esi
	jz	end
	mov	eax, DWORD PTR [esp+16]		; load inoutbuf
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	sub	ecx, eax			;	
	shr	esi, 2				; len /= 4
	jz	SHORT unroll_end		; len==0 --> less than 4 numbers

align 4
ltop:
	fld	QWORD PTR [ecx+eax]		; This is an unrolled loop
	fld	QWORD PTR [eax]			; that compares 4 doubles.
	add	eax, 32	
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
	dec	esi
	fstp	QWORD PTR [eax-8]
	ffree	st(0)
	jne	SHORT ltop

unroll_end:	
	mov	esi, DWORD PTR [esp+20]		; load original length
	and	si,3				; calculate remaining 
	jz	end

lrest:	
	fld	QWORD PTR [ecx+eax]		; compare the remaining numbers
	fld	QWORD PTR [eax]			; 
	add	eax, 8				
	fucomi	st(0),st(1)
	fcmovb	st(0),st(1)
	dec	si
	fstp	QWORD PTR [eax-8]
	ffree	st(0)
	jnz	lrest
end:
	pop	esi
	pop	ebx
	ret	
    }
}


/* ---------------------MPI_MIN----------------------------------*/
void min_error_func(void* d1,void*d2,unsigned int l) {
    MPIR_Op_errno = MPIR_ERRCLASS_TO_CODE(MPI_ERR_OP,MPIR_ERR_NOT_DEFINED);
    MPIR_ERROR(MPIR_COMM_WORLD,MPIR_Op_errno, "MPI_MIN" );
}

static void min_sint_mmx(int *inbuf,int*inoutbuf,unsigned int len);
static void min_sshort_mmx(short *inbuf,short *inoutbuf,unsigned int len);
static void min_sbyte_mmx(signed char *inbuf,signed char *inoutbuf,unsigned int len);

static void min_float_cmov(float *inbuf,float *inoutbuf,unsigned int len);
static void min_double_cmov(double *inbuf,double *inoutbuf,unsigned int len);
static void min_uint_cmov(unsigned int *inbuf,unsigned int*inoutbuf,unsigned int len);
static void min_ushort_cmov(unsigned short *inbuf,unsigned short *inoutbuf,unsigned int len);
static void min_ubyte_cmov(unsigned char *inbuf,unsigned char *inoutbuf,unsigned int len);


static  const proto_func jmp_min_mmx_cmov[] = {
    min_sint_mmx,   /*int*/
    min_float_cmov, /*float*/
    min_double_cmov,/*double*/
    min_error_func, /*complex*/
    min_sint_mmx,   /*long*/
    min_sshort_mmx, /*short*/
    min_sbyte_mmx,  /*char*/
    min_ubyte_cmov, /*byte*/
    min_ubyte_cmov, /*uchar*/
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
    min_sint_mmx    /*fort_int*/
};

void MPIR_min_mmx_cmov(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_min_mmx_cmov[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}



__declspec(naked)
void min_sint_mmx(int *inbuf,int*inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
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
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that compares 4*2 integers.
	    
	add	eax, 32				; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	movq	mm4,mm1
	pcmpgtd	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	movq	mm4,mm3
	pcmpgtd	mm4,mm2
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4				;
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	movq	mm4,mm1
	pcmpgtd	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
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
void min_sshort_mmx(short *inbuf,short* inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	remain
		
al_top:	
	movq	mm0, QWORD PTR [ecx+eax]	; compare shorts
	movq	mm1, QWORD PTR [eax]		; 4 at a time
	add	eax,8
	movq	mm4,mm1
	pcmpgtw	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
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
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that compares 4*4 shorts.
	    
	add	eax, 32				; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	movq	mm4,mm1
	pcmpgtw	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	movq	mm4,mm3
	pcmpgtw	mm4,mm2
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	movq	mm4,mm1
	pcmpgtw	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	movq	mm4,mm3
	pcmpgtw	mm4,mm2
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4
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
	movq	mm4,mm1
	pcmpgtw	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
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
void min_sbyte_mmx(char *inbuf,char *inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+20]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+20],ecx		; store new length

	mov	esi, edx			; Save counter
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
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
	cmovg	dx,bx
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
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that compares 4*8 bytes.
	    
	add	eax, 32				; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	movq	mm4,mm1
	pcmpgtb	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	movq	mm4,mm3
	pcmpgtb	mm4,mm2
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	movq	mm4,mm1
	pcmpgtb	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
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
	cmovg	dx,bx
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
	mov	esi, DWORD PTR [esp+20]		; Load length
	test	esi, esi
	jz	end
	mov	eax, DWORD PTR [esp+16]		; load inoutbuf
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	sub	ecx, eax			;	
	shr	esi, 2				; len /= 4
	jz	SHORT unroll_end		; len==0 --> less than 4 numbers

align 4
ltop:
	mov	ebx, DWORD PTR [ecx+eax]	; This is an unrolled loop
	mov	edx, DWORD PTR [eax]		; that compares 4 integers.
	add	eax, 16	
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
	and	si,3				; calculate remaining 
	jz	end

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
	mov	esi, DWORD PTR [esp+20]		; Load length
	test	esi, esi
	jz	end
	mov	eax, DWORD PTR [esp+16]		; load inoutbuf
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	sub	ecx, eax			;	
	shr	esi, 2				; len /= 4
	jz	SHORT unroll_end		; len==0 --> less than 4 numbers

align 4
ltop:
	mov	bx, WORD PTR [ecx+eax]		; This is an unrolled loop
	mov	dx, WORD PTR [eax]		; that compares 4 shorts.
	add	eax, 8	
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

	jnz	SHORT ltop

unroll_end:	
	mov	esi, DWORD PTR [esp+20]		; load original length
	and	si,3				; calculate remaining 
	jz	end

lrest:	
	mov	dx, WORD PTR [ecx+eax]		; compare the remaining shorts
	mov	bx, WORD PTR [eax]		; 
	add	eax, 2				
	cmp	bx, dx
	cmovb   dx, bx
	dec	si
	mov	WORD PTR [eax-2],dx	
	jnz	SHORT lrest
end:
	pop	esi
	pop	ebx
	ret	
    }
}

__declspec(naked)
void min_ubyte_cmov(unsigned char *inbuf,unsigned char* inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	push	esi
	mov	esi, DWORD PTR [esp+20]		; Load length
	test	esi, esi
	jz	end
	mov	eax, DWORD PTR [esp+16]		; load inoutbuf
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	sub	ecx, eax			;	
	shr	esi, 2				; len /= 4
	jz	SHORT unroll_end		; len==0 --> less than 4 numbers

align 4
ltop:
	mov	bl, BYTE PTR [ecx+eax]		; This is an unrolled loop
	mov	dl, BYTE PTR [eax]		; that compares 4 shorts.
	add	eax, 4
	cmp	bl, dl
	cmovb	dx, bx
	mov	BYTE PTR [eax-4],dl

	mov	bl, BYTE PTR [ecx+eax-3]	
	mov	dl, BYTE PTR [eax-3]		
	cmp	bl, dl
	cmovb	dx, bx
	mov	BYTE PTR [eax-3],dl

	mov	bl, BYTE PTR [ecx+eax-2]	
	mov	dl, BYTE PTR [eax-2]		
	cmp	bl, dl
	cmovb	dx, bx
	mov	BYTE PTR [eax-2],dl

	mov	bl, BYTE PTR [ecx+eax-1]	
	mov	dl, BYTE PTR [eax-1]		
	cmp	bl, dl
	cmovb	dx, bx
	dec	esi	
	mov	BYTE PTR [eax-1],dl

	jnz	SHORT ltop

unroll_end:	
	mov	esi, DWORD PTR [esp+20]		; load original length
	and	si,3				; calculate remaining 
	jz	end

lrest:	
	mov	dl, BYTE PTR [ecx+eax]		; compare the remaining shorts
	mov	bl, BYTE PTR [eax]		; 
	inc	eax
	cmp	bl, dl
	cmovb   dx, bx
	dec	si
	mov	BYTE PTR [eax-1],dl	
	jnz	SHORT lrest
end:
	pop	esi
	pop	ebx
	ret	
    }
}

__declspec(naked)
void min_float_cmov(float *inbuf,float* inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	push	esi
	mov	esi, DWORD PTR [esp+20]		; Load length
	test	esi, esi
	jz	end
	mov	eax, DWORD PTR [esp+16]		; load inoutbuf
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	sub	ecx, eax			;	
	shr	esi, 2				; len /= 4
	jz	SHORT unroll_end		; len==0 --> less than 4 numbers

align 4
ltop:
	fld	DWORD PTR [ecx+eax]		; This is an unrolled loop
	fld	DWORD PTR [eax]			; that compares 4 floats.
	add	eax, 16	
	fucomi	st(0),st(1)
	fcmovnbe	st(0),st(1)
	fstp	DWORD PTR [eax-16]
	ffree	st(0)

	fld	DWORD PTR [ecx+eax-12]
	fld	DWORD PTR [eax-12]
	fucomi	st(0),st(1)
	fcmovnbe	st(0),st(1)
	fstp	DWORD PTR [eax-12]
	ffree	st(0)

	fld	DWORD PTR [ecx+eax-8]
	fld	DWORD PTR [eax-8]
	fucomi	st(0),st(1)
	fcmovnbe	st(0),st(1)
	fstp	DWORD PTR [eax-8]
	ffree	st(0)

	fld	DWORD PTR [ecx+eax-4]
	fld	DWORD PTR [eax-4]
	fucomi	st(0),st(1)
	fcmovnbe	st(0),st(1)
	dec	esi
	fstp	DWORD PTR [eax-4]
	ffree	st(0)
	jne	SHORT ltop

unroll_end:	
	mov	esi, DWORD PTR [esp+20]		; load original length
	and	si,3				; calculate remaining 
	jz	end

lrest:	
	fld	DWORD PTR [ecx+eax]		; compare the remaining numbers
	fld	DWORD PTR [eax]			; 
	add	eax, 4				
	fucomi	st(0),st(1)
	fcmovnbe	st(0),st(1)
	dec	si
	fstp	DWORD PTR [eax-4]
	ffree	st(0)
	jnz	lrest
end:
	pop	esi
	pop	ebx
	ret	
    }
}

__declspec(naked)
void min_double_cmov(double *inbuf,double* inoutbuf,unsigned int len) {
__asm{

	
	push	ebx
	push	esi
	mov	esi, DWORD PTR [esp+20]		; Load length
	test	esi, esi
	jz	end
	mov	eax, DWORD PTR [esp+16]		; load inoutbuf
	mov	ecx, DWORD PTR [esp+12]		; load inbuf
	sub	ecx, eax			;	
	shr	esi, 2				; len /= 4
	jz	SHORT unroll_end		; len==0 --> less than 4 numbers

align 4
ltop:
	fld	QWORD PTR [ecx+eax]		; This is an unrolled loop
	fld	QWORD PTR [eax]			; that compares 4 doubles.
	add	eax, 32	
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
	dec	esi
	fstp	QWORD PTR [eax-8]
	ffree	st(0)
	jne	SHORT ltop

unroll_end:	
	mov	esi, DWORD PTR [esp+20]		; load original length
	and	si,3				; calculate remaining 
	jz	end

lrest:	
	fld	QWORD PTR [ecx+eax]		; compare the remaining numbers
	fld	QWORD PTR [eax]			; 
	add	eax, 8				
	fucomi	st(0),st(1)
	fcmovnbe	st(0),st(1)
	dec	si
	fstp	QWORD PTR [eax-8]
	ffree	st(0)
	jnz	lrest
end:
	pop	esi
	pop	ebx
	ret	
    }
}


/* ---------------------MPI_band----------------------------------*/
void band_error_func(void* d1,void*d2,unsigned int l) {
    MPIR_Op_errno = MPIR_ERRCLASS_TO_CODE(MPI_ERR_OP,MPIR_ERR_NOT_DEFINED);
    MPIR_ERROR(MPIR_COMM_WORLD,MPIR_Op_errno, "MPI_BAND" );
}

static void band_int_mmx(int *inbuf,int*inoutbuf,unsigned int len);
static void band_short_mmx(short *inbuf,short *inoutbuf,unsigned int len);
static void band_byte_mmx(signed char *inbuf,signed char *inoutbuf,unsigned int len);


static  const proto_func jmp_band_mmx[] = {
    band_int_mmx,   /*int*/
    band_error_func, /*float*/
    band_error_func,/*double*/
    band_error_func, /*complex*/
    band_int_mmx,   /*long*/
    band_short_mmx, /*short*/
    band_byte_mmx,  /*char*/
    band_byte_mmx, /*byte*/
    band_byte_mmx, /*uchar*/
    band_short_mmx,/*ushort*/
    band_int_mmx,  /*ulong*/
    band_int_mmx,  /*uint*/
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
    band_int_mmx,   /*logical*/
    band_int_mmx    /*fort_int*/
};

void MPIR_band_mmx(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_band_mmx[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}

__declspec(naked)
void band_int_mmx(int *inbuf,int*inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
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
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*2 integers.
	    
	add	eax, 32				; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	pand	mm0, mm1			;
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	pand	mm2, mm3			;
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	pand	mm0, mm1			;
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	pand	mm2, mm3			;
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
void band_short_mmx(short *inbuf,short* inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
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
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*4 shorts.
	    
	add	eax, 32				; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	pand	mm0, mm1			;
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	pand	mm2, mm3			;
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	pand	mm0, mm1			;
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	pand	mm2, mm3			;
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
	pand	mm0, mm1			;
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
void band_byte_mmx(char *inbuf,char *inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
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
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*8 bytes.
	    
	add	eax, 32				; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	pand	mm0, mm1			;
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	pand	mm2, mm3			;
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	pand	mm0, mm1			;
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	pand	mm2, mm3			;
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
	pand	mm0, mm1			;
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
void bor_error_func(void* d1,void*d2,unsigned int l) {
    MPIR_Op_errno = MPIR_ERRCLASS_TO_CODE(MPI_ERR_OP,MPIR_ERR_NOT_DEFINED);
    MPIR_ERROR(MPIR_COMM_WORLD,MPIR_Op_errno, "MPI_BOR" );
}

static void bor_int_mmx(int *inbuf,int*inoutbuf,unsigned int len);
static void bor_short_mmx(short *inbuf,short *inoutbuf,unsigned int len);
static void bor_byte_mmx(signed char *inbuf,signed char *inoutbuf,unsigned int len);


static  const proto_func jmp_bor_mmx[] = {
    bor_int_mmx,   /*int*/
    bor_error_func, /*float*/
    bor_error_func,/*double*/
    bor_error_func, /*complex*/
    bor_int_mmx,   /*long*/
    bor_short_mmx, /*short*/
    bor_byte_mmx,  /*char*/
    bor_byte_mmx, /*byte*/
    bor_byte_mmx, /*uchar*/
    bor_short_mmx,/*ushort*/
    bor_int_mmx,  /*ulong*/
    bor_int_mmx,  /*uint*/
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
    bor_int_mmx,   /*logical*/
    bor_int_mmx    /*fort_int*/
};

void MPIR_bor_mmx(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_bor_mmx[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}

__declspec(naked)
void bor_int_mmx(int *inbuf,int*inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
	sub	ecx, eax			;
	shr	edx,1				; divide by 2
	jz	remain
	
	
al_top:	
	movq	mm1, QWORD PTR [eax]		; handle integers
	movq	mm0, QWORD PTR [ecx+eax]	; 2 at a time
	add	eax,8
	por	mm0, mm1			;
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
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*2 integers.
	    
	add	eax, 32				; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	por	mm0, mm1			;
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	por	mm2, mm3			;
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	por	mm0, mm1			;
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	por	mm2, mm3			;
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
	por	mm0, mm1			;
	movq	QWORD PTR [eax-8], mm0		;
	dec	edx				;
	jne	lrest				;
norest:
	
	emms					;
	test	bl,1				;
	jz	end				; No--> we are finished
	mov	edx, DWORD PTR [ecx+eax]	; handle the remaining integer
	or	edx, DWORD PTR [eax]		;
	mov	[eax],edx			;
end:
	pop	ebx
	ret	
    }
}

__declspec(naked)
void bor_short_mmx(short *inbuf,short* inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
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
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*4 shorts.
	    
	add	eax, 32				; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	por	mm0, mm1			;
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	por	mm2, mm3			;
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	por	mm0, mm1			;
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	por	mm2, mm3			;
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
	por	mm0, mm1			;
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
void bor_byte_mmx(char *inbuf,char *inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
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
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*8 bytes.
	    
	add	eax, 32				; We try to interleave some
						; instructions to allow
	movq	mm2, QWORD PTR [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3, QWORD PTR [eax-24]		;
	  
	por	mm0, mm1			;
	movq	QWORD PTR [eax-32], mm0		;
	
	movq	mm0, QWORD PTR [ecx+eax-16]	;
	movq	mm1, QWORD PTR [eax-16]		;
	
	por	mm2, mm3			;
	movq	QWORD PTR [eax-24], mm2
	
	movq	mm2, QWORD PTR [ecx+eax-8]	;
	movq	mm3, QWORD PTR [eax-8]		;
	
	por	mm0, mm1			;
	movq	QWORD PTR [eax-16], mm0		;
	
	dec	edx				;
	por	mm2, mm3			;
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
	por	mm0, mm1			;
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
void bxor_error_func(void* d1,void*d2,unsigned int l) {
    MPIR_Op_errno = MPIR_ERRCLASS_TO_CODE(MPI_ERR_OP,MPIR_ERR_NOT_DEFINED);
    MPIR_ERROR(MPIR_COMM_WORLD,MPIR_Op_errno, "MPI_BXOR" );
}

static void bxor_int_mmx(int *inbuf,int*inoutbuf,unsigned int len);
static void bxor_short_mmx(short *inbuf,short *inoutbuf,unsigned int len);
static void bxor_byte_mmx(signed char *inbuf,signed char *inoutbuf,unsigned int len);


static  const proto_func jmp_bxor_mmx[] = {
    bxor_int_mmx,   /*int*/
    bxor_error_func, /*float*/
    bxor_error_func,/*double*/
    bxor_error_func, /*complex*/
    bxor_int_mmx,   /*long*/
    bxor_short_mmx, /*short*/
    bxor_byte_mmx,  /*char*/
    bxor_byte_mmx, /*byte*/
    bxor_byte_mmx, /*uchar*/
    bxor_short_mmx,/*ushort*/
    bxor_int_mmx,  /*ulong*/
    bxor_int_mmx,  /*uint*/
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
    bxor_int_mmx,   /*logical*/
    bxor_int_mmx    /*fort_int*/
};

void MPIR_bxor_mmx(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_bxor_mmx[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}

__declspec(naked)
void bxor_int_mmx(int *inbuf,int*inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
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
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*2 integers.
	    
	add	eax, 32				; We try to interleave some
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
void bxor_short_mmx(short *inbuf,short* inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
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
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*4 shorts.
	    
	add	eax, 32				; We try to interleave some
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
void bxor_byte_mmx(char *inbuf,char *inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value
match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
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
align 4
ltop:
	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*8 bytes.
	    
	add	eax, 32				; We try to interleave some
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
void lxor_error_func(void* d1,void*d2,unsigned int l) {
    MPIR_Op_errno = MPIR_ERRCLASS_TO_CODE(MPI_ERR_OP,MPIR_ERR_NOT_DEFINED);
    MPIR_ERROR(MPIR_COMM_WORLD,MPIR_Op_errno, "MPI_lxor" );
}

static void lxor_int_mmx(int *inbuf,int*inoutbuf,unsigned int len);
static void lxor_short_mmx(short *inbuf,short *inoutbuf,unsigned int len);
static void lxor_byte_mmx(signed char *inbuf,signed char *inoutbuf,unsigned int len);


static  const proto_func jmp_lxor_mmx[] = {
    lxor_int_mmx,   /*int*/
    lxor_error_func, /*float*/
    lxor_error_func,/*double*/
    lxor_error_func, /*complex*/
    lxor_int_mmx,   /*long*/
    lxor_short_mmx, /*short*/
    lxor_byte_mmx,  /*char*/
    lxor_byte_mmx, /*byte*/
    lxor_byte_mmx, /*uchar*/
    lxor_short_mmx,/*ushort*/
    lxor_int_mmx,  /*ulong*/
    lxor_int_mmx,  /*uint*/
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
    lxor_int_mmx,   /*logical*/
    lxor_int_mmx    /*fort_int*/
};

void MPIR_lxor_mmx(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_lxor_mmx[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}

__declspec(naked)
void lxor_int_mmx(int *inbuf,int*inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value

match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
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
align 4
ltop:

	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*2 integers.
		
	add	eax, 32				; We try to interleave some
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
void lxor_short_mmx(short *inbuf,short *inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value

match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
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
align 4
ltop:

	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that handles 4*4 shorts.
		
	add	eax, 32				; We try to interleave some
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
void lxor_byte_mmx(char *inbuf,char *inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value

match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
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
align 4
ltop:

	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that handles 4*8 bytes.
		
	add	eax, 32				; We try to interleave some
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
void land_error_func(void* d1,void*d2,unsigned int l) {
    MPIR_Op_errno = MPIR_ERRCLASS_TO_CODE(MPI_ERR_OP,MPIR_ERR_NOT_DEFINED);
    MPIR_ERROR(MPIR_COMM_WORLD,MPIR_Op_errno, "MPI_LAND" );
}

static void land_int_mmx(int *inbuf,int*inoutbuf,unsigned int len);
static void land_short_mmx(short *inbuf,short *inoutbuf,unsigned int len);
static void land_byte_mmx(signed char *inbuf,signed char *inoutbuf,unsigned int len);


static  const proto_func jmp_land_mmx[] = {
    land_int_mmx,   /*int*/
    land_error_func, /*float*/
    land_error_func,/*double*/
    land_error_func, /*complex*/
    land_int_mmx,   /*long*/
    land_short_mmx, /*short*/
    land_byte_mmx,  /*char*/
    land_byte_mmx, /*byte*/
    land_byte_mmx, /*uchar*/
    land_short_mmx,/*ushort*/
    land_int_mmx,  /*ulong*/
    land_int_mmx,  /*uint*/
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
    land_int_mmx,   /*logical*/
    land_int_mmx    /*fort_int*/
};

void MPIR_land_mmx(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_land_mmx[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}

__declspec(naked)
void land_int_mmx(int *inbuf,int*inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value

match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
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
align 4
ltop:

	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*2 integers.
		
	add	eax, 32				; We try to interleave some
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
void land_short_mmx(short *inbuf,short *inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value

match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
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
align 4
ltop:

	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that handles 4*4 shorts.
		
	add	eax, 32				; We try to interleave some
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
void land_byte_mmx(char *inbuf,char *inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value

match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
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
align 4
ltop:

	movq	mm1, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm0, QWORD PTR [eax]		; that handles 4*8 bytes.
		
	add	eax, 32				; We try to interleave some
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
void lor_error_func(void* d1,void*d2,unsigned int l) {
    MPIR_Op_errno = MPIR_ERRCLASS_TO_CODE(MPI_ERR_OP,MPIR_ERR_NOT_DEFINED);
    MPIR_ERROR(MPIR_COMM_WORLD,MPIR_Op_errno, "MPI_LOR" );
}

static void lor_int_mmx(int *inbuf,int*inoutbuf,unsigned int len);
static void lor_short_mmx(short *inbuf,short *inoutbuf,unsigned int len);
static void lor_byte_mmx(signed char *inbuf,signed char *inoutbuf,unsigned int len);


static  const proto_func jmp_lor_mmx[] = {
    lor_int_mmx,   /*int*/
    lor_error_func, /*float*/
    lor_error_func,/*double*/
    lor_error_func, /*complex*/
    lor_int_mmx,   /*long*/
    lor_short_mmx, /*short*/
    lor_byte_mmx,  /*char*/
    lor_byte_mmx, /*byte*/
    lor_byte_mmx, /*uchar*/
    lor_short_mmx,/*ushort*/
    lor_int_mmx,  /*ulong*/
    lor_int_mmx,  /*uint*/
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
    lor_int_mmx,   /*logical*/
    lor_int_mmx    /*fort_int*/
};

void MPIR_lor_mmx(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_lor_mmx[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}

__declspec(naked)
void lor_int_mmx(int *inbuf,int*inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value

match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
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
align 4
ltop:

	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that adds 4*2 integers.
		
	add	eax, 32				; We try to interleave some
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
void lor_short_mmx(short *inbuf,short *inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value

match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
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
align 4
ltop:

	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that handles 4*4 shorts.
		
	add	eax, 32				; We try to interleave some
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
void lor_byte_mmx(char *inbuf,char *inoutbuf,unsigned int len) {
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

	mov	ecx,DWORD PTR [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	match				;
	mov	edx,ecx				; yes, use smaller value

match:
	sub	ecx,edx				; calculate new length
	mov	DWORD PTR [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, DWORD PTR [esp+8]		; load inbuf
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
align 4
ltop:

	movq	mm0, QWORD PTR [ecx+eax]	; This is an unrolled loop
	movq	mm1, QWORD PTR [eax]		; that handles 4*8 bytes.
		
	add	eax, 32				; We try to interleave some
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

#endif /* _M_IX86 */

#endif
