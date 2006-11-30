
.section	.text,"ax"
.align 4
.globl  _mpid_smi_wc64_memcpy
.type _mpid_smi_wc64_memcpy,@function
.align 16
		# $Id$ 
		# Fast remote SCI copy for systems with write-combining enabled.
		# This is the version for 64-byte write-combine buffer in the CPU,
		# as required for the Pentium4.
	
_mpid_smi_wc64_memcpy:
                pushl %edi 
                pushl %esi 

                movl 12(%esp), %edi       # dest address
                movl 16(%esp), %esi       # src address
                movl 20(%esp), %ecx	  # total length
		pushl %eax		  # local dummy variable for flushing
	
                cmpl $63, %ecx
                jle cmp4

                # Align the destination to a 64 Byte boundary
                movl %edi, %eax
                andl $63, %eax
                je  cmp64
                movl %ecx, %edx		# calculate the number of bytes to copy
                movl $64, %ecx
                subl %eax, %ecx
                subl %ecx, %edx		# reduce total len
                rep		; movsb    # copy the leading bytes
                movl %edx, %ecx

		jmp cmp64
.align 16
copy64:		movl (%esi), %eax         # copy a cacheline
		movl %eax, (%edi)
		movl 4(%esi), %eax         
		movl %eax, 4(%edi)
		movl 8(%esi), %eax         
		movl %eax, 8(%edi)
		movl 12(%esi), %eax         
		movl %eax, 12(%edi)
		movl 16(%esi), %eax         
		movl %eax, 16(%edi)
		movl 20(%esi), %eax         
		movl %eax, 20(%edi)
		movl 24(%esi), %eax         
		movl %eax, 24(%edi)
		movl 28(%esi), %eax         
		movl %eax, 28(%edi)
		movl 32(%esi), %eax         
		movl %eax, 32(%edi)
		movl 36(%esi), %eax         
		movl %eax, 36(%edi)
		movl 40(%esi), %eax         
		movl %eax, 40(%edi)
		movl 44(%esi), %eax         
		movl %eax, 44(%edi)
		movl 48(%esi), %eax         
		movl %eax, 48(%edi)
		movl 52(%esi), %eax         
		movl %eax, 52(%edi)
		movl 56(%esi), %eax         
		movl %eax, 56(%edi)
		movl 60(%esi), %eax         
		movl %eax, 60(%edi)

		xchg %eax, (%esp)	# flush the write-combine buffer 

	        subl $64, %ecx
                addl $64, %esi 
                addl $64, %edi

cmp64:          cmpl $63, %ecx
                jg  copy64
                jmp cmp4

.align 4
copy4:		movl (%esi), %eax
                movl %eax, (%edi)         

                subl $4, %ecx
                addl $4, %esi 
                addl $4, %edi 

cmp4:           cmpl $3, %ecx
                jg  copy4
.align 4
bytes:		testl %ecx, %ecx
                je  end			# nothing remaining 
                rep;  movsb     
.align 4
end:		
		xchg %eax, (%esp)	# flush the write-combine buffer once again

		popl %eax
		popl %esi 
                popl %edi 
                ret 
end_wc64_copy:

.size	_mpid_smi_wc64_memcpy, end_wc64_copy-_mpid_smi_wc64_memcpy
