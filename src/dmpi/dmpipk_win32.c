/*
*  $Id$
*
*/



#include "mpiimpl.h"
#include "mpidmpi.h"

#include "..\coll\x86_ops.h"

//#if CPU_ARCH_IS_X86
#ifdef NOTDEF

__declspec(naked)
void MPIR_Pack8_mmx(void *out, void *in, int c, int count, MPI_Aint stride) {
#define out esp+16
#define in out+4
#define c out+8
#define count out+12
#define stride out+16
	__asm {
		push	esi
			push	edi
			push	ebp

			mov	edi, DWORD PTR [out];
		mov	esi, DWORD PTR [in];

		mov	ebp, DWORD PTR [c];
		mov	edx, DWORD PTR [count];
		mov	eax, DWORD PTR [stride];
		neg	eax

			align 4
LTOP_OUTER:
		mov	ecx, ebp
			align 4
LTOP_INNER:

		movq	mm0,[esi]
		movq	[edi],mm0
			sub	esi, eax
			add	edi, 8
			dec	ecx
			jnz	LTOP_INNER

			lea	esi, [esi+eax+8]
			dec	edx
				jnz	SHORT LTOP_OUTER
				emms

LEND:
			pop	ebp
				pop	edi
				pop	esi
				ret
	}

#undef out
#undef in 
#undef c
#undef count
#undef stride
}

__declspec(naked)
void MPIR_Pack16_mmx(void *out, void *in, int c, int count, MPI_Aint stride) {
#define out esp+16
#define in out+4
#define c out+8
#define count out+12
#define stride out+16
	__asm {
		push	esi
			push	edi
			push	ebp

			mov	edi, DWORD PTR [out];
		mov	esi, DWORD PTR [in];

		mov	ebp, DWORD PTR [c];
		mov	edx, DWORD PTR [count];
		mov	eax, DWORD PTR [stride];
		neg	eax

			align 4
LTOP_OUTER:
		mov	ecx, ebp
			align 4
LTOP_INNER:

		movq	mm0,[esi]
		movq	mm1,[esi+8]
		movq	[edi],mm0
			movq	[edi+8],mm1
			sub	esi, eax
			add	edi, 16
			dec	ecx
			jnz	LTOP_INNER

			lea	esi, [esi+eax+16]
			dec	edx
				jnz	SHORT LTOP_OUTER
				emms

LEND:
			pop	ebp
				pop	edi
				pop	esi
				ret
	}

#undef out
#undef in 
#undef c
#undef count
#undef stride
}

__declspec(naked)
void MPIR_Pack24_mmx(void *out, void *in, int c, int count, MPI_Aint stride) {
#define out esp+16
#define in out+4
#define c out+8
#define count out+12
#define stride out+16
	__asm {
		push	esi
			push	edi
			push	ebp

			mov	edi, DWORD PTR [out];
		mov	esi, DWORD PTR [in];

		mov	ebp, DWORD PTR [c];
		mov	edx, DWORD PTR [count];
		mov	eax, DWORD PTR [stride];
		neg	eax

			align 4
LTOP_OUTER:
		mov	ecx, ebp
			align 4
LTOP_INNER:

		movq	mm0,[esi]
		movq	mm1,[esi+8]
		movq	mm2,[esi+16]
		movq	[edi],mm0
			movq	[edi+8],mm1
			movq	[edi+16],mm1
			sub	esi, eax
			add	edi, 24
			dec	ecx
			jnz	LTOP_INNER

			lea	esi, [esi+eax+24]
			dec	edx
				jnz	SHORT LTOP_OUTER
				emms

LEND:
			pop	ebp
				pop	edi
				pop	esi
				ret
	}

#undef out
#undef in 
#undef c
#undef count
#undef stride
}

__declspec(naked)
void MPIR_Pack32_mmx(void *out, void *in, int c, int count, MPI_Aint stride) {
#define out esp+16
#define in out+4
#define c out+8
#define count out+12
#define stride out+16
	__asm {
		push	esi
			push	edi
			push	ebp

			mov	edi, DWORD PTR [out];
		mov	esi, DWORD PTR [in];

		mov	ebp, DWORD PTR [c];
		mov	edx, DWORD PTR [count];
		mov	eax, DWORD PTR [stride];
		neg	eax

			align 4
LTOP_OUTER:
		mov	ecx, ebp
			align 4
LTOP_INNER:

		movq	mm0,[esi]
		movq	mm1,[esi+8]
		movq	mm2,[esi+16]
		movq	mm3,[esi+24]
		movq	[edi],mm0
			movq	[edi+8],mm1
			movq	[edi+16],mm2
			movq	[edi+24],mm3
			sub	esi, eax
			add	edi, 32
			dec	ecx
			jnz	LTOP_INNER

			lea	esi, [esi+eax+32]
			dec	edx
				jnz	SHORT LTOP_OUTER
				emms

LEND:
			pop	ebp
				pop	edi
				pop	esi
				ret
	}

#undef out
#undef in 
#undef c
#undef count
#undef stride
}



__declspec(naked)
void MPIR_Pack4(void *out, void *in, int c, int count, MPI_Aint stride) {
#define out esp+20
#define in out+4
#define c out+8
#define count out+12
#define stride out+16
	__asm {
		push	esi
			push	edi
			push	ebp
			push	ebx


			mov	edi, DWORD PTR [out];
		mov	esi, DWORD PTR [in];

		mov	ebp, DWORD PTR [c];
		mov	edx, DWORD PTR [count];
		mov	eax, DWORD PTR [stride];
		neg	eax

			align 4
LTOP_OUTER:
		mov	ecx, ebp
			align 4
LTOP_INNER:
		mov	ebx, DWORD PTR [esi]
		mov	[edi], ebx
			sub	esi, eax
			add	edi, 4
			dec	ecx
			jnz	LTOP_INNER

			lea	esi, [esi+eax+4]
			dec	edx
				jnz	SHORT LTOP_OUTER	

LEND:
			pop	ebx
				pop	ebp
				pop	edi
				pop	esi
				ret
	}

#undef out
#undef in 
#undef c
#undef count
#undef stride
}

__declspec(naked)
void MPIR_Pack8(void *out, void *in, int c, int count, MPI_Aint stride) {
#define out esp+20
#define in out+4
#define c out+8
#define count out+12
#define stride out+16
	__asm {
		push	esi
			push	edi
			push	ebp
			push	ebx


			mov	edi, DWORD PTR [out];
		mov	esi, DWORD PTR [in];

		mov	ebp, DWORD PTR [c];
		mov	edx, DWORD PTR [count];
		mov	eax, DWORD PTR [stride];
		neg	eax

			align 4
LTOP_OUTER:
		mov	ecx, ebp
			align 4
LTOP_INNER:
		mov	ebx, DWORD PTR [esi]
		mov	[edi], ebx
			mov	ebx, DWORD PTR [esi+4]
			mov	[edi+4], ebx
				sub	esi, eax
				add	edi, 8
				dec	ecx
				jnz	LTOP_INNER

				lea	esi, [esi+eax+8]
				dec	edx
					jnz	SHORT LTOP_OUTER	

LEND:
				pop	ebx
					pop	ebp
					pop	edi
					pop	esi
					ret
	}

#undef out
#undef in 
#undef c
#undef count
#undef stride
}

__declspec(naked)
void MPIR_Pack_arb(void *out, void *in, int c, int count, int blen, MPI_Aint stride) {
#define out esp+20
#define in out+4
#define c out+8
#define count out+12
#define blen out+16
#define stride out+20

	__asm {
		push	esi
			push	edi
			push	ebp
			push	ebx

			mov	edi, [out];
		mov	esi, [in]
		mov	ebp, [c];
		mov	ebx, [blen];
		mov	edx, [stride]
		sub	edx, ebx


			//	cmp	ebx, 31
			//	ja	short LTOP_OUTER

			mov	eax, ebp
			mov	ecx, ebx
			jmp	LDWORDS

			align 4
LTOP_OUTER:
		mov	eax, ebp
			align 4
LTOP_INNER:
		mov	ecx, ebx
			/*shr	ecx, 5
			jnz	short LLINE

			mov	ecx, ebx
			jmp	LDWORDS

			align 4
			LLINE:
			movq	mm0,[esi]
			movq	mm1,[esi+8]
			movq	mm2,[esi+16]
			movq	mm3,[esi+24]
			add	esi,32

			movq	[edi],mm0
			movq	[edi+8],mm1
			movq	[edi+16],mm2
			movq	[edi+24],mm3
			add	edi,32
			dec	ecx
			jnz	short LLINE

			mov	ecx, ebx
			and	ecx, 31
			*/
LDWORDS:
		shr	ecx,2
			rep movsd

LBYTES:
		mov	ecx, ebx
			and	ecx,3
			rep movsb

			add	esi, edx
			dec	eax
			jnz	SHORT LTOP_INNER

			sub	esi, edx
			mov	eax, DWORD PTR [count]
			dec	eax
				mov	DWORD PTR [count], eax
				jne	SHORT LTOP_OUTER
				//	emms

LEND:
			pop	ebx
				pop	ebp
				pop	edi
				pop	esi
				ret
	}

#undef out
#undef in 
#undef c
#undef count
#undef stride
#undef blen
}

void MPIR_Pack_Hvector( 
	char *buf, 
	int count, 
struct MPIR_DATATYPE *datatype, 
	char *outbuf )
{
	int count1 = datatype->count,           /* Number of blocks */
		blocklen = datatype->blocklen;      /* Number of elements in each block */
	MPI_Aint    stride   = datatype->stride;  /* Bytes between blocks */
	int extent = datatype->old_type->extent;  /* Extent of underlying type */
	int blen   = blocklen * extent;
	int c, i, j;

	if(!count || !count1) return;
	/* We can't use c = count * count1 since that moves the location of the second
	of the count elements after the stride from the first, rather than after the
	last element */
	c = count1;

	/* Handle the special case of 4 or 8 byte items, with appropriate 
	alignment.  We do this to avoid the cost of a memcpy call for each
	element.
	*/
	switch(blen) {
case 4: 
	MPIR_Pack4(outbuf,buf,c,count,stride);
	/*
	{
	register int *outb = (int *)outbuf, *inb = (int *)buf;
	stride = stride >> 2;
	for (j=0; j<count; j++) {
	for (i=0; i<c; i++) {
	outb[i] = *inb;
	inb    += stride;
	}
	inb  -= stride;
	inb  += 1;
	outb += c;
	}
	break;
	}*/
case 8:
	if(MPIR_X86Features.mmx)
		MPIR_Pack8_mmx(outbuf,buf,c,count,stride);
	else
		MPIR_Pack8(outbuf,buf,c,count,stride);
	break;
case 16:
	if(MPIR_X86Features.mmx)
		MPIR_Pack16_mmx(outbuf,buf,c,count,stride);
	else
		MPIR_Pack_arb(outbuf,buf,c,count,16,stride);
	break;

case 24:
	if(MPIR_X86Features.mmx)
		MPIR_Pack24_mmx(outbuf,buf,c,count,stride);
	else
		MPIR_Pack_arb(outbuf,buf,c,count,24,stride);
	break;

case 32:
	if(MPIR_X86Features.mmx)
		MPIR_Pack32_mmx(outbuf,buf,c,count,stride);
	else
		MPIR_Pack_arb(outbuf,buf,c,count,32,stride);
	break;

default:
	MPIR_Pack_arb(outbuf,buf,c,count,blen,stride);
	/*
	for (j=0; j<count; j++) {
	for (i=0; i<c; i++) {
	memcpy( outbuf, buf, blen );
	outbuf += blen; 
	buf    += stride;
	}
	buf -= stride;
	buf += blen;
	}
	*/
	}
}


__declspec(naked)
void MPIR_UnPack8_mmx(void *out, void *in, int c, int count, MPI_Aint stride) {
#define out esp+16
#define in out+4
#define c out+8
#define count out+12
#define stride out+16
	__asm {
		push	esi
			push	edi
			push	ebp

			mov	edi, DWORD PTR [out];
		mov	esi, DWORD PTR [in];
		mov	ebp, DWORD PTR [c];
		mov	edx, DWORD PTR [count];
		mov	eax, DWORD PTR [stride];
		neg	eax

			align 4
LTOP_OUTER:
		mov	ecx, ebp
			align 4
LTOP_INNER:
		movq	mm0, [esi]
		movq	[edi], mm0
			sub	edi, eax
			add	esi, 8
			dec	ecx
			jnz	LTOP_INNER

			lea	edi, [edi+eax+8]
			dec	edx
				jnz	SHORT LTOP_OUTER
				emms

LEND:
			pop	ebp
				pop	edi
				pop	esi
				ret
	}

#undef out
#undef in 
#undef c
#undef count
#undef stride
}

__declspec(naked)
void MPIR_UnPack16_mmx(void *out, void *in, int c, int count, MPI_Aint stride) {
#define out esp+16
#define in out+4
#define c out+8
#define count out+12
#define stride out+16
	__asm {
		push	esi
			push	edi
			push	ebp

			mov	edi, DWORD PTR [out];
		mov	esi, DWORD PTR [in];
		mov	ebp, DWORD PTR [c];
		mov	edx, DWORD PTR [count];
		mov	eax, DWORD PTR [stride];
		neg	eax

			align 4
LTOP_OUTER:
		mov	ecx, ebp
			align 4
LTOP_INNER:
		movq	mm0, [esi]
		movq	mm1, [esi+8]
		movq	[edi], mm0
			movq	[edi+8], mm1
			sub	edi, eax
			add	esi, 16
			dec	ecx
			jnz	LTOP_INNER

			lea	edi, [edi+eax+16]
			dec	edx
				jnz	SHORT LTOP_OUTER
				emms

LEND:
			pop	ebp
				pop	edi
				pop	esi
				ret
	}

#undef out
#undef in 
#undef c
#undef count
#undef stride
}

__declspec(naked)
void MPIR_UnPack24_mmx(void *out, void *in, int c, int count, MPI_Aint stride) {
#define out esp+16
#define in out+4
#define c out+8
#define count out+12
#define stride out+16
	__asm {
		push	esi
			push	edi
			push	ebp

			mov	edi, DWORD PTR [out];
		mov	esi, DWORD PTR [in];
		mov	ebp, DWORD PTR [c];
		mov	edx, DWORD PTR [count];
		mov	eax, DWORD PTR [stride];
		neg	eax

			align 4
LTOP_OUTER:
		mov	ecx, ebp
			align 4
LTOP_INNER:
		movq	mm0, [esi]
		movq	mm1, [esi+8]
		movq	mm2, [esi+16]
		movq	[edi], mm0
			movq	[edi+8], mm1
			movq	[edi+16], mm2
			sub	edi, eax
			add	esi, 24
			dec	ecx
			jnz	LTOP_INNER

			lea	edi, [edi+eax+24]
			dec	edx
				jnz	SHORT LTOP_OUTER
				emms

LEND:
			pop	ebp
				pop	edi
				pop	esi
				ret
	}

#undef out
#undef in 
#undef c
#undef count
#undef stride
}
__declspec(naked)
void MPIR_UnPack32_mmx(void *out, void *in, int c, int count, MPI_Aint stride) {
#define out esp+16
#define in out+4
#define c out+8
#define count out+12
#define stride out+16
	__asm {
		push	esi
			push	edi
			push	ebp

			mov	edi, DWORD PTR [out];
		mov	esi, DWORD PTR [in];
		mov	ebp, DWORD PTR [c];
		mov	edx, DWORD PTR [count];
		mov	eax, DWORD PTR [stride];
		neg	eax

			align 4
LTOP_OUTER:
		mov	ecx, ebp
			align 4
LTOP_INNER:
		movq	mm0, [esi]
		movq	mm1, [esi+8]
		movq	mm2, [esi+16]
		movq	mm3, [esi+24]
		movq	[edi], mm0
			movq	[edi+8], mm1
			movq	[edi+16], mm2
			movq	[edi+24], mm3
			sub	edi, eax
			add	esi, 32
			dec	ecx
			jnz	LTOP_INNER

			lea	edi, [edi+eax+32]
			dec	edx
				jnz	SHORT LTOP_OUTER
				emms

LEND:
			pop	ebp
				pop	edi
				pop	esi
				ret
	}

#undef out
#undef in 
#undef c
#undef count
#undef stride
}

__declspec(naked)
void MPIR_UnPack4(void *out, void *in, int c, int count, MPI_Aint stride) {
#define out esp+20
#define in out+4
#define c out+8
#define count out+12
#define stride out+16
	__asm {
		push	esi
			push	edi
			push	ebp
			push	ebx


			mov	edi, DWORD PTR [out];
		mov	esi, DWORD PTR [in];

		mov	ebp, DWORD PTR [c];
		mov	edx, DWORD PTR [count];
		mov	eax, DWORD PTR [stride];
		neg	eax

			align 4
LTOP_OUTER:
		mov	ecx,ebp
			align 4
LTOP_INNER:

		mov	ebx, DWORD PTR [esi]
		mov	[edi], ebx

			sub	edi, eax
			add	esi, 4
			dec	ecx
			jnz	LTOP_INNER

			lea	edi, [edi+eax+4]
			dec	edx
				jnz	SHORT LTOP_OUTER

LEND:
			pop	ebx
				pop	ebp
				pop	edi
				pop	esi
				ret
	}

#undef out
#undef in 
#undef c
#undef count
#undef stride
}
__declspec(naked)
void MPIR_UnPack8(void *out, void *in, int c, int count, MPI_Aint stride) {
#define out esp+20
#define in out+4
#define c out+8
#define count out+12
#define stride out+16
	__asm {
		push	esi
			push	edi
			push	ebp
			push	ebx


			mov	edi, DWORD PTR [out];
		mov	esi, DWORD PTR [in];

		mov	ebp, DWORD PTR [c];
		mov	edx, DWORD PTR [count];
		mov	eax, DWORD PTR [stride];
		neg	eax

			align 4
LTOP_OUTER:
		xor	ecx,ecx
			align 4
LTOP_INNER:

		mov	ebx, DWORD PTR [esi+ecx*8]
		mov	[edi], ebx
			mov	ebx, DWORD PTR [esi+ecx*8+4]
			mov	[edi+4], ebx

				sub	edi, eax
				inc	ecx
				cmp	ecx, ebp
				jb	LTOP_INNER

				lea	esi, [esi+ebp*8]
				lea	edi, [edi+eax+8]
				dec	edx
					jnz	SHORT LTOP_OUTER

LEND:
				pop	ebx
					pop	ebp
					pop	edi
					pop	esi
					ret
	}

#undef out
#undef in 
#undef c
#undef count
#undef stride
}

__declspec(naked)
void MPIR_UnPack_arb(void *out, void *in, int c, int count, int blen, MPI_Aint stride) {
#define out esp+20
#define in out+4
#define c out+8
#define count out+12
#define blen out+16
#define stride out+20

	__asm {
		push	esi
			push	edi
			push	ebp
			push	ebx

			mov	edx, [stride]
			mov	ebp, [c];
			mov	ebx, [blen];
			sub	edx, ebx
				mov	edi, [out];
			mov	esi, DWORD PTR [in]


			//	cmp	ebx, 31
			//	ja	LTOP_OUTER

			mov	eax, ebp
				mov	ecx, ebx
				jmp	LDWORDS

				align 4
LTOP_OUTER:
			mov	eax, ebp
				align 4
LTOP_INNER:
			mov	ecx, ebx
				/*	shr	ecx, 5
				jnz	LLINE

				mov	ecx, ebx
				jmp	LDWORDS

				align 4
				LLINE:
				movq	mm0,[esi]
				movq	mm1,[esi+8]
				movq	mm2,[esi+16]
				movq	mm3,[esi+24]
				add	esi,32

				movq	[edi],mm0
				movq	[edi+8],mm1
				movq	[edi+16],mm2
				movq	[edi+24],mm3
				add	edi,32
				dec	ecx
				jnz	LLINE

				mov	ecx, ebx
				and	ecx, 31
				*/
LDWORDS:
			shr	ecx,2
				rep movsd

LBYTES:
			mov	ecx, ebx
				and	ecx,3
				rep movsb

				add	edi, edx
				dec	eax
				jnz	SHORT LTOP_INNER

				sub	edi, edx
				mov	eax, DWORD PTR [count]
				dec	eax
					mov	DWORD PTR [count], eax
					jne	SHORT LTOP_OUTER
					emms

LEND:
				pop	ebx
					pop	ebp
					pop	edi
					pop	esi
					ret
	}

#undef out
#undef in 
#undef c
#undef count
#undef stride
#undef blen
}

void MPIR_UnPack_Hvector( 
						 char *inbuf, 
						 int count, 
struct MPIR_DATATYPE *datatype, 
	char *outbuf )
{
	int count1 = datatype->count,            /* Number of blocks */
		blocklen = datatype->blocklen;       /* Number of elements in each block */
	MPI_Aint    stride   = datatype->stride; /* Bytes between blocks */
	int extent = datatype->old_type->extent;  /* Extent of underlying type */
	int blen   = blocklen * extent;
	register int c, i;
	int          j;

	if(!count || !count1) return;
	/* We can't use c = count * count1 since that moves the location of the second
	of the count elements after the stride from the first, rather than after the
	last element */
	c = count1;
	switch(blen) {
case 4:
	MPIR_UnPack4(outbuf,inbuf,c,count,stride);
	/*{
	register int *outb = (int *)outbuf, *inb = (int *)inbuf;
	stride = stride >> 2;
	for (j=0; j<count; j++) {
	for (i=0; i<c; i++) {
	*outb = inb[i];
	outb  += stride;
	}
	outb -= stride;
	outb += 1;
	inb  += c;
	}

	break;
	}
	*/
case 8:
	if(MPIR_X86Features.mmx)
		MPIR_UnPack8_mmx(outbuf,inbuf,c,count,stride);
	else
		MPIR_UnPack8(outbuf,inbuf,c,count,stride);
	break;
case 16:
	if(MPIR_X86Features.mmx)
		MPIR_UnPack16_mmx(outbuf,inbuf,c,count,stride);
	else
		MPIR_UnPack_arb(outbuf,inbuf,c,count,16,stride);
	break;
case 24:
	if(MPIR_X86Features.mmx)
		MPIR_UnPack24_mmx(outbuf,inbuf,c,count,stride);
	else
		MPIR_UnPack_arb(outbuf,inbuf,c,count,24,stride);
	break;
case 32:
	if(MPIR_X86Features.mmx)
		MPIR_UnPack32_mmx(outbuf,inbuf,c,count,stride);
	else
		MPIR_UnPack_arb(outbuf,inbuf,c,count,32,stride);
	break;
default:
	MPIR_UnPack_arb(outbuf,inbuf,c,count,blen,stride);
	/*
	for (j=0; j<count; j++) {
	for (i=0; i<c; i++) {
	memcpy( outbuf, inbuf, blen );
	outbuf += stride;
	inbuf  += blen;
	}
	outbuf -= stride;
	outbuf += blen;
	}
	*/
	}
}

#else
/* 
This file contains the first pass at routines to pack and unpack datatypes
for the ADI.  THESE WILL CHANGE

In order to aid in debugging, it is possible to cause the datatype
pack/unpack actions to be written out.
*/

/* Pack for a send.  Eventually, this will need to handle the Heterogeneous 
case - XXXX.  

It also fails to detect an overrun error, or inadequate input data.
*/
void MPIR_Pack_Hvector( 
	char *buf, 
	int count, 
struct MPIR_DATATYPE *datatype, 
	char *outbuf )
{
	int count1 = datatype->count,           /* Number of blocks */
		blocklen = datatype->blocklen;      /* Number of elements in each block */
	MPI_Aint    stride   = datatype->stride;  /* Bytes between blocks */
	MPI_Aint extent = datatype->old_type->extent;  /* Extent of underlying type */
	MPI_Aint blen   = blocklen * extent;
	int c, i, j;

	/* We can't use c = count * count1 since that moves the location of the second
	of the count elements after the stride from the first, rather than after the
	last element */
	c = count1;

	/* Handle the special case of 4 or 8 byte items, with appropriate 
	alignment.  We do this to avoid the cost of a memcpy call for each
	element.
	*/
	if (blen == 4 && ((MPI_Aint)buf & 0x3) == 0 && (stride & 0x3) == 0 && 
		sizeof(int) == 4 && ((MPI_Aint)outbuf & 0x3) == 0) {
			register int *outb = (int *)outbuf, *inb = (int *)buf;
			stride = stride >> 2;
			for (j=0; j<count; j++) {
				for (i=0; i<c; i++) {
					outb[i] = *inb;
					inb    += stride;
				}
				inb  -= stride;
				inb  += 1;
				outb += c;
			}
		}
	else if (blen == 8 && ((MPI_Aint)buf & 0x7) == 0 && (stride & 0x7) == 0 && 
		sizeof(double) == 8 && ((MPI_Aint)outbuf & 0x7) == 0) {
			register double *outb = (double *)outbuf, *inb = (double *)buf;
			stride = stride >> 3;
			for (j=0; j<count; j++) {
				for (i=0; i<c; i++) {
					outb[i] = *inb;
					inb    += stride;
				}
				inb -= stride;
				inb += 1;
				outb += c;
			}
		}
	else {
		for (j=0; j<count; j++) {
			for (i=0; i<c; i++) {
				memcpy( outbuf, buf, blen );
				outbuf += blen; 
				buf    += stride;
			}
			buf -= stride;
			buf += blen;
		}
	}
}

void MPIR_UnPack_Hvector( 
						 char *inbuf, 
						 int count, 
struct MPIR_DATATYPE *datatype, 
	char *outbuf )
{
	int count1 = datatype->count,            /* Number of blocks */
		blocklen = datatype->blocklen;       /* Number of elements in each block */
	MPI_Aint    stride   = datatype->stride; /* Bytes between blocks */
	MPI_Aint extent = datatype->old_type->extent;  /* Extent of underlying type */
	MPI_Aint blen   = blocklen * extent;
	register int c, i;
	int          j;

	/* We can't use c = count * count1 since that moves the location of the second
	of the count elements after the stride from the first, rather than after the
	last element */
	c = count1;
	if (blen == 4 && ((MPI_Aint)inbuf & 0x3) == 0 && (stride & 0x3) == 0 && 
		sizeof(int) == 4 && ((MPI_Aint)outbuf & 0x3) == 0 ) {
			register int *outb = (int *)outbuf, *inb = (int *)inbuf;
			stride = stride >> 2;
			for (j=0; j<count; j++) {
				for (i=0; i<c; i++) {
					*outb = inb[i];
					outb  += stride;
				}
				outb -= stride;
				outb += 1;
				inb  += c;
			}
		}
	else if (blen == 8 && ((MPI_Aint)inbuf & 0x7) == 0 && (stride & 0x7) == 0 && 
		sizeof(double) == 8 && ((MPI_Aint)outbuf & 0x7) == 0) {
			register double *outb = (double *)outbuf, *inb = (double *)inbuf;
			stride = stride >> 3;
			for (j=0; j<count; j++) {
				for (i=0; i<c; i++) {
					*outb   = inb[i];
					outb    += stride;
				}
				outb -= stride;
				outb += 1;
				inb += c;
			}
		}
	else {
		for (j=0; j<count; j++) {
			for (i=0; i<c; i++) {
				memcpy( outbuf, inbuf, blen );
				outbuf += stride;
				inbuf  += blen;
			}
			outbuf -= stride;
			outbuf += blen;
		}
	}
}

#endif

/* Get the length needed for the Hvector as a contiguous lump */
int MPIR_HvectorLen( 
					int count,
struct MPIR_DATATYPE *datatype)
{
	return datatype->size * count;
}
