# $Id$
	
# INET checksum algorithm taken from the Linux kernel 
#  (arch/i386/lib/checksum.S, optimized version for 686 (>= Pentium II) CPUs
#
# unsigned int MPID_SMI_csum_netdev_gen (const unsigned char * buff, int len, unsigned int sum)
		
.section	.text,"ax"
.align 4
.globl MPID_SMI_csum_netdev_gen
.type MPID_SMI_csum_netdev_gen,@function
.align 16
			
MPID_SMI_csum_netdev_gen:
	
    
	pushl %esi
	pushl %ebx
		
	movl 20(%esp),%eax	# Function arg: unsigned int sum
	movl 16(%esp),%ecx	# Function arg: int len
	movl 12(%esp),%esi	# Function arg:	const unsigned char *buf

	testl $2, %esi         
	jnz 30f                 
10:
	movl %ecx, %edx
	movl %ecx, %ebx
	andl $0x7c, %ebx
	shrl $7, %ecx
	addl %ebx,%esi
	shrl $2, %ebx  
	negl %ebx
	lea 45f(%ebx,%ebx,2), %ebx
	testl %esi, %esi
	jmp *%ebx

	# Handle 2-byte-aligned regions
20:	addw (%esi), %ax
	lea 2(%esi), %esi
	adcl $0, %eax
	jmp 10b

30:	subl $2, %ecx          
	ja 20b                 
	je 32f
	movzbl (%esi),%ebx	# csumming 1 byte, 2-aligned
	addl %ebx, %eax
	adcl $0, %eax
	jmp 80f
32:
	addw (%esi), %ax	# csumming 2 bytes, 2-aligned
	adcl $0, %eax
	jmp 80f

40: 
	addl -128(%esi), %eax
	adcl -124(%esi), %eax
	adcl -120(%esi), %eax
	adcl -116(%esi), %eax   
	adcl -112(%esi), %eax   
	adcl -108(%esi), %eax
	adcl -104(%esi), %eax
	adcl -100(%esi), %eax
	adcl -96(%esi), %eax
	adcl -92(%esi), %eax
	adcl -88(%esi), %eax
	adcl -84(%esi), %eax
	adcl -80(%esi), %eax
	adcl -76(%esi), %eax
	adcl -72(%esi), %eax
	adcl -68(%esi), %eax
	adcl -64(%esi), %eax     
	adcl -60(%esi), %eax     
	adcl -56(%esi), %eax     
	adcl -52(%esi), %eax   
	adcl -48(%esi), %eax   
	adcl -44(%esi), %eax
	adcl -40(%esi), %eax
	adcl -36(%esi), %eax
	adcl -32(%esi), %eax
	adcl -28(%esi), %eax
	adcl -24(%esi), %eax
	adcl -20(%esi), %eax
	adcl -16(%esi), %eax
	adcl -12(%esi), %eax
	adcl -8(%esi), %eax
	adcl -4(%esi), %eax
45:
	lea 128(%esi), %esi
	adcl $0, %eax
	dec %ecx
	jge 40b
	movl %edx, %ecx
50:	andl $3, %ecx
	jz 80f

	# Handle the last 1-3 bytes without jumping
	notl %ecx		# 1->2, 2->1, 3->0, higher bits are masked
	movl $0xffffff,%ebx	# by the shll and shrl instructions
	shll $3,%ecx
	shrl %cl,%ebx
	andl -128(%esi),%ebx	# esi is 4-aligned so should be ok
	addl %ebx,%eax
	adcl $0,%eax

80:
	popl %ebx
	popl %esi 
	ret
MPID_SMI_csum_netdev_gen_end:
	
.size	MPID_SMI_csum_netdev_gen, MPID_SMI_csum_netdev_gen_end - MPID_SMI_csum_netdev_gen
