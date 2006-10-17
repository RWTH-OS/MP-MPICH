
.section	.text,"ax"
.align 4
.globl  _mpid_smi_mmx_prefetchnta_memcpy
.type  _mpid_smi_mmx_prefetchnta_memcpy,@function
.align 16
		# $Id $ 
		# Fast remote SCI copy for systems with write-combining enabled.
		# This is a version using MMX instructions and prefetching
	        # (requires at least: Intel Pentium 3, AMD K6-2 )
	
_mpid_smi_mmx_prefetchnta_memcpy:
                push %edi 
                push %esi
		push %eax
		push %ebx
		push %ecx
		push %edx

                mov 28(%esp), %edi       # dest address
                mov 32(%esp), %esi       # src address
                mov 36(%esp), %ebx	  # total length

		# edx takes number of bytes%64
		mov %ebx, %edx
		and $63, %edx

		# ebx takes number of bytes/64
		shr     $6, %ebx
		jz      byteCopy


loop4k:		# flush 4k into temporary buffer

		push %esi 
		mov %ebx, %ecx
		# copy per block of 64 bytes. Must not override 64*64= 4096 bytes.
		cmp $64, %ecx
		jle     skipMiniMize
		mov     $64, %ecx
skipMiniMize:
		# eax takes the number of 64bytes packet for this block.
		mov %ecx, %eax

loopMemToL1: 
		prefetchnta 64(%esi) # Prefetch next loop, non-temporal 
		prefetchnta 96(%esi)

		movq  0(%esi), %mm1 # Read in source data 
		movq  8(%esi), %mm2 
		movq 16(%esi), %mm3
		movq 24(%esi), %mm4
		movq 32(%esi), %mm5
		movq 40(%esi), %mm6
		movq 48(%esi), %mm7
		movq 56(%esi), %mm0

		add $64, %esi
		dec %ecx 
		jnz loopMemToL1 

		pop %esi  # Now copy from L1 to system memory 
		mov %eax, %ecx

loopL1ToMem:
		movq  0(%esi), %mm1 # Read in source data from L1 
		movq  8(%esi), %mm2 
		movq 16(%esi), %mm3
		movq 24(%esi), %mm4
		movq 32(%esi), %mm5
		movq 40(%esi), %mm6
		movq 48(%esi), %mm7
		movq 56(%esi), %mm0

		movntq %mm1, 0(%edi)  # Non-temporal stores 
		movntq %mm2, 8(%edi)
		movntq %mm3, 16(%edi)
		movntq %mm4, 24(%edi)
		movntq %mm5, 32(%edi)
		movntq %mm6, 40(%edi)
		movntq %mm7, 48(%edi)
		movntq %mm0, 56(%edi)

		add $64, %esi
		add $64, %edi
		dec %ecx 
		jnz loopL1ToMem

		# Do next 4k block 
		sub %eax, %ebx
		jnz loop4k 

		emms

byteCopy:
		# Do last bytes with std cpy
		mov     %edx, %ecx
		rep movsb

ende:
		pop %edx
		pop %ecx
		pop %ebx
		pop %eax
		pop %esi 
                pop %edi 
                ret 
end_sse_copy:

.size	_mpid_smi_mmx_prefetchnta_memcpy, end_sse_copy-_mpid_smi_mmx_prefetchnta_memcpy
