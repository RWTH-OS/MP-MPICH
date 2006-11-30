			# $Id$
			# determine command extensions of x86 CPUs	
			#  call:	CPU_detect(int* MMX,int* SSE,int* _3DNow)
.686: 
.XMM: 
.K3D: 
#.MODEL flat,C

.text

.align 4
.globl  CPU_detect
.type	CPU_detect,@function

.equ _MMX$, 8
.equ _SSE$, 12 
.equ __3DNow$, 16

CPU_detect:
               pushl   %ebp
               movl    %esp,%ebp

                movl _MMX$(%ebp), %ecx
                movl $0, (%ecx)
                movl _SSE$(%ebp), %ecx
                movl $0, (%ecx)
                movl __3DNow$(%ebp), %ecx
                movl $0, (%ecx)

                ## check whether CPUID is supported
                ## (bit 21 of Eflags can be toggled)
                pushfl                                  #save Eflags
                popl            %eax                            #transfer Eflags into EAX
                movl            %eax, %edx              #save original Eflags
                xorl            $0x0200000, %eax #toggle bit 21
                pushl   %eax                            #put new value of stack
                popfl                                   #transfer new value to Eflags
                pushfl                                  #save updated Eflags
                popl            %eax                            #transfer Eflags to EAX
                xorl            %edx, %eax              #updated Eflags and original differ?
                jz              NO_3DNow                #no diff, bit 21 canÆt be toggled

                movl            $1, %eax
                cpuid
                testl   $0x0800000, %edx # Is IA MMX technology bit (Bit 23 of EDX)
                                                                # in feature flags set?
                jz              NoMMX      #
                movl            _MMX$(%ebx), %ecx
                movl            $1, (%ecx)
NoMMX:  testl   $0x2000000, %edx
                jz              NoSSE #
                movl            _SSE$(%ebp), %ecx
                movl            $1, (%ecx)
NoSSE:  
                ##test whether extended function 80000001h is supported
                movl            $0x80000000, %eax #call extended function 80000000h
                cpuid                                   #reports back highest supported ext.function
                cmpl            $0x80000000, %eax #supports functions > 80000000h?
                jbe             NO_3DNow                #no 3DNow! support, either

                ##test if function 80000001h indicates 3DNow! support
                movl            $0x80000001, %eax #call extended function 80000001h
                cpuid                                   #reports back extended feature flags
                testl   $0x80000000, %edx #bit 31 in extended features
                jz              NO_3DNow                #if set, 3DNow! is supported
                movl            __3DNow$(%ebp), %ecx
                movl            $1, (%ecx)
NO_3DNow: 
	        popl %ebp
                ret
EndFunc:

.size	CPU_detect,EndFunc-CPU_detect




