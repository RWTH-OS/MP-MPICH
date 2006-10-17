
	.section	.text,"ax"
	.align	4

	.globl	getticks
	.type	getticks,@function
	.align	16
getticks:
	pushl	%ebp
	movl	%esp,%ebp
.L14:

	movl	8(%ebp), %esi
	rdtsc
	movl	%eax, 0(%esi)
	movl	%edx, 4(%esi)
.L13:
	movl	%ebp,%esp
	popl	%ebp
	ret
	.size	getticks,.-getticks

