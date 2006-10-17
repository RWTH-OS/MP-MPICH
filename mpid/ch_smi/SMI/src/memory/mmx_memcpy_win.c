/* $Id$ */

#if defined(WIN32)

#include <emmintrin.h>

#if defined(_M_IX86)

__declspec(naked) void  _smi_mmx_memcpy(char *dst, char *src, int n) {
_asm {

		push edi;
		push esi;

		mov edi, DWORD PTR [ESP+12];
        mov esi, DWORD PTR [ESP+16];
        mov ecx, [ESP+20];

		cmp ecx,7
		jle short dwords

		; Align the destination to a 8 Byte boundary
		mov eax,edi
		and eax,7
		je  short cmp64
		mov edx,ecx
		mov ecx,8
		sub ecx,eax
		sub edx,ecx
		rep movsb		; Copy the leading bytes
		mov ecx,edx
				
		jmp short cmp64
align 4
stream:	movq mm0, qword ptr [esi] ; /*copy 2 cachelines*/
		movq mm1, qword ptr [esi+8] ;
		movq mm2, qword ptr [esi+16] ;
		movq mm3, qword ptr [esi+24] ;
		movq mm4, qword ptr [esi+32] ;
		movq mm5, qword ptr [esi+40] ;
		movq mm6, qword ptr [esi+48] ;
		movq mm7, qword ptr [esi+56] ;
				
		movq qword ptr [edi]    ,mm0 ;
		movq qword ptr [edi+8]  ,mm1 ;
		movq qword ptr [edi+16] ,mm2 ;
		movq qword ptr [edi+24] ,mm3;
		movq qword ptr [edi+32] ,mm4;
		movq qword ptr [edi+40] ,mm5;
		movq qword ptr [edi+48] ,mm6;
		movq qword ptr [edi+56] ,mm7;
                
		add ecx,-64
		add esi,64;
        add edi,64;
cmp64:
		cmp ecx,63
		jg  short stream
		jmp cmp8
              
align 4
copy8:	movq mm0, qword ptr [esi] 
		movq qword ptr [edi] ,mm0 ;
		
		add ecx,-8
		add esi,8;
        add edi,8;
cmp8:
		cmp ecx,7
		jg  short copy8

		emms ;
align 4
		

dwords:	cmp ecx,3
		jle short bytes
		movsd
		add ecx,-4
align 4				
bytes:	test ecx,ecx
        je  short end;    /* nothing remaining */
		rep  movsb;
align 4
end:	pop esi;
		pop edi;
		ret;
}
}

#elif defined(_M_AMD64)

/* Remember, 64-bit assembly code for Windows cannot use the older MMX, 3D Now! and 
   x87 instruction extensions; they have been superseded by SSE/SSE2. */
/*void _smi_mmx_memcpy(void *dst, const void *src, unsigned int size)
{
	char*	a = (char*) src;
	char*	b = (char*) dst;
	size_t	j = 0;
	__m64	xmm[8];

	// Align the destination to a 8 byte boundary 
	for(; (j < size) && (((size_t) &b[j]) % 8 != 0); j++)
		b[j] = a[j];
		
	// copy 64 byte per loop 
	for (; (j+64) < size; j+=64) 
	{
		// load 64 Byte into xmm register
		xmm[0].m64_i64 = *((__int64*) (a+j));
		xmm[1].m64_i64 = *((__int64*) (a+(j+8)));
		xmm[2].m64_i64 = *((__int64*) (a+(j+16)));
		xmm[3].m64_i64 = *((__int64*) (a+(j+24)));
		xmm[4].m64_i64 = *((__int64*) (a+(j+32)));
		xmm[5].m64_i64 = *((__int64*) (a+(j+40)));
		xmm[6].m64_i64 = *((__int64*) (a+(j+48)));
		xmm[7].m64_i64 = *((__int64*) (a+(j+56)));

		// store 64 byte
		*((__int64*) (b+j)) = xmm[0].m64_i64;
		*((__int64*) (b+(j+8))) = xmm[1].m64_i64;
		*((__int64*) (b+(j+16))) = xmm[2].m64_i64;
		*((__int64*) (b+(j+24))) = xmm[3].m64_i64;
		*((__int64*) (b+(j+32))) = xmm[4].m64_i64;
		*((__int64*) (b+(j+40))) = xmm[5].m64_i64;
		*((__int64*) (b+(j+48))) = xmm[6].m64_i64;
		*((__int64*) (b+(j+56))) = xmm[7].m64_i64;
		
		_mm_empty();
	}
	
	// copy tail
	for(; j<size; j++)
		b[j] = a[j];
} */
#endif

#endif /* WIN32*/