
.section	.text,"ax"
.align 4
.globl  _smi_wc_memcpy
.type _smi_wc_memcpy,@function
.align 16
		# $Id$ 
		# fast remote SCI copy for systems with write-combining enabled
	
_smi_wc_memcpy:
                pushl %edi 
                pushl %esi 

                movl 12(%esp), %edi       # dest address
                movl 16(%esp), %esi       # src address
                movl 20(%esp), %ecx	  # total length
		pushl %eax		  # local dummy variable for flushing
	
                cmpl $31, %ecx
                jle cmp4

                # Align the destination to a 32 Byte boundary
                movl %edi, %eax
                andl $31, %eax
                je  cmp32
                movl %ecx, %edx		# calculate the number of bytes to copy
                movl $32, %ecx
                subl %eax, %ecx
                subl %ecx, %edx		# reduce total len
                rep		; movsb    # copy the leading bytes
                movl %edx, %ecx

		jmp cmp32
.align 16
copy32:		movl (%esi), %eax         # copy a cacheline
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

		xchg %eax, (%esp)	# flush the write-combine buffer 

	        subl $32, %ecx
                addl $32, %esi 
                addl $32, %edi

cmp32:          cmpl $31, %ecx
                jg  copy32
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
_smi_end_wc_copy:

.size	_smi_wc_memcpy, _smi_end_wc_copy-_smi_wc_memcpy
