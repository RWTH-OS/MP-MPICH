/*; $Id$

; INET checksum algorithm taken from the Linux kernel 
;  (arch/i386/lib/checksum.S, optimized version for 686 (>= Pentium II) CPUs
;
; unsigned int MPID_SMI_csum_netdev_gen (const unsigned char * buff, int len, unsigned int sum)
*/

#if defined(WIN32) && defined(_M_IX86)

__declspec(naked)
unsigned int MPID_SMI_csum_netdev_gen(const unsigned char * buff, int len, unsigned int sum) {
__asm {
        
        push  esi
        push  ebx

        mov  eax, [esp+20]      ; Function arg: unsigned int sum
        mov  ecx, [esp+16]      ; Function arg: int len
        mov  esi, [esp+12]       ; Function arg: const unsigned char *buf
	
	

        test  esi,2
        jnz short l30
l10: 
        mov  edx,ecx
        mov  ebx,ecx
        and  ebx,07ch
        shr  ecx,7
        add  esi,ebx
        shr  ebx,2
        neg  ebx
		lea ebx,DWORD PTR [ebx+ebx*2+l45]
        test  esi,esi
        jmp ebx

        ; Handle 2-byte-aligned regions
l20:	add  ax, WORD PTR [esi]
	lea esi,[esi+2]
        adc  eax,0
        jmp l10

l30:	sub  ecx,2
        ja short l20
        je short l32
        movzx  ebx,BYTE PTR [esi]   ; csumming 1 byte, 2-aligned
        add  eax,ebx
        adc  eax,0
        jmp short l80
l32: 
        add  ax, WORD PTR [esi]          ; csumming 2 bytes, 2-aligned
        adc  eax,0
        jmp short l80

l40: 
        add  eax, [esi-128]
        adc  eax, [esi-124]
        adc  eax, [esi-120]
        adc  eax, [esi-116]
        adc  eax, [esi-112]
        adc  eax, [esi-108]
        adc  eax, [esi-104]
        adc  eax, [esi-100]
        adc  eax, [esi-96]
        adc  eax, [esi-92]
        adc  eax, [esi-88]
        adc  eax, [esi-84]
        adc  eax, [esi-80]
        adc  eax, [esi-76]
        adc  eax, [esi-72]
        adc  eax, [esi-68]
        adc  eax, [esi-64]
        adc  eax, [esi-60]
        adc  eax, [esi-56]
        adc  eax, [esi-52]
        adc  eax, [esi-48]
        adc  eax, [esi-44]
        adc  eax, [esi-40]
        adc  eax, [esi-36]
        adc  eax, [esi-32]
        adc  eax, [esi-28]
        adc  eax, [esi-24]
        adc  eax, [esi-20]
        adc  eax, [esi-16]
        adc  eax, [esi-12]
        adc  eax, [esi-8]
        adc  eax, [esi-4]
l45: 
	lea esi,[esi+128]
        adc  eax,0
	dec ecx
        jge l40
        mov  ecx,edx
l50:   and  ecx,3
        jz short l80

        ; Handle the last 1-3 bytes without jumping
        not  ecx                ; 1->2, 2->1, 3->0, higher bits are masked
        mov  ebx,0ffffffh       ; by the shll and shrl instructions
        shl  ecx,3
        shr  ebx,cl
        and  ebx, [esi-128]     ; esi is 4-aligned so should be ok
        add  eax,ebx
        adc  eax,0

l80: 
        pop  ebx
        pop  esi        
        ret
}
}

#endif