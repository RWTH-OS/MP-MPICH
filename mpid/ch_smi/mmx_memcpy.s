
.section	.text,"ax"
.align 4
.globl  _mpid_smi_mmx_memcpy
.type _mpid_smi_mmx_memcpy,@function
.align 16
		# $Id: mmx_memcpy.s,v 1.8 2001/06/20 17:40:08 joachim Exp $ - fast remote SCI copy
	
_mpid_smi_mmx_memcpy:
                pushl %edi 
                pushl %esi 

                movl 12(%esp), %edi       
                movl 16(%esp), %esi       
                movl 20(%esp), %ecx 

                cmpl $7, %ecx
                jle dwords

                # Align the destination to a 8 Byte boundary
                movl %edi, %eax
                andl $7, %eax
                je  cmp64
                movl %ecx, %edx
                movl $8, %ecx
                subl %eax, %ecx
                subl %ecx, %edx
                rep		; movsb    # copy the leading bytes
                movl %edx, %ecx

		jmp cmp64
.align 16
stream:		movq (%esi), %mm0         # copy 2 cachelines
                movq 8(%esi), %mm1          
                movq 16(%esi), %mm2          
                movq 24(%esi), %mm3          
                movq 32(%esi), %mm4          
                movq 40(%esi), %mm5          
                movq 48(%esi), %mm6          
                movq 56(%esi), %mm7          

                movq %mm0, (%edi)            
                movq %mm1, 8(%edi)           
                movq %mm2, 16(%edi)          
                movq %mm3, 24(%edi)         
                movq %mm4, 32(%edi)         
                movq %mm5, 40(%edi)         
                movq %mm6, 48(%edi)         
                movq %mm7, 56(%edi)         

                addl $-64, %ecx
                addl $64, %esi 
                addl $64, %edi 
cmp64: 
                cmpl $63, %ecx
                jg  stream
                jmp cmp8

.align 4
copy8:		movq (%esi), %mm0
                movq %mm0, (%edi)         

                addl $-8, %ecx
                addl $8, %esi 
                addl $8, %edi 
cmp8: 
                cmpl $7, %ecx
                jg  copy8
                emms 
.align 4
dwords:		cmpl $3, %ecx
                jle bytes
                movsl
		add $-4, %ecx
.align 4
bytes:		testl %ecx, %ecx
                je  end      #     nothing remaining 
                rep;  movsb     
.align 4
end:            popl %esi 
                popl %edi 
                ret 
end_mmx_copy:

.size	_mpid_smi_mmx_memcpy, end_mmx_copy-_mpid_smi_mmx_memcpy
