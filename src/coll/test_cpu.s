.section        .text,"ax"
.align 4
.globl  MPIR_test_cpu
.type MPIR_test_cpu,@function
.align 16
                # $Id$ - tes
                # MPIR_test_cpu (X86_CPU_feature_t *f)
MPIR_test_cpu:
        pushl %edi
        pushl %esi
        pushl %ebx
        # f is located at %esp + 16
        movl 16(%esp), %ecx
        # first set all fields to 0 
        movl $0, (%ecx)
        movl $0, 4(%ecx)
        movl $0, 8(%ecx)
        movl $0, 12(%ecx)
        movl $0, 16(%ecx)
           # check whether CPUID is supported
            # (bit 21 of Eflags can be toggled)
            pushfl                      #save Eflags
            popl        %eax            #transfer Eflags into EAX
            movl        %eax,%edx       #save original Eflags
            xorl        $0x0200000,%eax #toggle bit 21
            pushl       %eax            #put new value of stack
            popfl                       #transfer new value to Eflags
            pushfl                      #save updated Eflags
            popl        %eax            #transfer Eflags to EAX
            xorl        %edx,%eax       #updated Eflags and original differ?
            jz          NO_3DNow        #no diff, bit 21 can\222t be toggled

            movl        $1,%eax
            cpuid
            movl         16(%esp), %ecx

            testl       $0x4000,%edx    # Is cmov bit (Bit 15 of EDX)
                                                      # in feature flags set?
            jz          nocmov
           movl        $1,(%ecx)
nocmov:
            testl       $0x0800000,%edx # Is IA MMX technology bit (Bit 23 of EDX)
                                                        # in feature flags set?
            jz          NoMMX
           movl        $1,4(%ecx)
NoMMX:      testl       $0x2000000,%edx # Is IA SSE technology bit (Bit 25
                                                                 # in feature flags set?
            jz          NoSSE 
            movl        $1,8(%ecx)
            testl       $0x4000000,%edx # Is IA SSE2 technology bit (Bit 26
                                                         # in feature flags set?
            jz          NoSSE           #
            movl        $1,12(%ecx)
NoSSE:
            #test whether extended function 80000001h is supported
            movl        $0x80000000,%eax #call extended function 80000000h
            cpuid                       #reports back highest supported ext. function

            cmpl        $0x80000000,%eax #supports functions > 80000000h?
            jbe         NO_3DNow        #no 3DNow! support, either

            #test if function 80000001h indicates 3DNow! support
            movl        $0x80000001,%eax #call extended function 80000001h
            cpuid                       #reports back extended feature flags
            testl       $0x80000000,%edx #bit 31 in extended features
            jz          NO_3DNow        #if set, 3DNow! is supported
           mov         16(%esp),%ecx
            movl        $1,16(%ecx)

NO_3DNow:
         popl %ebx
         popl %esi
         popl %edi
         ret

end_MPIR_test_cpu:

.size   MPIR_test_cpu, end_MPIR_test_cpu - MPIR_test_cpu
