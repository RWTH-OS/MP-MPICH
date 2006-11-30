
.section	.text,"ax"
.align 4
.globl  _mpid_smi_sse64_memcpy
.type  _mpid_smi_sse64_memcpy,@function
.align 16
		# $Id$ 
		# Fast remote SCI copy for systems with write-combining enabled.
		# This is the version using SSE instructions to copy 128 Byte blocks,
		# and flushes after 64 Byte.
	        # (required for the Intel Pentium4/Xeon and AMD Athlon)
	
_mpid_smi_sse64_memcpy:
                pushl %edi 
                pushl %esi 

                movl 12(%esp), %edi       # dest address
                movl 16(%esp), %esi       # src address
                movl 20(%esp), %ecx	  # total length
	
                cmpl $127, %ecx
                jle cmp4

                # Align the destination to a 64 Byte boundary
                movl %edi, %eax
                andl $63, %eax
                je  cmp128
                movl %ecx, %edx		# calculate the number of bytes to copy
                movl $64, %ecx
                subl %eax, %ecx
                subl %ecx, %edx		# reduce total len
                rep		; movsb    # copy the leading bytes
                movl %edx, %ecx

		jmp cmp128
.align 16
copy128:        movups   0(%esi) ,  %xmm0
		movups  16(%esi) ,  %xmm1
		movups  32(%esi) ,  %xmm2
                movups  48(%esi) ,  %xmm3
                movups  64(%esi) ,  %xmm4
                movups  80(%esi) ,  %xmm5
                movups  96(%esi) ,  %xmm6
                movups  112(%esi),  %xmm7
                movntps %xmm0 ,  0(%edi)
                movntps %xmm1 ,  16(%edi)
                movntps %xmm2 ,  32(%edi)
                movntps %xmm3 ,  48(%edi)
                sfence                      # flush WC buffers
                movntps %xmm4 ,  64(%edi)
                movntps %xmm5 ,  80(%edi)
                movntps %xmm6 ,  96(%edi)
                movntps %xmm7 ,  112(%edi)
                sfence                      # flush WC buffers

	        subl $128, %ecx
                addl $128, %esi 
                addl $128, %edi

cmp128:         cmpl $127, %ecx
                jg  copy128
		emms			# clean up SSE/FP state
		jmp cmp4

.align 4
copy4:		movl (%esi), %eax
                movl %eax, (%edi)         

                subl $4, %ecx
                addl $4, %esi 
                addl $4, %edi 

                movl %edi, %eax
                andl $31, %eax
                je  cmp4
		sfence

cmp4:           cmpl $3, %ecx
                jg  copy4
.align 4
bytes:		testl %ecx, %ecx
                je  end			# nothing remaining 
                rep;  movsb     
.align 4
end:		
		sfence			# flush the write-combine buffer once again

		popl %esi 
                popl %edi 
                ret 
end_sse_copy:

.size	_mpid_smi_sse64_memcpy, end_sse_copy-_mpid_smi_sse64_memcpy
