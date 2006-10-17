
/*$Id$ 

     fast remote SCI copy for systems with write-combining enabled (WIN32 version)
     wc32 or wc64 must be chosen depending on the size of the write-combine buffer in 
     the CPU (32 byte in Pentium <= 3, 64 byte in Pentium 4)
*/

#if defined(WIN32) && defined(_M_IX86)

#define DST  12
#define SRC  16
#define LEN  20

__declspec(naked) void _mpid_smi_wc32_memcpy(void *dest, void *src, unsigned long size) {
    __asm {
                push edi 
                push esi 

                mov edi,[esp+DST]	; dest address
                mov esi,[esp+SRC]	; src address
                mov ecx,[esp+LEN]	; total length
		push eax		; local dummy variable for flushing
	
                cmp  ecx,31
                jle copy4

                ; Align the destination to a 32 Byte boundary
                mov eax,edi
                and eax,31
                je  cmp32

                mov edx,ecx		; calculate the number of bytes to copy
                mov ecx,32
                sub ecx,eax
                sub edx,ecx		; reduce total len
                rep movsb		; copy the leading bytes
                mov ecx,edx
		
		jmp cmp32

.align 4
copy32:		mov eax,[esi]		; copy a cacheline
		mov [edi],eax
		mov eax,[esi+4]		
		mov [edi+4],eax
		mov eax,[esi+8]		
		mov [edi+8],eax
		mov eax,[esi+12]	
		mov [edi+12],eax
		mov eax,[esi+16]	
		mov [edi+16],eax
		mov eax,[esi+20]
		mov [edi+20],eax
		mov eax,[esi+24]
		mov [edi+24],eax
		mov eax,[esi+28]
		mov [edi+28],eax

		xchg [esp],eax		; flush the write-combine buffer 

	        sub ecx,32
                add esi,32 
                add edi,32

cmp32:          cmp ecx,31
                jg  copy32

.align 4
copy4:		cmp ecx,3
		jle bytes
		mov edx,ecx		; save ecx
		shr ecx,2		; ecx /= 4
		rep movsd
		mov ecx,edx		; get old ecx
		and ecx,3		; ecx %= 4
		
.align 4
bytes:		test ecx, ecx
                je  end			; nothing remaining 
                rep movsb     
.align 4
end:		
		xchg  [esp],eax		; flush the write-combine buffer once again

		pop eax
		pop esi 
                pop edi 
                ret 
    }
}

__declspec(naked) void _mpid_smi_wc64_memcpy(void *dest, void *src, unsigned long size) {
    __asm {
                push edi 
                push esi 

                mov edi,[esp+DST]	; dest address
                mov esi,[esp+SRC]	; src address
                mov ecx,[esp+LEN]	; total length
		push eax		; local dummy variable for flushing
	
                cmp  ecx,63
                jle copy4

                ; Align the destination to a 32 Byte boundary
                mov eax,edi
                and eax,63
                je  cmp64

                mov edx,ecx		; calculate the number of bytes to copy
                mov ecx,64
                sub ecx,eax
                sub edx,ecx		; reduce total len
                rep movsb		; copy the leading bytes
                mov ecx,edx
		
		jmp cmp64

.align 4
copy64:		mov eax,[esi]		; copy a cacheline
		mov [edi],eax
		mov eax,[esi+4]		
		mov [edi+4],eax
		mov eax,[esi+8]		
		mov [edi+8],eax
		mov eax,[esi+12]	
		mov [edi+12],eax
		mov eax,[esi+16]	
		mov [edi+16],eax
		mov eax,[esi+20]
		mov [edi+20],eax
		mov eax,[esi+24]
		mov [edi+24],eax
		mov eax,[esi+28]
		mov [edi+28],eax
		mov eax,[esi+32]
		mov [edi+32],eax
		mov eax,[esi+36]
		mov [edi+36],eax
		mov eax,[esi+40]
		mov [edi+40],eax
		mov eax,[esi+44]
		mov [edi+44],eax
		mov eax,[esi+48]
		mov [edi+48],eax
		mov eax,[esi+52]
		mov [edi+52],eax
		mov eax,[esi+56]
		mov [edi+56],eax
		mov eax,[esi+60]
		mov [edi+60],eax

		xchg [esp],eax		; flush the write-combine buffer 

	        sub ecx,64
                add esi,64 
                add edi,64

cmp64:          cmp ecx,63
                jg  copy64

.align 4
copy4:		cmp ecx,3
		jle bytes
		mov edx,ecx		; save ecx
		shr ecx,2		; ecx /= 4
		rep movsd
		mov ecx,edx		; get old ecx
		and ecx,3		; ecx %= 4
		
.align 4
bytes:		test ecx, ecx
                je  end			; nothing remaining 
                rep movsb     
.align 4
end:		
		xchg  [esp],eax		; flush the write-combine buffer once again

		pop eax
		pop esi 
                pop edi 
                ret 
    }
}


__declspec(naked) void _mpid_smi_mmx32_memcpy(void *dest, void *src, unsigned long size) {
    __asm {
                push edi 
                push esi 

                mov edi,[esp+DST]	; dest address
                mov esi,[esp+SRC]	; src address
                mov ecx,[esp+LEN]	; total length
	
                cmp  ecx,31
                jle copy4

                ; Align the destination to a 32 Byte boundary
                mov eax,edi
                and eax,31
                je  cmp32

                mov edx,ecx		; calculate the number of bytes to copy
                mov ecx,32
                sub ecx,eax
                sub edx,ecx		; reduce total len
                rep movsb		; copy the leading bytes
                mov ecx,edx
		
		jmp cmp32

.align 4
copy32:		movq mm0,[esi]		; copy a 32-byte cacheline
		movq mm1,[esi+8]
		movq mm2,[esi+16]
		movq mm3,[esi+24]
		movq [edi], mm0
		movq [edi+8], mm1
		movq [edi+16], mm2
		movq [edi+24], mm3

		sfence	                ; flush the write-combine buffer 
		emms

	        sub ecx,32
                add esi,32 
                add edi,32

cmp32:          cmp ecx,31
                jg  copy32

.align 4
copy4:		cmp ecx,3
		jle bytes
		mov edx,ecx		; save ecx
		shr ecx,2		; ecx /= 4
		rep movsd
		mov ecx,edx		; get old ecx
		and ecx,3		; ecx %= 4
		
.align 4
bytes:		test ecx, ecx
                je  end			; nothing remaining 
                rep movsb     
.align 4
end:		
		sfence		        ; flush the write-combine buffer once again

		pop esi 
                pop edi 
                ret 
    }
}


__declspec(naked) void _mpid_smi_mmx64_memcpy(void *dest, void *src, unsigned long size) {
    __asm {
                push edi 
                push esi 

                mov edi,[esp+DST]	; dest address
                mov esi,[esp+SRC]	; src address
                mov ecx,[esp+LEN]	; total length
	
                cmp  ecx,63
                jle copy4

                ; Align the destination to a 32 Byte boundary
                mov eax,edi
                and eax,63
                je  cmp64

                mov edx,ecx		; calculate the number of bytes to copy
                mov ecx,64
                sub ecx,eax
                sub edx,ecx		; reduce total len
                rep movsb		; copy the leading bytes
                mov ecx,edx
		
		jmp cmp64

.align 4
copy64:		movq mm0,[esi]		; copy a cacheline
		movq mm1,[esi+8]
		movq mm2,[esi+16]
		movq mm3,[esi+24]
		movq mm4,[esi+32]
		movq mm5,[esi+40]
		movq mm6,[esi+48]
		movq mm7,[esi+56]
		movq [edi], mm0
		movq [edi+8], mm1
		movq [edi+16], mm2
		movq [edi+24], mm3
		movq [edi+32], mm4
		movq [edi+40], mm5
		movq [edi+48], mm6
		movq [edi+56], mm7

		sfence	                ; flush the write-combine buffer 
		emms

	        sub ecx,64
                add esi,64 
                add edi,64

cmp64:          cmp ecx,63
                jg  copy64

.align 4
copy4:		cmp ecx,3
		jle bytes
		mov edx,ecx		; save ecx
		shr ecx,2		; ecx /= 4
		rep movsd
		mov ecx,edx		; get old ecx
		and ecx,3		; ecx %= 4
		
.align 4
bytes:		test ecx, ecx
                je  end			; nothing remaining 
                rep movsb     
.align 4
end:		
		sfence		        ; flush the write-combine buffer once again

		pop esi 
                pop edi 
                ret 
    }
}

#endif