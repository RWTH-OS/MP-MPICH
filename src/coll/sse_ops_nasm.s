; assembler code taken from win32 version sse_ops.c Version 1.5

oneInt:		dd	1,1
oneShort:	dw	1,1,1,1
oneByte:	db	1,1,1,1,1,1,1,1


%define ALIGNEMENT 32

%define LN_DOUBLE 3
%define LN_INT 2
%define LN_FLOAT 2
%define LN_SHORT 1


	
; void add_int_sse(int *inbuf,int*inoutbuf,unsigned int len) 
global add_int_sse
add_int_sse:	
	push	ebx

	mov	ecx, [esp+16]		; Load length
	mov	eax, [esp+12]		; load inoutbuf
	
	mov	ebx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_INT			; to get .aligned addresses
	
	mov	eax,ebx				; restore address of inoutbuf
	prefetchnta [eax+32]
	
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	[esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx, [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,1				; divide by 2
	jz	.remain
	
align 4	
.al_top:	
	movq	mm1, [eax]		; Add integers
	movq	mm0, [ecx+eax]	; 2 at a time
	
	paddd	mm0, mm1			;
	add	eax,8
	dec	edx
	movq	[eax-8], mm0		;
	jne	.al_top
.remain:
	test	bl,1
	jz	.startit

	mov	edx, [ecx+eax]		; Add .remaining
	add	edx, [eax]		; integer
	mov	[eax],edx
	add	eax,4
	jmp	.startit

.aligned:	
	mov	eax, ebx
	mov	ecx, [esp+8]		; load inbuf
	sub	ecx, eax			;
.startit:	
	mov	edx, [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT .unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]			; Prefetch the following cacheline
	prefetchnta [ecx+eax+32]		; 
align 4
.ltop:
	movq	mm0, [ecx+eax]	; This is an unrolled loop
	movq	mm1, [eax]		; that adds 4*2 integers.
	add	eax, 32				; 
	prefetchnta [eax+32]			; 
	prefetchnta [ecx+eax+32]		; 
	
	
	
	movq	mm2, [ecx+eax-24]	; 
	movq	mm3, [eax-24]		;	  
	paddd	mm0, mm1			;
	movq	[eax-32], mm0		;
	
	

	movq	mm0, [ecx+eax-16]	;
	movq	mm1, [eax-16]		;
	paddd	mm2, mm3			;
	movq	[eax-24], mm2
	
	

	movq	mm2, [ecx+eax-8]	;
	movq	mm3, [eax-8]		;
	paddd	mm0, mm1			;
	movq	[eax-16], mm0		;
	
	dec	edx				;
	paddd	mm2, mm3			;
	movq	[eax-8], mm2		;
	
	jne	SHORT .ltop			;
.unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,7				; calculate .remaining 
	shr	edx,1				; number of integers
	jz	.norest				; 
.lrest:
	movq	mm0, [ecx+eax]	; add two ints
	movq	mm1, [eax]		;
	add	eax,8
	paddd	mm0, mm1			;
	movq	[eax-8], mm0		;
	dec	edx				;
	jne	.lrest				;
.norest:
	
	emms					;
	test	bl,1				;
	jz	.end				; No--> we are finished
	mov	edx, [eax]		; Add the .remaining integer
	add	edx, [ecx+eax]		;
	mov	[eax],edx			;
.end:
	pop	ebx
	ret	



; void add_short_sse(short *inbuf,short* inoutbuf,unsigned int len) 
global add_short_sse
add_short_sse:
	push	ebx
	mov	eax,  [esp+12]		; load inoutbuf	
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT			; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	.remain
		
.al_top:	
	movq	mm1,  [eax]		; Add shorts
	movq	mm0,  [ecx+eax]	; 4 at a time
	add	eax,8
	paddw	mm0, mm1			;
	movq	 [eax-8], mm0		;
	
	dec	edx
	jne	.al_top
.remain:
	and	bl,3
	jz	.startit
.al_rest:
	mov	dx, [ecx+eax]		; Add .remaining
	add	dx, [eax]		; shorts
	mov	 [eax],dx
	add	eax,2
	dec	bl
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:

	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 4				; len /= 16
	jz	SHORT .unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]			; Prefetch the following cacheline
	prefetchnta [ecx+eax+32]		; 	
align 4
.ltop:
	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that adds 4*4 shorts.
	    
	add	eax, 32				; 

	prefetchnta [eax+32]			; 
	prefetchnta [ecx+eax+32]		; 

						; 
	movq	mm2,  [ecx+eax-24]	; 
	movq	mm3,  [eax-24]		;
	  
	paddw	mm0, mm1			;
	movq	 [eax-32], mm0		;
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	
	paddw	mm2, mm3			;
	movq	 [eax-24], mm2
	
	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	
	paddw	mm0, mm1			;
	movq	 [eax-16], mm0		;
	
	dec	edx				;
	paddw	mm2, mm3			;
	movq	 [eax-8], mm2		;
	
	jne	SHORT .ltop			;

.unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,15				; calculate .remaining 
	shr	edx,2				; number of shorts
	jz	.norest				; 

.lrest:
	movq	mm0,  [ecx+eax]	; add 4 shorts
	movq	mm1,  [eax]		;
	add	eax,8
	paddw	mm0, mm1			;
	movq	 [eax-8], mm0		;
	dec	edx				;
	jne	.lrest				;
.norest:
	
	emms					; restore FP state
	
	and	bl,3				; calculate .remaining shorts
	jz	.end				; 0 --> we are finished
.lshort:
	mov	dx,  [ecx+eax]		; Add the .remaining integer
	add	dx,  [eax]		;
	mov	 [eax],dx		;
	add	eax,2
	dec	bl
	jnz	.lshort

.end:
	pop	ebx
	ret	



; void add_byte_sse(char *inbuf,char *inoutbuf,unsigned int len) 
global add_byte_sse
add_byte_sse:	
	push	ebx
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of bytes 
	sub	edx,eax				; that have to be handled	
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,3				; divide by 8
	jz	.remain
		
.al_top:	
	movq	mm1,  [eax]		; Add bytes
	movq	mm0,  [ecx+eax]	; 8 at a time
	add	eax,8
	paddb	mm0, mm1			;
	movq	 [eax-8], mm0		;
	
	dec	edx
	jne	.al_top
.remain:
	and	bl,7
	jz	.startit

.al_rest:
	mov	dl, [ecx+eax]		; Add .remaining
	add	dl, [eax]		; bytes
	mov	[eax],dl
	inc	eax
	dec	bl
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 5				; len /= 32
	jz	SHORT .unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]			; 
	prefetchnta [ecx+eax+32]		; 
align 4
.ltop:
	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that adds 4*8 bytes.
	    
	add	eax, 32				; 
	prefetchnta [eax+32]			; 
	prefetchnta [ecx+eax+32]		; 

						; 
	movq	mm2,  [ecx+eax-24]	; 
	movq	mm3,  [eax-24]		;
	  
	paddb	mm0, mm1			;
	movq	 [eax-32], mm0		;
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	
	paddb	mm2, mm3			;
	movq	 [eax-24], mm2
	
	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	
	paddb	mm0, mm1			;
	movq	 [eax-16], mm0		;
	
	dec	edx				;
	paddb	mm2, mm3			;
	movq	 [eax-8], mm2		;
	
	jne	SHORT .ltop			;

.unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,31				; calculate .remaining 
	shr	edx,3				; number of bytes
	jz	.norest				; 

.lrest:
	movq	mm0,  [ecx+eax]	; add 8 bytes
	movq	mm1,  [eax]		;
	add	eax,8
	paddb	mm0, mm1			;
	movq	 [eax-8], mm0		;
	dec	edx				;
	jne	.lrest				;
.norest:
	
	emms					; restore FP state
	
	and	bl,7				; calculate .remaining bytes
	jz	.end				; 0 --> we are finished
.lbyte:
	mov	dl,  [ecx+eax]		; Add the .remaining bytes
	add	dl,  [eax]		;
	mov	 [eax],dl		;
	inc	eax
	dec	bl
	jnz	.lbyte

.end:
	pop	ebx
	ret	



; void add_float_sse(float *inbuf,float *inoutbuf,unsigned int len) 
global add_float_sse
add_float_sse:
	push	ebx
	mov	eax,  [esp+12]		; load inoutbuf
	
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	NEAR .aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_FLOAT			; to get .aligned addresses
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]
	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	.remain
	
.al_top:	
	movups	xmm1,  [eax]		; Add floats
	movups	xmm0,  [ecx+eax]	; 4 at a time
	add	eax,16
	addps	xmm0, xmm1			;
	dec	edx
	movups	 [eax-16], xmm0	;
	jne	.al_top
.remain:
	and	bl,3
	jz	.startit

.al_rest:
	movss	xmm1, [ecx+eax]	; Add .remaining
	addss	xmm1, [eax]		; floats
	add	eax,4
	dec	bl
	movss	 [eax-4],xmm1
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;
.startit:
	mov	edx, ecx		; check if inbuf
	and	edx, ALIGNEMENT-1	; is also aligned
	jz	.startit_aligned
	jmp	NEAR .startit_unaligned

; aligned version
.startit_aligned:	
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT .unroll_end_aligned	; len==0 --> less than 8 numbers
	prefetchnta [eax+32]			; Prefetch the following cacheline
	prefetchnta [ecx+eax+32]		; 
	jmp	.INTOLOOP_aligned
ALIGN 4
.ltop_aligned:
	movaps	 [eax-16], xmm2		;

.INTOLOOP_aligned:
	movaps	xmm1,  [ecx+eax]	; This is an unrolled loop
	movaps	xmm0,  [eax] 	; that adds 2*4 floats.
	add	eax, 32				
	prefetchnta [eax+32]			; 
	prefetchnta [ecx+eax+32]		; prefetch the cacheline after the next
	
	addps	xmm0, xmm1			;    
	movaps	xmm2,  [ecx+eax-16]	; 
	movaps	xmm3,  [eax-16]	;
	  	
	movaps	 [eax-32], xmm0	;
	dec	edx				;	
	addps	xmm2, xmm3			;
	jnz	SHORT .ltop_aligned		;

	movaps	 [eax-16], xmm2	;
	
.unroll_end_aligned:	
	test	bl, 4
	jz	.norest_aligned				;
	movaps	xmm1,  [eax]		; Add floats
	movaps	xmm0,  [ecx+eax]	; 4 at a time
	add	eax,16
	addps	xmm0, xmm1			;
	movaps	 [eax-16], xmm0	;
.norest_aligned:	
	

	and	bl,3				;
	jz	.end_aligned			; No--> we are finished
align 4
.lfloat_aligned:	
	movss	xmm1, [ecx+eax]	; Add .remaining
	addss	xmm1, [eax]		; floats
	add	eax,4
	dec	bl
	movss	 [eax-4],xmm1
	jnz	short .lfloat_aligned
.end_aligned:
	pop	ebx
	ret	


; unaligned version
.startit_unaligned:	
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT .unroll_end_unaligned	; len==0 --> less than 8 numbers
	prefetchnta [eax+32]			; Prefetch the following cacheline
	prefetchnta [ecx+eax+32]		; 
	jmp	.INTOLOOP_unaligned
ALIGN 4
.ltop_unaligned:
	movaps	 [eax-16], xmm2		;

.INTOLOOP_unaligned:
	movups	xmm1,  [ecx+eax]	; This is an unrolled loop
	movaps	xmm0,  [eax] 	; that adds 2*4 floats.
	add	eax, 32				
	prefetchnta [eax+32]			; 
	prefetchnta [ecx+eax+32]		; prefetch the cacheline after the next
	
	addps	xmm0, xmm1			;    
	movups	xmm2,  [ecx+eax-16]	; 
	movaps	xmm3,  [eax-16]	;
	  	
	movaps	 [eax-32], xmm0	;
	dec	edx				;	
	addps	xmm2, xmm3			;
	jnz	SHORT .ltop_unaligned		;

	movaps	 [eax-16], xmm2	;
	
.unroll_end_unaligned:	
	test	bl, 4
	jz	.norest_unaligned	;
	movaps	xmm1,  [eax]		; Add floats
	movups	xmm0,  [ecx+eax]	; 4 at a time
	add	eax,16
	addps	xmm0, xmm1			;
	movaps	 [eax-16], xmm0	;
.norest_unaligned:	
	

	and	bl,3				;
	jz	.end_unaligned		; No--> we are finished
align 4
.lfloat_unaligned:	
	movss	xmm1, [ecx+eax]	; Add .remaining
	addss	xmm1, [eax]		; floats
	add	eax,4
	dec	bl
	movss	 [eax-4],xmm1
	jnz	short .lfloat_unaligned
.end_unaligned:
	pop	ebx
	ret	



; void add_s_complex_sse(s_complex *inbuf,s_complex *inoutbuf,unsigned int len) 
global add_s_complex_sse
add_s_complex_sse:
	push	ebx
	mov	eax,  [esp+12]		; load inoutbuf
	
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	NEAR .aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of structs 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_DOUBLE			; to get .aligned addresses
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]
	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,1				; divide by 2
	jz	.remain
	
.al_top:	
	movups	xmm1,  [eax]		; Add complex numbers
	movups	xmm0,  [ecx+eax]	; 2 at a time
	add	eax,16
	addps	xmm0, xmm1			;
	dec	edx
	movups	 [eax-16], xmm0	;
	jne	.al_top
.remain:
	test	bl,1
	jz	.startit

.al_rest:
	movss	xmm1, [ecx+eax]	; Add .remaining
	addss	xmm1, [eax]		; complex number
	movss	 [eax],xmm1
	add	eax,4
	movss	xmm1, [ecx+eax]	
	addss	xmm1, [eax]		
	movss	 [eax],xmm1
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;
.startit:
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 2				; len /= 4
	jz	SHORT .unroll_end		; len==0 --> less than 4 numbers
	prefetchnta [eax+32]			; Prefetch the following cacheline
	prefetchnta [ecx+eax+32]		; 
	jmp	.INTOLOOP
ALIGN 4
.ltop:
	movaps	 [eax-16], xmm2		;

.INTOLOOP:
	movaps	xmm1,  [ecx+eax]	; This is an unrolled loop
	movaps	xmm0,  [eax] 	; that adds 2*2 complex numbers.
	add	eax, 32				
	prefetchnta [eax+32]			; 
	prefetchnta [ecx+eax+32]		; prefetch the cacheline after the next
	
	addps	xmm0, xmm1			;    
	movaps	xmm2,  [ecx+eax-16]	; 
	movaps	xmm3,  [eax-16]	;
	  	
	movaps	 [eax-32], xmm0	;
	dec	edx				;	
	addps	xmm2, xmm3			;
	jnz	SHORT .ltop			;

	movaps	 [eax-16], xmm2	;
	
.unroll_end:	
	test	bl, 2
	jz	.norest				;
	movaps	xmm1,  [eax]		; Add 2 complex
	movaps	xmm0,  [ecx+eax]	; numbers
	add	eax,16
	addps	xmm0, xmm1			;
	movaps	 [eax-16], xmm0	;

.norest:	
	test	bl,1				;
	jz	.end				; No--> we are finished
.lfloat:	
	movss	xmm1, [ecx+eax]	; Add .remaining
	addss	xmm1, [eax]		; number
	movss	 [eax],xmm1
	movss	xmm1, [ecx+eax+4]	
	addss	xmm1, [eax+4]		
	movss	 [eax+4],xmm1
.end:
	pop	ebx
	ret	



; void prod_short_sse(short *inbuf,short* inoutbuf,unsigned int len) 
global prod_short_sse
prod_short_sse:
	push	ebx
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT			; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]
	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	.remain
		
.al_top:	
	movq	mm1,  [eax]		; Multiply shorts
	movq	mm0,  [ecx+eax]	; 4 at a time
	add	eax,8
	pmullw	mm0, mm1			;
	movq	 [eax-8], mm0		;
	
	dec	edx
	jne	.al_top
.remain:
	and	bl,3
	jz	.startit
.al_rest:
	mov	dx, [ecx+eax]		; Multiply .remaining
	imul	dx, [eax]		; shorts
	mov	 [eax],dx
	add	eax,2
	dec	bl
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 4				; len /= 16
	jz	SHORT .unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:
	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that adds 4*4 shorts.
	    
	add	eax, 32				; 
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
	
	movq	mm2,  [ecx+eax-24]	; 
	movq	mm3,  [eax-24]		;
	  
	pmullw	mm0, mm1			;
	movq	 [eax-32], mm0		;
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	
	pmullw	mm2, mm3			;
	movq	 [eax-24], mm2
	
	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	
	pmullw	mm0, mm1			;
	movq	 [eax-16], mm0		;
	
	dec	edx				;
	pmullw	mm2, mm3			;
	movq	 [eax-8], mm2		;
	
	jne	SHORT .ltop			;

.unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,15				; calculate .remaining 
	shr	edx,2				; number of shorts
	jz	.norest				; 

.lrest:
	movq	mm0,  [ecx+eax]	; add 4 shorts
	movq	mm1,  [eax]		;
	add	eax,8
	pmullw	mm0, mm1			;
	movq	 [eax-8], mm0		;
	dec	edx				;
	jne	.lrest				;
.norest:
	
	emms					; restore FP state
	
	and	bl,3				; calculate .remaining shorts
	jz	.end				; 0 --> we are finished
.lshort:
	mov	dx,  [ecx+eax]		; Multiply the .remaining shorts
	imul	dx,  [eax]		;
	mov	 [eax],dx		;
	add	eax,2
	dec	bl
	jnz	.lshort

.end:
	pop	ebx
	ret	



; void prod_float_sse(float *inbuf,float *inoutbuf,unsigned int len) 
global prod_float_sse
prod_float_sse:
	push	ebx
	mov	eax,  [esp+12]		; load inoutbuf
	
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	NEAR .aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_FLOAT			; to get .aligned addresses
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]
	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	.remain
	
.al_top:	
	movups	xmm1,  [eax]		; Add floats
	movups	xmm0,  [ecx+eax]	; 4 at a time
	add	eax,16
	mulps	xmm0, xmm1			;
	dec	edx
	movups	 [eax-16], xmm0	;
	jne	.al_top
.remain:
	and	bl,3
	jz	.startit

.al_rest:
	movss	xmm1, [ecx+eax]	; Add .remaining
	mulss	xmm1, [eax]		; floats
	add	eax,4
	dec	bl
	movss	 [eax-4],xmm1
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;
.startit:
	mov	edx, ecx		; check if inbuf
	and	edx, ALIGNEMENT-1	; is also aligned
	jz	.startit_aligned
	jmp	NEAR .startit_unaligned

	
.startit_aligned:
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT .unroll_end_aligned	; len==0 --> less than 8 numbers
	prefetchnta [eax+32]			; Prefetch the following cacheline
	prefetchnta [ecx+eax+32]		; 
	jmp	.INTOLOOP_aligned

ALIGN 4
.ltop_aligned:
	movaps	 [eax-16], xmm2		;

.INTOLOOP_aligned:
	movaps	xmm1,  [ecx+eax]	; This is an unrolled loop
	movaps	xmm0,  [eax] 	; that adds 2*4 floats.
	add	eax, 32				
	prefetchnta [eax+32]			; 
	prefetchnta [ecx+eax+32]		; prefetch the cacheline after the next
	
	mulps	xmm0, xmm1			;    
	movaps	xmm2,  [ecx+eax-16]	; 
	movaps	xmm3,  [eax-16]	;
	  	
	movaps	 [eax-32], xmm0	;
	dec	edx				;	
	mulps	xmm2, xmm3			;
	jnz	SHORT .ltop_aligned		;

	movaps	 [eax-16], xmm2	;
	
.unroll_end_aligned:	
	test	bl, 4
	jz	.norest_aligned			;
	movaps	xmm1,  [eax]		; Add floats
	movaps	xmm0,  [ecx+eax]	; 4 at a time
	add	eax,16
	mulps	xmm0, xmm1			;
	movaps	 [eax-16], xmm0	;
.norest_aligned:	
	

	and	bl,3				;
	jz	.end_aligned			; No--> we are finished
align 4
.lfloat_aligned:	
	movss	xmm1, [ecx+eax]	; Add .remaining
	mulss	xmm1, [eax]		; floats
	add	eax,4
	dec	bl
	movss	 [eax-4],xmm1
	jnz	short .lfloat_aligned
.end_aligned:
	pop	ebx
	ret	


.startit_unaligned:
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT .unroll_end_unaligned	; len==0 --> less than 8 numbers
	prefetchnta [eax+32]			; Prefetch the following cacheline
	prefetchnta [ecx+eax+32]		; 
	jmp	.INTOLOOP_unaligned

ALIGN 4
.ltop_unaligned:
	movaps	 [eax-16], xmm2		;

.INTOLOOP_unaligned:
	movups	xmm1,  [ecx+eax]	; This is an unrolled loop
	movaps	xmm0,  [eax] 	; that adds 2*4 floats.
	add	eax, 32				
	prefetchnta [eax+32]			; 
	prefetchnta [ecx+eax+32]		; prefetch the cacheline after the next
	
	mulps	xmm0, xmm1			;    
	movups	xmm2,  [ecx+eax-16]	; 
	movaps	xmm3,  [eax-16]	;
	  	
	movaps	 [eax-32], xmm0	;
	dec	edx				;	
	mulps	xmm2, xmm3			;
	jnz	SHORT .ltop_unaligned		;

	movaps	 [eax-16], xmm2	;
	
.unroll_end_unaligned:	
	test	bl, 4
	jz	.norest_aligned			;
	movaps	xmm1,  [eax]		; Add floats
	movups	xmm0,  [ecx+eax]	; 4 at a time
	add	eax,16
	mulps	xmm0, xmm1			;
	movaps	 [eax-16], xmm0	;
.norest_unaligned:	
	

	and	bl,3				;
	jz	.end_unaligned			; No--> we are finished
align 4
.lfloat_unaligned:	
	movss	xmm1, [ecx+eax]	; Add .remaining
	mulss	xmm1, [eax]		; floats
	add	eax,4
	dec	bl
	movss	 [eax-4],xmm1
	jnz	short .lfloat_unaligned
.end_unaligned:
	pop	ebx
	ret	




; void max_sint_sse(int *inbuf,int*inoutbuf,unsigned int len) 
global max_sint_sse
max_sint_sse:
	push	ebx
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	NEAR .aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_INT			; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]
	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,1				; divide by 2
	jz	.remain
		
.al_top:	
	movq	mm1,  [eax]		; compare integers
	movq	mm0,  [ecx+eax]	; 2 at a time
	add	eax,8
	movq	mm4,mm0
	pcmpgtd	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
	movq	 [eax-8], mm0		;
	
	dec	edx
	jne	.al_top
.remain:
	test	bl,1
	jz	.startit

	mov	edx, [ecx+eax]		; compare .remaining
	mov	ebx, [eax]		; integer
	cmp	ebx,edx
	cmovg   edx,ebx
	mov	 [eax],edx
	add	eax,4
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	NEAR .unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:
	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that compares 4*2 integers.
	    
	add	eax, 32				; 
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
	  
	movq	mm4,mm0
	pcmpgtd	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4

	movq	mm2,  [ecx+eax-24]	; 
	movq	mm3,  [eax-24]		;
	movq	 [eax-32], mm0		;
		
	movq	mm4,mm2
	pcmpgtd	mm4,mm3
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4				;

	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	movq	 [eax-24], mm2
		
	movq	mm4,mm0
	pcmpgtd	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4

	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	movq	 [eax-16], mm0		;
	
	dec	edx				;
	movq	mm4,mm2
	pcmpgtd	mm4,mm3
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4
	movq	 [eax-8], mm2		;
	
	jne	NEAR .ltop			;
.unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,7				; calculate .remaining 
	shr	edx,1				; number of integers
	jz	.norest				; 
.lrest:
	movq	mm0,  [ecx+eax]	; add two ints
	movq	mm1,  [eax]		;
	add	eax,8
	movq	mm4,mm0
	pcmpgtd	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4				;
	movq	 [eax-8], mm0		;
	dec	edx				;
	jne	.lrest				;
.norest:
	
	emms					;
	test	bl,1				;
	jz	.end				; No--> we are finished
	mov	edx,  [ecx+eax]	; compare the .remaining integer
	mov	ebx, [eax]		; 
	cmp	ebx,edx
	cmovg   edx,ebx
	mov	 [eax],edx
.end:
	pop	ebx
	ret	



; void max_sshort_sse(short *inbuf,short* inoutbuf,unsigned int len) 
global max_sshort_sse
max_sshort_sse:
	push	ebx
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT			; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	.remain
		
.al_top:	
	movq	mm0,  [ecx+eax]	; compare shorts
	movq	mm1,  [eax]		; 4 at a time
	add	eax,8
	pmaxsw	mm0,mm1
	movq	 [eax-8], mm0		;
	
	dec	edx
	jne	.al_top
.remain:
	and	bl,3
	jz	.startit
.al_rest:
	mov	dx, [ecx+eax]		; compare .remaining
	cmp	dx, [eax]		; shorts
	cmovl	dx, [eax]	
	mov	 [eax],dx
	add	eax,2
	dec	bl
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 4				; len /= 16
	jz	SHORT .unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:
	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that compares 4*4 shorts.
	    
	add	eax, 32				; 
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
	
	
	
	movq	mm2,  [ecx+eax-24]	; 
	movq	mm3,  [eax-24]		;	
	pmaxsw	mm0,mm1
	movq	 [eax-32], mm0		;
	
	pmaxsw	mm2,mm3
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;	
	movq	 [eax-24], mm2
	
	pmaxsw	mm0,mm1
	
	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	pmaxsw	mm0,mm1
	movq	 [eax-16], mm0		;
	
	dec	edx				;
	pmaxsw	mm2,mm3
	movq	 [eax-8], mm2		;
	
	jne	SHORT .ltop			;

.unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,15				; calculate .remaining 
	shr	edx,2				; number of shorts
	jz	.norest				; 

.lrest:
	movq	mm0,  [ecx+eax]	; compare 4 shorts
	movq	mm1,  [eax]		;
	add	eax,8
	pmaxsw	mm0,mm1
	movq	 [eax-8], mm0		;
	dec	edx				;
	jne	.lrest				;
.norest:
	
	emms					; restore FP state
	
	and	bl,3				; calculate .remaining shorts
	jz	.end				; 0 --> we are finished
.lshort:
	mov	dx,  [ecx+eax]		; compare the .remaining shorts
	add	eax,2
	cmp	dx,  [eax-2]		;
	cmovl	dx,  [eax-2]		;
	dec	bl
	mov	 [eax-2],dx		;
	jnz	.lshort

.end:
	pop	ebx
	ret	



; void max_sbyte_sse(char *inbuf,char *inoutbuf,unsigned int len) 
global max_sbyte_sse
max_sbyte_sse:
	push	ebx
	push	esi
	mov	eax,  [esp+16]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	NEAR .aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of bytes 
	sub	edx,eax				; that have to be handled	
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+20]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+20],ecx		; store new length

	mov	esi, edx			; Save counter
	mov	ecx,  [esp+12]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,3				; divide by 8
	jz	.remain
		
.al_top:	
	movq	mm0,  [ecx+eax]	; compare bytes
	movq	mm1,  [eax]		; 8 at a time
	add	eax,8
	dec	edx
	movq	mm4,mm0
	pcmpgtb	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
	movq	 [eax-8], mm0		;
	
	
	jne	.al_top
.remain:
	and	si,7
	jz	.startit
	
.al_rest:
	mov	dl, [ecx+eax]		; compare .remaining
	mov	bl, [eax]
	inc	eax
	cmp	dl,bl				; bytes
	cmovl	dx,bx
	dec	si
	mov	 [eax-1],dl	
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+12]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	edx,  [esp+20]		; Load length
	mov	esi,edx				; Save it
	shr	edx, 5				; len /= 32
	jz	NEAR .unroll_end		; len==0 --> less than 32 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:
	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that compares 4*8 bytes.
	    
	add	eax, 32		
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

	movq	mm4,mm0
	pcmpgtb	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4

	movq	mm2,  [ecx+eax-24]
	movq	mm3,  [eax-24]		;
	movq	 [eax-32], mm0		;
		
	movq	mm4,mm2
	pcmpgtb	mm4,mm3
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	movq	 [eax-24], mm2
		
	movq	mm4,mm0
	pcmpgtb	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4

	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	movq	 [eax-16], mm0		;
	
	dec	edx				;

	movq	mm4,mm2
	pcmpgtb	mm4,mm3
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4
	movq	 [eax-8], mm2		;
	
	jne	NEAR .ltop			;

.unroll_end:	
	mov	edx, esi			; load original length
	and	edx,31				; calculate .remaining 
	shr	edx,3				; number of bytes
	jz	.norest				; 

.lrest:
	movq	mm0,  [ecx+eax]	; compare 8 bytes
	movq	mm1,  [eax]		;
	add	eax,8
	movq	mm4,mm0
	pcmpgtb	mm4,mm1
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
	movq	 [eax-8], mm0		;
	dec	edx				;
	jne	.lrest				;
.norest:
	
	emms					; restore FP state
	
	and	si,7				; calculate .remaining bytes
	jz	.end				; 0 --> we are finished
align 4
.lbyte:
	mov	dl,  [ecx+eax]		; compare the .remaining bytes
	mov	bl,  [eax]		;
	inc	eax
	cmp	dl, bl				;
	cmovl	dx,bx
	dec	si
	mov	 [eax-1],dl		;	
	jnz	.lbyte

.end:
	pop	esi
	pop	ebx
	ret	


; void max_ubyte_sse(unsigned char *inbuf,unsigned char *inoutbuf,unsigned int len) 
global max_ubyte_sse
max_ubyte_sse:
	push	ebx
	push	esi
	mov	eax,  [esp+16]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of bytes 
	sub	edx,eax				; that have to be handled	
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+20]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+20],ecx		; store new length

	mov	esi, edx			; Save counter
	mov	ecx,  [esp+12]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,3				; divide by 8
	jz	.remain
		
.al_top:	
	movq	mm0,  [ecx+eax]	; compare bytes
	movq	mm1,  [eax]		; 8 at a time
	add	eax,8
	dec	edx
	pmaxub	mm0,mm1
	movq	 [eax-8], mm0		;	
	jne	.al_top
.remain:
	and	si,7
	jz	.startit
	
.al_rest:
	mov	dl, [ecx+eax]		; compare .remaining
	mov	bl, [eax]
	inc	eax
	cmp	dl,bl				; bytes
	cmovb	dx,bx
	dec	si
	mov	 [eax-1],dl	
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+12]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	edx,  [esp+20]		; Load length
	mov	esi,edx				; Save it
	shr	edx, 5				; len /= 32
	jz	SHORT .unroll_end		; len==0 --> less than 32 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:
	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that compares 4*8 bytes.
	    
	add	eax, 32		
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

	movq	mm2,  [ecx+eax-24]
	movq	mm3,  [eax-24]		;
	  
	pmaxub	mm0,mm1
	movq	 [eax-32], mm0		;
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	
	pmaxub	mm2,mm3
	movq	 [eax-24], mm2
	
	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	
	pmaxub	mm0,mm1
	movq	 [eax-16], mm0		;
	
	dec	edx				;

	pmaxub	mm2,mm3
	movq	 [eax-8], mm2		;
	
	jne	SHORT .ltop			;

.unroll_end:	
	mov	edx, esi			; load original length
	and	edx,31				; calculate .remaining 
	shr	edx,3				; number of bytes
	jz	.norest				; 

.lrest:
	movq	mm0,  [ecx+eax]	; compare 8 bytes
	movq	mm1,  [eax]		;
	add	eax,8
	pmaxub	mm0,mm1
	movq	 [eax-8], mm0		;
	dec	edx				;
	jne	.lrest				;
.norest:
	
	emms					; restore FP state
	
	and	si,7				; calculate .remaining bytes
	jz	.end				; 0 --> we are finished
align 4
.lbyte:
	mov	dl,  [ecx+eax]		; compare the .remaining bytes
	mov	bl,  [eax]		;
	inc	eax
	cmp	dl, bl				;
	cmovb	dx,bx
	dec	si
	mov	 [eax-1],dl		;	
	jnz	.lbyte

.end:
	pop	esi
	pop	ebx
	ret	



; void max_uint_cmov(unsigned int *inbuf,unsigned int *inoutbuf,unsigned int len) 
global max_uint_cmov
max_uint_cmov:
	push	ebx
	push	esi
	mov	ecx, [esp+20]		; Load length
	test	ecx,ecx
	jz	NEAR .end

	mov	eax,  [esp+16]		; load inoutbuf
	mov	ebx,eax				; Save address of inoutbuf
	

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_INT
	
	mov	eax,ebx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+20]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+20],ecx		; store new length

	mov	esi, edx			; Save counter
	mov	ecx,  [esp+12]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
align 4			
.al_top:	
	mov	edx, [ecx+eax]		; 
	mov	ebx, [eax]
	add	eax,4
	cmp	edx,ebx				; 
	cmovb	edx,ebx
	dec	si
	mov	 [eax-4],edx	
	jnz	.al_top
	jmp	.startit

.aligned:	
	mov	eax, ebx
	mov	ecx,  [esp+12]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	esi,  [esp+20]		; Load length
	shr	esi, 3				; len /= 8
	jz	NEAR .unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:	
	mov	ebx,  [ecx+eax]	; This is an unrolled loop
	mov	edx,  [eax]		; that compares 8 integers.

	add	eax, 32	
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

	cmp	ebx, edx
	cmova	edx, ebx
	mov	 [eax-32],edx

	mov	ebx,  [ecx+eax-28]	
	mov	edx,  [eax-28]		
	cmp	ebx, edx
	cmova	edx, ebx
	mov	 [eax-28],edx

	mov	ebx,  [ecx+eax-24]	
	mov	edx,  [eax-24]		
	cmp	ebx, edx
	cmova	edx, ebx
	mov	 [eax-24],edx

	mov	ebx,  [ecx+eax-20]	
	mov	edx,  [eax-20]		
	cmp	ebx, edx
	cmova	edx, ebx
	mov	 [eax-20],edx

	mov	ebx,  [ecx+eax-16]	
	mov	edx,  [eax-16]	
	cmp	ebx, edx
	cmova	edx, ebx
	mov	 [eax-16],edx

	mov	ebx,  [ecx+eax-12]	
	mov	edx,  [eax-12]		
	cmp	ebx, edx
	cmova	edx, ebx
	mov	 [eax-12],edx

	mov	ebx,  [ecx+eax-8]	
	mov	edx,  [eax-8]		
	cmp	ebx, edx
	cmova	edx, ebx
	mov	 [eax-8],edx

	mov	ebx,  [ecx+eax-4]	
	mov	edx,  [eax-4]		
	cmp	ebx, edx
	cmova	edx, ebx
	dec	esi	
	mov	 [eax-4],edx

	jne	NEAR .ltop

.unroll_end:	
	mov	esi,  [esp+20]		; load original length
	and	si,7				; calculate .remaining 
	jz	.end
align 4
.lrest:	
	mov	edx,  [ecx+eax]	; compare the .remaining integers
	mov	ebx,  [eax]		; 
	add	eax, 4				
	cmp	ebx, edx
	cmova   edx, ebx
	dec	si
	mov	 [eax-4],edx	
	jnz	.lrest
.end:
	pop	esi
	pop	ebx
	ret	



; void max_ushort_cmov(unsigned short *inbuf,unsigned short *inoutbuf,unsigned int len) 
global max_ushort_cmov
max_ushort_cmov:
	push	ebx
	push	esi

	mov	ecx, [esp+20]		; Load length
	test	ecx, ecx
	jz	NEAR .end

	mov	eax,  [esp+16]		; load inoutbuf
	mov	ebx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT
	
	mov	eax,ebx				; restore address of inoutbuf
	prefetchnta [eax+32]

	
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length

	mov	 [esp+20],ecx		; store new length
	mov	esi, edx			; Save counter

	mov	ecx,  [esp+12]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
align 4			
.al_top:	
	mov	dx, [ecx+eax]		; 
	mov	bx, [eax]
	add	eax,2
	cmp	dx,bx				; 
	cmova	bx,dx
	dec	si
	mov	 [eax-2],bx	
	jnz	.al_top
	jmp	.startit

.aligned:	
	mov	eax, ebx
	mov	ecx,  [esp+12]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	esi,  [esp+20]		; Load length
	shr	esi, 4				; len /= 16
	jz	NEAR .unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:	
	mov	bx,  [ecx+eax]		; This is an unrolled loop
	mov	dx,  [eax]		; that compares 16 shorts.

	add	eax, 32	
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

	cmp	bx, dx
	cmova	dx, bx
	mov	 [eax-32],dx

	mov	bx,  [ecx+eax-30]	
	mov	dx,  [eax-30]		
	cmp	bx, dx
	cmova	dx, bx
	mov	 [eax-30],dx

	mov	bx,  [ecx+eax-28]	
	mov	dx,  [eax-28]		
	cmp	bx, dx
	cmova	dx, bx
	mov	 [eax-28],dx

	mov	bx,  [ecx+eax-26]	
	mov	dx,  [eax-26]		
	cmp	bx, dx
	cmova	dx, bx
	mov	 [eax-26],dx

	mov	bx,  [ecx+eax-24]	
	mov	dx,  [eax-24]	
	cmp	bx, dx
	cmova	dx, bx
	mov	 [eax-24],dx

	mov	bx,  [ecx+eax-22]	
	mov	dx,  [eax-22]		
	cmp	bx, dx
	cmova	dx, bx
	mov	 [eax-22],dx

	mov	bx,  [ecx+eax-20]	
	mov	dx,  [eax-20]		
	cmp	bx, dx
	cmova	dx, bx
	mov	 [eax-20],dx

	mov	bx,  [ecx+eax-18]	
	mov	dx,  [eax-18]		
	cmp	bx, dx
	cmova	dx, bx	
	mov	 [eax-18],dx

	mov	bx,  [ecx+eax-16]		
	mov	dx,  [eax-16]		
	cmp	bx, dx
	cmova	dx, bx
	mov	 [eax-16],dx

	mov	bx,  [ecx+eax-14]	
	mov	dx,  [eax-14]		
	cmp	bx, dx
	cmova	dx, bx
	mov	 [eax-14],dx

	mov	bx,  [ecx+eax-12]	
	mov	dx,  [eax-12]		
	cmp	bx, dx
	cmova	dx, bx
	mov	 [eax-12],dx

	mov	bx,  [ecx+eax-10]	
	mov	dx,  [eax-10]		
	cmp	bx, dx
	cmova	dx, bx
	mov	 [eax-10],dx

	mov	bx,  [ecx+eax-8]	
	mov	dx,  [eax-8]	
	cmp	bx, dx
	cmova	dx, bx
	mov	 [eax-8],dx

	mov	bx,  [ecx+eax-6]	
	mov	dx,  [eax-6]		
	cmp	bx, dx
	cmova	dx, bx
	mov	 [eax-6],dx

	mov	bx,  [ecx+eax-4]	
	mov	dx,  [eax-4]		
	cmp	bx, dx
	cmova	dx, bx
	mov	 [eax-4],dx

	mov	bx,  [ecx+eax-2]	
	mov	dx,  [eax-2]		
	cmp	bx, dx
	cmova	dx, bx
	dec	esi	
	mov	 [eax-2],dx
	jne	NEAR .ltop

.unroll_end:	
	mov	esi,  [esp+20]		; load original length
	and	si,15				; calculate .remaining 
	jz	.end
align 4
.lrest:	
	mov	dx,  [ecx+eax]	; compare the .remaining shorts
	mov	bx,  [eax]		; 
	add	eax, 2				
	cmp	bx, dx
	cmova   dx, bx
	dec	si
	mov	 [eax-2],dx	
	jnz	.lrest
.end:
	pop	esi
	pop	ebx
	ret	



; void max_float_sse(float *inbuf,float* inoutbuf,unsigned int len) 
global max_float_sse
max_float_sse:
	push	ebx
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	NEAR .aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_FLOAT			; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	.remain
		
.al_top:	
	movups	xmm0,  [ecx+eax]	; compare floats
	movups	xmm1,  [eax]
	maxps	xmm0, xmm1			; 4 at a time
	add	eax,16
	dec	edx
	movups	 [eax-16], xmm0	;
	jne	.al_top
.remain:
	and	bl,3
	jz	.startit
.al_rest:
	movss	xmm0, [ecx+eax]
	maxss	xmm0, [eax]
	add	eax,4
	movss	 [eax-4],xmm0
	dec	bl
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	edx, ecx		; check if inbuf
	and	edx, ALIGNEMENT-1	; is also aligned
	jz	.startit_aligned
	jmp	NEAR .startit_unaligned

.startit_aligned:
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT .unroll_end_aligned		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop_aligned:
	movaps	xmm0,  [ecx+eax]	; This is an unrolled loop
	movaps	xmm1,  [eax]		; that compares 2*4 floats.
	    
	add	eax, 32				; 
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
	maxps	xmm0,xmm1

	movaps	xmm2,  [ecx+eax-16]	; 
	movaps	xmm3,  [eax-16]	;

	maxps	xmm2,xmm3
	movaps	 [eax-32], xmm0	;
	dec	edx			
	movaps	 [eax-16], xmm2
	jne	SHORT .ltop_aligned

.unroll_end_aligned:	
	test	bl, 4				; at least 4 numbers left?
	jz	.norest_aligned

	movaps	xmm0,  [ecx+eax]	; compare 4 floats
	movaps	xmm1,  [eax]		;
	add	eax,16
	maxps	xmm0,xmm1
	movaps	 [eax-16], xmm0

.norest_aligned:
	and	bl,3				; calculate .remaining floats
	jz	.end_aligned			; 0 --> we are finished
.lfloat_aligned:
	movss	xmm0,  [ecx+eax]	; compare the .remaining floats
	maxss	xmm0,  [eax]		;
	add	eax,4
	dec	bl
	movss	 [eax-4],xmm0		;
	jnz	.lfloat_aligned

.end_aligned:
	pop	ebx
	ret	


.startit_unaligned:
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT .unroll_end_unaligned		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop_unaligned:
	movups	xmm0,  [ecx+eax]	; This is an unrolled loop
	movaps	xmm1,  [eax]		; that compares 2*4 floats.
	    
	add	eax, 32				; 
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
	maxps	xmm0,xmm1

	movups	xmm2,  [ecx+eax-16]	; 
	movaps	xmm3,  [eax-16]	;

	maxps	xmm2,xmm3
	movaps	 [eax-32], xmm0	;
	dec	edx			
	movaps	 [eax-16], xmm2
	jne	SHORT .ltop_unaligned

.unroll_end_unaligned:	
	test	bl, 4				; at least 4 numbers left?
	jz	.norest_unaligned

	movups	xmm0,  [ecx+eax]	; compare 4 floats
	movaps	xmm1,  [eax]		;
	add	eax,16
	maxps	xmm0,xmm1
	movaps	 [eax-16], xmm0

.norest_unaligned:
	and	bl,3				; calculate .remaining floats
	jz	.end_unaligned			; 0 --> we are finished
.lfloat_unaligned:
	movss	xmm0,  [ecx+eax]	; compare the .remaining floats
	maxss	xmm0,  [eax]		;
	add	eax,4
	dec	bl
	movss	 [eax-4],xmm0		;
	jnz	.lfloat_unaligned

.end_unaligned:
	pop	ebx
	ret	



; void max_double_cmov(double *inbuf,double* inoutbuf,unsigned int len)
	;; WARNING:	doesn't check for 8-Byte Alignment. Alignment isn't
	;;		needed for FPU instructions. May change later (SSE2...)
global max_double_cmov
max_double_cmov:
	push	ebx

	mov	ecx, [esp+16]		; Load length
	test	ecx,ecx
	jz	NEAR .end

	mov	eax,  [esp+12]		; load inoutbuf
	mov	ebx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_DOUBLE			; to get .aligned addresses

	mov	eax,ebx				; restore address of inoutbuf
	prefetchnta [eax+32]

	
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
		
align 4	
.al_top:	
	fld	QWORD [ecx+eax]		; 
	fld	QWORD [eax]			; 
	add	eax, 8	
	fucomi	st0,st1
	fcmovb	st0,st1
	fstp	QWORD [eax-8]
	dec	edx
	ffree	st0	
	jg	.al_top		; jg allows him to work with "not 8-Byte aligned" memory
	jmp	.startit
.aligned:	
	mov	eax, ebx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 2				; len /= 4
	jz	SHORT .unroll_end		; len==0 --> less than 4 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:
	fld	QWORD [ecx+eax]		; This is an unrolled loop
	fld	QWORD [eax]			; that compares 4 doubles.
	add	eax, 32	

	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

	fucomi	st0,st1
	fcmovb	st0,st1
	fstp	QWORD [eax-32]
	ffree	st0

	fld	QWORD [ecx+eax-24]
	fld	QWORD [eax-24]
	fucomi	st0,st1
	fcmovb	st0,st1
	fstp	QWORD [eax-24]
	ffree	st0

	fld	QWORD [ecx+eax-16]
	fld	QWORD [eax-16]
	fucomi	st0,st1
	fcmovb	st0,st1
	fstp	QWORD [eax-16]
	ffree	st0

	fld	QWORD [ecx+eax-8]
	fld	QWORD [eax-8]
	fucomi	st0,st1
	fcmovb	st0,st1
	dec	edx
	fstp	QWORD [eax-8]
	ffree	st0
	jne	SHORT .ltop

.unroll_end:	
	and	bl,3				; calculate .remaining 
	jz	.end

.lrest:	
	fld	QWORD [ecx+eax]		; compare the .remaining numbers
	fld	QWORD [eax]			; 
	add	eax, 8				
	fucomi	st0,st1
	fcmovb	st0,st1
	dec	bl
	fstp	QWORD [eax-8]
	ffree	st0
	jnz	.lrest
.end:
	pop	ebx
	ret	



; void min_sint_sse(int *inbuf,int*inoutbuf,unsigned int len) 
global min_sint_sse
min_sint_sse:	
	push	ebx
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	NEAR .aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_INT			; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,1				; divide by 2
	jz	.remain
	
	
.al_top:	
	movq	mm1,  [eax]		; compare integers
	movq	mm0,  [ecx+eax]	; 2 at a time
	add	eax,8
	movq	mm4,mm1
	pcmpgtd	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
	movq	 [eax-8], mm0		;
	
	dec	edx
	jne	.al_top
.remain:
	test	bl,1
	jz	.startit

	mov	edx, [ecx+eax]		; compare .remaining
	mov	ebx, [eax]		; integer
	cmp	edx,ebx
	cmovg   edx,ebx
	mov	 [eax],edx
	add	eax,4
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	NEAR .unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

align 4
.ltop:
	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that compares 4*2 integers.
	    
	add	eax, 32				; 
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
						; 	  
	movq	mm4,mm1
	pcmpgtd	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4

	movq	mm2,  [ecx+eax-24]	; 
	movq	mm3,  [eax-24]		;
	movq	 [eax-32], mm0		;
		
	movq	mm4,mm3
	pcmpgtd	mm4,mm2
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4				;

	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	movq	 [eax-24], mm2
		
	movq	mm4,mm1
	pcmpgtd	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4

	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	movq	 [eax-16], mm0		;
	
	dec	edx				;
	movq	mm4,mm3
	pcmpgtd	mm4,mm2
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4
	movq	 [eax-8], mm2		;
	
	jne	NEAR .ltop			;
.unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,7				; calculate .remaining 
	shr	edx,1				; number of integers
	jz	.norest				; 
.lrest:
	movq	mm0,  [ecx+eax]	; add two ints
	movq	mm1,  [eax]		;
	add	eax,8
	movq	mm4,mm1
	pcmpgtd	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4				;
	movq	 [eax-8], mm0		;
	dec	edx				;
	jne	.lrest				;
.norest:
	
	emms					;
	test	bl,1				;
	jz	.end				; No--> we are finished
	mov	edx,  [ecx+eax]	; compare the .remaining integer
	mov	ebx, [eax]		; 
	cmp	edx,ebx
	cmovg   edx,ebx
	mov	 [eax],edx
.end:
	pop	ebx
	ret	



; void min_sshort_sse(short *inbuf,short* inoutbuf,unsigned int len) 
global min_sshort_sse
min_sshort_sse:	
	push	ebx
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT			; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	.remain
		
.al_top:	
	movq	mm0,  [ecx+eax]	; compare shorts
	movq	mm1,  [eax]		; 4 at a time
	add	eax,8
	pminsw	mm0,mm1
	movq	 [eax-8], mm0		;
	
	dec	edx
	jne	.al_top
.remain:
	and	bl,3
	jz	.startit
.al_rest:
	mov	dx, [ecx+eax]		; compare .remaining
	cmp	dx, [eax]		; shorts
	cmovg	dx, [eax]	
	mov	 [eax],dx
	add	eax,2
	dec	bl
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 4				; len /= 16
	jz	SHORT .unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:
	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that compares 4*4 shorts.
	    
	add	eax, 32				; 
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
	
	pminsw	mm0,mm1
	
	movq	mm2,  [ecx+eax-24]	; 
	movq	mm3,  [eax-24]		;	
	movq	 [eax-32], mm0		;
	
	pminsw	mm2,mm3
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;	
	movq	 [eax-24], mm2
	
	pminsw	mm0,mm1
	
	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	movq	 [eax-16], mm0		;
	
	dec	edx				;
	pminsw	mm2,mm3
	movq	 [eax-8], mm2		;
	
	jne	SHORT .ltop			;

.unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,15				; calculate .remaining 
	shr	edx,2				; number of shorts
	jz	.norest				; 

.lrest:
	movq	mm0,  [ecx+eax]	; compare 4 shorts
	movq	mm1,  [eax]		;
	pminsw	mm0,mm1
	add	eax,8
	dec	edx				;
	movq	 [eax-8], mm0		;	
	jne	.lrest				;
.norest:
	
	emms					; restore FP state
	
	and	bl,3				; calculate .remaining shorts
	jz	.end				; 0 --> we are finished
.lshort:
	mov	dx,  [ecx+eax]		; compare the .remaining shorts
	add	eax,2
	cmp	dx,  [eax-2]		;
	cmovg	dx,  [eax-2]		;
	dec	bl
	mov	 [eax-2],dx		;
	jnz	.lshort

.end:
	pop	ebx
	ret	




; void min_sbyte_sse(char *inbuf,char *inoutbuf,unsigned int len) 
global min_sbyte_sse
min_sbyte_sse:	
	push	ebx
	push	esi
	mov	eax,  [esp+16]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	NEAR .aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of bytes 
	sub	edx,eax				; that have to be handled	
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+20]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+20],ecx		; store new length

	mov	esi, edx			; Save counter
	mov	ecx,  [esp+12]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,3				; divide by 8
	jz	.remain
		
.al_top:	
	movq	mm0,  [ecx+eax]	; compare bytes
	movq	mm1,  [eax]		; 8 at a time
	add	eax,8
	dec	edx
	movq	mm4,mm1
	pcmpgtb	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
	movq	 [eax-8], mm0		;
	
	
	jne	.al_top
.remain:
	and	si,7
	jz	.startit
	
.al_rest:
	mov	dl, [ecx+eax]		; compare .remaining
	mov	bl, [eax]
	inc	eax
	cmp	dl,bl				; bytes
	cmova	dx,bx
	dec	si
	mov	 [eax-1],dl	
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+12]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	edx,  [esp+20]		; Load length
	mov	esi,edx				; Save it
	shr	edx, 5				; len /= 32
	jz	NEAR .unroll_end		; len==0 --> less than 32 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

align 4
.ltop:
	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that compares 4*8 bytes.
	    
	add	eax, 32		
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

	movq	mm4,mm1
	pcmpgtb	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4

	movq	mm2,  [ecx+eax-24]
	movq	mm3,  [eax-24]		;
	movq	 [eax-32], mm0		;
		
	movq	mm4,mm3
	pcmpgtb	mm4,mm2
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	movq	 [eax-24], mm2
		
	movq	mm4,mm1
	pcmpgtb	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4

	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	movq	 [eax-16], mm0		;
	
	dec	edx				;

	movq	mm4,mm3
	pcmpgtb	mm4,mm2
	pand	mm2,mm4
	pandn	mm4,mm3
	por	mm2,mm4
	movq	 [eax-8], mm2		;
	
	jne	NEAR .ltop			;

.unroll_end:	
	mov	edx, esi			; load original length
	and	edx,31				; calculate .remaining 
	shr	edx,3				; number of bytes
	jz	.norest				; 

.lrest:
	movq	mm0,  [ecx+eax]	; compare 8 bytes
	movq	mm1,  [eax]		;
	add	eax,8
	movq	mm4,mm1
	pcmpgtb	mm4,mm0
	pand	mm0,mm4
	pandn	mm4,mm1
	por	mm0,mm4
	movq	 [eax-8], mm0		;
	dec	edx				;
	jne	.lrest				;
.norest:
	
	emms					; restore FP state
	
	and	si,7				; calculate .remaining bytes
	jz	.end				; 0 --> we are finished
align 4
.lbyte:
	mov	dl,  [ecx+eax]		; compare the .remaining bytes
	mov	bl,  [eax]		;
	inc	eax
	cmp	dl, bl				;
	cmova	dx,bx
	dec	si
	mov	 [eax-1],dl		;	
	jnz	.lbyte

.end:
	pop	esi
	pop	ebx
	ret	



; void min_uint_cmov(unsigned int *inbuf,unsigned int*inoutbuf,unsigned int len) 
global min_uint_cmov
min_uint_cmov:
	push	ebx
	push	esi
	mov	ecx, [esp+20]		; Load length
	test	ecx,ecx
	jz	NEAR .end

	mov	eax,  [esp+16]		; load inoutbuf
	mov	ebx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_INT
	
	mov	eax,ebx				; restore address of inoutbuf
	prefetchnta [eax+32]

	
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+20],ecx		; store new length

	mov	esi, edx			; Save counter
	mov	ecx,  [esp+12]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
align 4			
.al_top:	
	mov	edx, [ecx+eax]		; 
	mov	ebx, [eax]
	add	eax, 4
	cmp	ebx,edx				; 
	cmovb	edx,ebx
	dec	si
	mov	 [eax-4],edx	
	jnz	.al_top
	jmp	.startit

.aligned:	
	mov	eax, ebx
	mov	ecx,  [esp+12]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	esi,  [esp+20]		; Load length
	shr	esi, 3				; len /= 8
	jz	NEAR .unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:	
	mov	ebx,  [ecx+eax]	; This is an unrolled loop
	mov	edx,  [eax]		; that compares 8 integers.

	add	eax, 32	
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

	cmp	ebx, edx
	cmovb	edx, ebx
	mov	 [eax-32],edx

	mov	ebx,  [ecx+eax-28]	
	mov	edx,  [eax-28]		
	cmp	ebx, edx
	cmovb	edx, ebx
	mov	 [eax-28],edx

	mov	ebx,  [ecx+eax-24]	
	mov	edx,  [eax-24]		
	cmp	ebx, edx
	cmovb	edx, ebx
	mov	 [eax-24],edx

	mov	ebx,  [ecx+eax-20]	
	mov	edx,  [eax-20]		
	cmp	ebx, edx
	cmovb	edx, ebx
	mov	 [eax-20],edx

	mov	ebx,  [ecx+eax-16]	
	mov	edx,  [eax-16]	
	cmp	ebx, edx
	cmovb	edx, ebx
	mov	 [eax-16],edx

	mov	ebx,  [ecx+eax-12]	
	mov	edx,  [eax-12]		
	cmp	ebx, edx
	cmovb	edx, ebx
	mov	 [eax-12],edx

	mov	ebx,  [ecx+eax-8]	
	mov	edx,  [eax-8]		
	cmp	ebx, edx
	cmovb	edx, ebx
	mov	 [eax-8],edx

	mov	ebx,  [ecx+eax-4]	
	mov	edx,  [eax-4]		
	cmp	ebx, edx
	cmovb	edx, ebx
	dec	esi	
	mov	 [eax-4],edx

	jne	NEAR .ltop

.unroll_end:	
	mov	esi,  [esp+20]		; load original length
	and	si,7				; calculate .remaining 
	jz	.end
align 4
.lrest:	
	mov	edx,  [ecx+eax]	; compare the .remaining integers
	mov	ebx,  [eax]		; 
	add	eax, 4				
	cmp	ebx, edx
	cmovb   edx, ebx
	dec	si
	mov	 [eax-4],edx	
	jnz	.lrest
.end:
	pop	esi
	pop	ebx
	ret	



; void min_ushort_cmov(unsigned short *inbuf,unsigned short *inoutbuf,unsigned int len) 
global min_ushort_cmov
min_ushort_cmov:
	push	ebx
	push	esi
	
	mov	ecx, [esp+20]		; Load length
	test	ecx,ecx
	jz	NEAR .end

	mov	eax,  [esp+16]		; load inoutbuf
	mov	ebx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT
	
	mov	eax,ebx				; restore address of inoutbuf
	prefetchnta [eax+32]

	cmp	ecx,edx				; Is length smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+20],ecx		; store new length
	mov	esi, edx

	mov	ecx,  [esp+12]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
align 4			
.al_top:	
	mov	dx, [ecx+eax]		; 
	mov	bx, [eax]
	add	eax, 2
	cmp	bx,dx				; 
	cmovb	dx,bx
	dec	si
	mov	 [eax-2],dx	
	jnz	.al_top
	jmp	.startit

.aligned:	
	mov	eax, ebx
	mov	ecx,  [esp+12]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	esi,  [esp+20]		; Load length
	shr	esi, 4				; len /= 16
	jz	NEAR .unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:	
	mov	bx,  [ecx+eax]		; This is an unrolled loop
	mov	dx,  [eax]		; that compares 16 shorts.

	add	eax, 32	
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

	cmp	bx, dx
	cmovb	dx, bx
	mov	 [eax-32],dx

	mov	bx,  [ecx+eax-30]	
	mov	dx,  [eax-30]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	 [eax-30],dx

	mov	bx,  [ecx+eax-28]	
	mov	dx,  [eax-28]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	 [eax-28],dx

	mov	bx,  [ecx+eax-26]	
	mov	dx,  [eax-26]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	 [eax-26],dx

	mov	bx,  [ecx+eax-24]	
	mov	dx,  [eax-24]	
	cmp	bx, dx
	cmovb	dx, bx
	mov	 [eax-24],dx

	mov	bx,  [ecx+eax-22]	
	mov	dx,  [eax-22]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	 [eax-22],dx

	mov	bx,  [ecx+eax-20]	
	mov	dx,  [eax-20]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	 [eax-20],dx

	mov	bx,  [ecx+eax-18]	
	mov	dx,  [eax-18]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	 [eax-18],dx

	mov	bx,  [ecx+eax-16]		
	mov	dx,  [eax-16]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	 [eax-16],dx

	mov	bx,  [ecx+eax-14]	
	mov	dx,  [eax-14]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	 [eax-14],dx

	mov	bx,  [ecx+eax-12]	
	mov	dx,  [eax-12]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	 [eax-12],dx

	mov	bx,  [ecx+eax-10]	
	mov	dx,  [eax-10]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	 [eax-10],dx

	mov	bx,  [ecx+eax-8]	
	mov	dx,  [eax-8]	
	cmp	bx, dx
	cmovb	dx, bx
	mov	 [eax-8],dx

	mov	bx,  [ecx+eax-6]	
	mov	dx,  [eax-6]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	 [eax-6],dx

	mov	bx,  [ecx+eax-4]	
	mov	dx,  [eax-4]		
	cmp	bx, dx
	cmovb	dx, bx
	mov	 [eax-4],dx

	mov	bx,  [ecx+eax-2]	
	mov	dx,  [eax-2]		
	cmp	bx, dx
	cmovb	dx, bx
	dec	esi	
	mov	 [eax-2],dx
	jne	NEAR .ltop

.unroll_end:	
	mov	esi,  [esp+20]		; load original length
	and	si,15				; calculate .remaining 
	jz	.end
align 4
.lrest:	
	mov	dx,  [ecx+eax]	; compare the .remaining shorts
	mov	bx,  [eax]		; 
	add	eax, 2				
	cmp	bx, dx
	cmovb   dx, bx
	dec	si
	mov	 [eax-2],dx	
	jnz	.lrest
.end:
	pop	esi
	pop	ebx
	ret	



; void min_ubyte_sse(unsigned char *inbuf,unsigned char *inoutbuf,unsigned int len) 
global min_ubyte_sse
min_ubyte_sse:
	push	ebx
	push	esi

	mov	ecx, [esp+20]		; Load length
	test	ecx,ecx
	jz	NEAR .end

	mov	eax,  [esp+16]		; load inoutbuf
	mov	ebx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of bytes 
	sub	edx,eax				; that have to be handled	
	
	mov	eax,ebx				; restore address of inoutbuf
	prefetchnta [eax+32]


	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+20],ecx		; store new length

	mov	esi, edx			; Save counter
	mov	ecx,  [esp+12]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,3				; divide by 8
	jz	.remain
		
.al_top:	
	movq	mm0,  [ecx+eax]	; compare bytes
	movq	mm1,  [eax]		; 8 at a time
	add	eax,8
	dec	edx
	pminub	mm0,mm1
	movq	 [eax-8], mm0		;	
	jne	.al_top
.remain:
	and	si,7
	jz	.startit
	
.al_rest:
	mov	dl, [ecx+eax]		; compare .remaining
	mov	bl, [eax]
	inc	eax
	cmp	dl,bl				; bytes
	cmova	dx,bx
	dec	si
	mov	 [eax-1],dl	
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ebx
	mov	ecx,  [esp+12]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	edx,  [esp+20]		; Load length
	mov	esi,edx				; Save it
	shr	edx, 5				; len /= 32
	jz	SHORT .unroll_end		; len==0 --> less than 32 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:
	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that compares 4*8 bytes.
	    
	add	eax, 32		
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

	movq	mm2,  [ecx+eax-24]
	movq	mm3,  [eax-24]		;
	  
	pminub	mm0,mm1
	movq	 [eax-32], mm0		;
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	
	pminub	mm2,mm3
	movq	 [eax-24], mm2
	
	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	
	pminub	mm0,mm1
	movq	 [eax-16], mm0		;
	
	dec	edx				;

	pminub	mm2,mm3
	movq	 [eax-8], mm2		;
	
	jne	SHORT .ltop			;

.unroll_end:	
	mov	edx, esi			; load original length
	and	edx,31				; calculate .remaining 
	shr	edx,3				; number of bytes
	jz	.norest				; 

.lrest:
	movq	mm0,  [ecx+eax]	; compare 8 bytes
	movq	mm1,  [eax]		;
	add	eax,8
	pminub	mm0,mm1
	movq	 [eax-8], mm0		;
	dec	edx				;
	jne	.lrest				;
.norest:
	
	emms					; restore FP state
	
	and	si,7				; calculate .remaining bytes
	jz	.end				; 0 --> we are finished
align 4
.lbyte:
	mov	dl,  [ecx+eax]		; compare the .remaining bytes
	mov	bl,  [eax]		;
	inc	eax
	cmp	dl, bl				;
	cmova	dx,bx
	dec	si
	mov	 [eax-1],dl		;	
	jnz	.lbyte

.end:
	pop	esi
	pop	ebx
	ret	



; void min_float_sse(float *inbuf,float* inoutbuf,unsigned int len) 
global min_float_sse
min_float_sse:
	push	ebx
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	NEAR .aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_FLOAT			; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	.remain
		
.al_top:	
	movups	xmm0,  [ecx+eax]	; compare floats
	movups	xmm1,  [eax]		; 4 at a time
	minps	xmm0, xmm1
	add	eax,16
	dec	edx
	movups	 [eax-16], xmm0	;
	jne	.al_top
.remain:
	and	bl,3
	jz	.startit
.al_rest:
	movss	xmm0, [ecx+eax]
	minss	xmm0, [eax]
	add	eax,4
	dec	bl
	movss	 [eax-4],xmm0
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	edx, ecx		; check if inbuf
	and	edx, ALIGNEMENT-1	; is also aligned
	jz	.startit_aligned
	jmp	NEAR .startit_unaligned


.startit_aligned:
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT .unroll_end_aligned	; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop_aligned:
	movaps	xmm0,  [ecx+eax]	; This is an unrolled loop
	movaps	xmm1,  [eax]		; that compares 2*4 floats.
	    
	add	eax, 32				; 
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
	movaps	xmm2,  [ecx+eax-16]	; 
	movaps	xmm3,  [eax-16]	;
	  
	minps	xmm0,xmm1
	movaps	 [eax-32], xmm0	;

	dec	edx			
	minps	xmm2,xmm3
	movaps	 [eax-16], xmm2
	jne	SHORT .ltop_aligned

.unroll_end_aligned:
	test	bl, 4				; at least 4 numbers left?
	jz	.norest_aligned				; 

	movaps	xmm0,  [ecx+eax]	; compare 4 floats
	movaps	xmm1,  [eax]		;
	add	eax,16
	minps	xmm0,xmm1
	movaps	 [eax-16], xmm0

.norest_aligned:	
	and	bl,3				; calculate .remaining floats
	jz	.end_aligned			; 0 --> we are finished
.lfloat_aligned:
	movss	xmm0,  [ecx+eax]	; compare the .remaining floats
	add	eax,4
	minss	xmm0,  [eax-4]		;
	dec	bl
	movss	[eax-4],xmm0		;
	jnz	.lfloat_aligned

.end_aligned:
	pop	ebx
	ret	


.startit_unaligned:
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT .unroll_end_unaligned	; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop_unaligned:
	movups	xmm0,  [ecx+eax]	; This is an unrolled loop
	movaps	xmm1,  [eax]		; that compares 2*4 floats.
	    
	add	eax, 32				; 
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
	movups	xmm2,  [ecx+eax-16]	; 
	movaps	xmm3,  [eax-16]	;
	  
	minps	xmm0,xmm1
	movaps	 [eax-32], xmm0	;

	dec	edx			
	minps	xmm2,xmm3
	movaps	 [eax-16], xmm2
	jne	SHORT .ltop_unaligned

.unroll_end_unaligned:
	test	bl, 4				; at least 4 numbers left?
	jz	.norest_unaligned				; 

	movups	xmm0,  [ecx+eax]	; compare 4 floats
	movaps	xmm1,  [eax]		;
	add	eax,16
	minps	xmm0,xmm1
	movaps	 [eax-16], xmm0

.norest_unaligned:	
	and	bl,3				; calculate .remaining floats
	jz	.end_unaligned			; 0 --> we are finished
.lfloat_unaligned:
	movss	xmm0,  [ecx+eax]	; compare the .remaining floats
	add	eax,4
	minss	xmm0,  [eax-4]		;
	dec	bl
	movss	[eax-4],xmm0		;
	jnz	.lfloat_unaligned

.end_unaligned:
	pop	ebx
	ret	



; void min_double_cmov(double *inbuf,double* inoutbuf,unsigned int len)
	;; WARNING:	doesn't check for 8-Byte Alignment. Alignment isn't
	;;		needed for FPU instructions. May change later (SSE2...)
global min_double_cmov
min_double_cmov:
	push	ebx

	mov	ecx, [esp+16]		; Load length
	test	ecx,ecx
	jz	NEAR .end

	mov	eax,  [esp+12]		; load inoutbuf
	mov	ebx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_DOUBLE			; to get .aligned addresses

	mov	eax,ebx				; restore address of inoutbuf
	prefetchnta [eax+32]

	
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
		
align 4	
.al_top:	
	fld	QWORD [ecx+eax]		; 
	fld	QWORD [eax]			; 
	add	eax, 8	
	fucomi	st0,st1
	fcmovnbe	st0,st1
	fstp	QWORD [eax-8]
	dec	edx
	ffree	st0	
	jg	.al_top		; jg allows him to work with "not 8-Byte aligned" memory	
	jmp	.startit
.aligned:	
	mov	eax, ebx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 2				; len /= 4
	jz	SHORT .unroll_end		; len==0 --> less than 4 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:
	fld	QWORD [ecx+eax]		; This is an unrolled loop
	fld	QWORD [eax]			; that compares 4 doubles.
	add	eax, 32	

	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

	fucomi	st0,st1
	fcmovnbe	st0,st1
	fstp	QWORD [eax-32]
	ffree	st0

	fld	QWORD [ecx+eax-24]
	fld	QWORD [eax-24]
	fucomi	st0,st1
	fcmovnbe	st0,st1
	fstp	QWORD [eax-24]
	ffree	st0

	fld	QWORD [ecx+eax-16]
	fld	QWORD [eax-16]
	fucomi	st0,st1
	fcmovnbe	st0,st1
	fstp	QWORD [eax-16]
	ffree	st0

	fld	QWORD [ecx+eax-8]
	fld	QWORD [eax-8]
	fucomi	st0,st1
	fcmovnbe	st0,st1
	dec	edx
	fstp	QWORD [eax-8]
	ffree	st0
	jne	SHORT .ltop

.unroll_end:	
	
	and	bl,3				; calculate .remaining 
	jz	.end

.lrest:	
	fld	QWORD [ecx+eax]		; compare the .remaining numbers
	fld	QWORD [eax]			; 
	add	eax, 8				
	fucomi	st0,st1
	fcmovnbe	st0,st1
	dec	bl
	fstp	QWORD [eax-8]
	ffree	st0
	jnz	.lrest
.end:
	pop	ebx
	ret	




; void band_int_sse(int *inbuf,int*inoutbuf,unsigned int len) 
global band_int_sse
band_int_sse:	
	push	ebx
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_INT			; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,1				; divide by 2
	jz	.remain
	
	
.al_top:	
	movq	mm1,  [eax]		; handle integers
	movq	mm0,  [ecx+eax]	; 2 at a time
	add	eax,8
	pand	mm0, mm1			;
	movq	 [eax-8], mm0		;
	
	dec	edx
	jne	.al_top
.remain:
	test	bl,1
	jz	.startit

	mov	edx, [ecx+eax]		; handle .remaining
	and	edx, [eax]		; integer
	mov	 [eax],edx
	add	eax,4
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;
.startit:

	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT .unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:
	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that adds 4*2 integers.
	    
	add	eax, 32				; 
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
	
	pand	mm0, mm1			

	movq	mm2,  [ecx+eax-24]	
	movq	mm3,  [eax-24]		;
	movq	 [eax-32], mm0		;
	
	pand	mm2, mm3			;
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	movq	 [eax-24], mm2
	
	pand	mm0, mm1			;
	
	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	movq	 [eax-16], mm0		;

	pand	mm2, mm3			;
	
	dec	edx				;
	movq	 [eax-8], mm2		;
	jne	SHORT .ltop			;
.unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,7				; calculate .remaining 
	shr	edx,1				; number of integers
	jz	.norest				; 
.lrest:
	movq	mm0,  [ecx+eax]	; handle two ints
	movq	mm1,  [eax]		;
	add	eax,8
	pand	mm0, mm1			;
	movq	 [eax-8], mm0		;
	dec	edx				;
	jne	.lrest				;
.norest:
	
	emms					;
	test	bl,1				;
	jz	.end				; No--> we are finished
	mov	edx,  [ecx+eax]	; handle the .remaining integer
	and	edx,  [eax]		;
	mov	[eax],edx			;
.end:
	pop	ebx
	ret	



; void band_short_sse(short *inbuf,short* inoutbuf,unsigned int len) 
global band_short_sse
band_short_sse:
	push	ebx
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT			; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	.remain
		
.al_top:	
	movq	mm1,  [eax]		; Add shorts
	movq	mm0,  [ecx+eax]	; 4 at a time
	add	eax,8
	pand	mm0, mm1			;
	movq	 [eax-8], mm0		;
	
	dec	edx
	jne	.al_top
.remain:
	and	bl,3
	jz	.startit
.al_rest:
	mov	dx,  [ecx+eax]		; Add .remaining
	and	dx,  [eax]		; shorts
	mov	 [eax],dx
	add	eax,2
	dec	bl
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 4				; len /= 16
	jz	SHORT .unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:
	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that adds 4*4 shorts.
	    
	add	eax, 32				; We try to interleave some
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
	
	pand	mm0, mm1			;

	movq	mm2,  [ecx+eax-24]	
	movq	mm3,  [eax-24]		;
	movq	 [eax-32], mm0		;

	pand	mm2, mm3			;

	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	movq	 [eax-24], mm2

	pand	mm0, mm1			;	

	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	movq	 [eax-16], mm0		;

	pand	mm2, mm3			;

	dec	edx				;
	movq	 [eax-8], mm2		;
	
	jne	SHORT .ltop			;

.unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,15				; calculate .remaining 
	shr	edx,2				; number of shorts
	jz	.norest				; 

.lrest:
	movq	mm0,  [ecx+eax]	; add 4 shorts
	movq	mm1,  [eax]		;
	pand	mm0, mm1			;
	add	eax,8	
	movq	 [eax-8], mm0		;
	dec	edx				;
	jne	.lrest				;
.norest:
	
	emms					; restore FP state
	
	and	bl,3				; calculate .remaining shorts
	jz	.end				; 0 --> we are finished
.lshort:
	mov	dx,  [ecx+eax]		; Add the .remaining integer
	and	dx,  [eax]		;
	mov	 [eax],dx		;
	add	eax,2
	dec	bl
	jnz	.lshort

.end:
	pop	ebx
	ret	




; void band_byte_sse(char *inbuf,char *inoutbuf,unsigned int len) 
global band_byte_sse
band_byte_sse:	
	push	ebx
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of bytes 
	sub	edx,eax				; that have to be handled	
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,3				; divide by 8
	jz	.remain
		
.al_top:	
	movq	mm1,  [eax]		; Add bytes
	movq	mm0,  [ecx+eax]	; 8 at a time
	add	eax,8
	pand	mm0, mm1			;
	movq	 [eax-8], mm0		;
	
	dec	edx
	jne	.al_top
.remain:
	and	bl,7
	jz	.startit

.al_rest:
	mov	dl, [ecx+eax]		; Add .remaining
	and	dl, [eax]		; bytes
	mov	[eax],dl
	inc	eax
	dec	bl
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 5				; len /= 32
	jz	SHORT .unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:
	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that adds 4*8 bytes.
	    
	add	eax, 32				
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
						
	pand	mm0, mm1			;
	
	movq	mm2,  [ecx+eax-24]	
	movq	mm3,  [eax-24]		;
	movq	 [eax-32], mm0		;
	
	pand	mm2, mm3			;
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	movq	 [eax-24], mm2
	
	pand	mm0, mm1			;

	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	movq	 [eax-16], mm0		;
	
	pand	mm2, mm3			;

	dec	edx				;
	movq	 [eax-8], mm2		;
	
	jne	SHORT .ltop			;

.unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,31				; calculate .remaining 
	shr	edx,3				; number of bytes
	jz	.norest				; 

.lrest:
	movq	mm0,  [ecx+eax]	; add 8 bytes
	movq	mm1,  [eax]		;
	pand	mm0, mm1			;
	add	eax,8
	movq	 [eax-8], mm0		;
	dec	edx				;
	jne	.lrest				;
.norest:
	
	emms					; restore FP state
	
	and	bl,7				; calculate .remaining bytes
	jz	.end				; 0 --> we are finished
.lbyte:
	mov	dl,  [ecx+eax]		; Add the .remaining bytes
	and	dl,  [eax]		;
	mov	 [eax],dl		;
	inc	eax
	dec	bl
	jnz	.lbyte

.end:
	pop	ebx
	ret	




; void bor_int_sse(int *inbuf,int*inoutbuf,unsigned int len) 
global bor_int_sse
bor_int_sse:	
	push	ebx
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_INT			; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,1				; divide by 2
	jz	.remain
	
	
.al_top:	
	movq	mm1,  [eax]		; handle integers
	movq	mm0,  [ecx+eax]	; 2 at a time
	por	mm0, mm1			;
	add	eax,8
	movq	 [eax-8], mm0		;
	
	dec	edx
	jne	.al_top
.remain:
	test	bl,1
	jz	.startit

	mov	edx, [ecx+eax]		; handle .remaining
	or	edx, [eax]		; integer
	mov	 [eax],edx
	add	eax,4
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;
.startit:

	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT .unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:
	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that adds 4*2 integers.
	    
	add	eax, 32			
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
	por	mm0, mm1			;
	
	movq	mm2,  [ecx+eax-24]
	movq	mm3,  [eax-24]		;
	movq	 [eax-32], mm0		;
	
	por	mm2, mm3			;
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	movq	 [eax-24], mm2
	
	por	mm0, mm1			;
	    
	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	movq	 [eax-16], mm0		;
	
	por	mm2, mm3			;
	dec	edx				;
	movq	 [eax-8], mm2		;
	
	jne	SHORT .ltop			;
.unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,7				; calculate .remaining 
	shr	edx,1				; number of integers
	jz	.norest				; 
.lrest:
	movq	mm0,  [ecx+eax]	; handle two ints
	movq	mm1,  [eax]		;
	por	mm0, mm1			;
	add	eax,8
	movq	 [eax-8], mm0		;
	dec	edx				;
	jne	.lrest				;
.norest:
	
	emms					;
	test	bl,1				;
	jz	.end				; No--> we are finished
	mov	edx,  [ecx+eax]	; handle the .remaining integer
	or	edx,  [eax]		;
	mov	 [eax],edx			;
.end:
	pop	ebx
	ret	




; void bor_short_sse(short *inbuf,short* inoutbuf,unsigned int len) 
global bor_short_sse
bor_short_sse:
	push	ebx
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT			; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	.remain
		
.al_top:	
	movq	mm1,  [eax]		; Add shorts
	movq	mm0,  [ecx+eax]	; 4 at a time
	add	eax,8
	por	mm0, mm1			;
	movq	 [eax-8], mm0		;
	
	dec	edx
	jne	.al_top
.remain:
	and	bl,3
	jz	.startit
.al_rest:
	mov	dx,  [ecx+eax]		; Add .remaining
	or	dx,  [eax]		; shorts
	mov	 [eax],dx
	add	eax,2
	dec	bl
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 4				; len /= 16
	jz	SHORT .unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:
	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that adds 4*4 shorts.
	    
	add	eax, 32		
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
	
	por	mm0, mm1			;

	movq	mm2,  [ecx+eax-24]
	movq	mm3,  [eax-24]		;
	movq	 [eax-32], mm0		;

	por	mm2, mm3			;

	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	movq	 [eax-24], mm2
	
	por	mm0, mm1			;

	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	movq	 [eax-16], mm0		;
	
	por	mm2, mm3			;
	dec	edx				;
	movq	 [eax-8], mm2		;
	
	jne	SHORT .ltop			;

.unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,15				; calculate .remaining 
	shr	edx,2				; number of shorts
	jz	.norest				; 

.lrest:
	movq	mm0,  [ecx+eax]	; add 4 shorts
	movq	mm1,  [eax]		;
	por	mm0, mm1			;
	add	eax,8
	movq	 [eax-8], mm0		;
	dec	edx				;
	jne	.lrest				;
.norest:
	
	emms					; restore FP state
	
	and	bl,3				; calculate .remaining shorts
	jz	.end				; 0 --> we are finished
.lshort:
	mov	dx,  [ecx+eax]		; Add the .remaining integer
	or	dx,  [eax]		;
	mov	 [eax],dx		;
	add	eax,2
	dec	bl
	jnz	.lshort

.end:
	pop	ebx
	ret	




; void bor_byte_sse(char *inbuf,char *inoutbuf,unsigned int len)
global bor_byte_sse
bor_byte_sse:	
	push	ebx
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of bytes 
	sub	edx,eax				; that have to be handled	
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,3				; divide by 8
	jz	.remain
		
.al_top:	
	movq	mm1,  [eax]		; handle bytes
	movq	mm0,  [ecx+eax]	; 8 at a time
	add	eax,8
	por	mm0, mm1			;
	movq	 [eax-8], mm0		;
	
	dec	edx
	jne	.al_top
.remain:
	and	bl,7
	jz	.startit

.al_rest:
	mov	dl, [ecx+eax]		; Add .remaining
	or	dl, [eax]		; bytes
	mov	[eax],dl
	inc	eax
	dec	bl
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 5				; len /= 32
	jz	SHORT .unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:
	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that adds 4*8 bytes.
	    
	add	eax, 32		
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	por	mm0, mm1			;
						
	movq	mm2,  [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3,  [eax-24]		;
	movq	 [eax-32], mm0		;
	
	por	mm2, mm3			;

	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	movq	 [eax-24], mm2
    
	por	mm0, mm1			;

	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	movq	 [eax-16], mm0		;

	por	mm2, mm3			;
	dec	edx				;
	movq	 [eax-8], mm2		;
	
	jne	SHORT .ltop			;

.unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,31				; calculate .remaining 
	shr	edx,3				; number of bytes
	jz	.norest				; 

.lrest:
	movq	mm0,  [ecx+eax]	; add 8 bytes
	movq	mm1,  [eax]		;
	por	mm0, mm1			;
	add	eax,8
	movq	 [eax-8], mm0		;
	dec	edx				;
	jne	.lrest				;
.norest:
	
	emms					; restore FP state
	
	and	bl,7				; calculate .remaining bytes
	jz	.end				; 0 --> we are finished
.lbyte:
	mov	dl,  [ecx+eax]		; Add the .remaining bytes
	or	dl,  [eax]		;
	mov	 [eax],dl		;
	inc	eax
	dec	bl
	jnz	.lbyte

.end:
	pop	ebx
	ret	




; void bxor_int_sse(int *inbuf,int*inoutbuf,unsigned int len)
global bxor_int_sse
bxor_int_sse:	
	push	ebx
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_INT			; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,1				; divide by 2
	jz	.remain
	
	
.al_top:	
	movq	mm1,  [eax]		; handle integers
	movq	mm0,  [ecx+eax]	; 2 at a time
	add	eax,8
	pxor	mm0, mm1			;
	movq	 [eax-8], mm0		;
	
	dec	edx
	jne	.al_top
.remain:
	test	bl,1
	jz	.startit

	mov	edx, [ecx+eax]		; handle .remaining
	xor	edx, [eax]		; integer
	mov	 [eax],edx
	add	eax,4
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;
.startit:

	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	SHORT .unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		
align 4
.ltop:
	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that adds 4*2 integers.
	    
	add	eax, 32				
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2,  [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3,  [eax-24]		;
	  
	pxor	mm0, mm1			;
	movq	 [eax-32], mm0		;
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	
	pxor	mm2, mm3			;
	movq	 [eax-24], mm2
	
	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	
	pxor	mm0, mm1			;
	movq	 [eax-16], mm0		;
	
	dec	edx				;
	pxor	mm2, mm3			;
	movq	 [eax-8], mm2		;
	
	jne	SHORT .ltop			;
.unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,7				; calculate .remaining 
	shr	edx,1				; number of integers
	jz	.norest				; 
.lrest:
	movq	mm0,  [ecx+eax]	; handle two ints
	movq	mm1,  [eax]		;
	add	eax,8
	pxor	mm0, mm1			;
	movq	 [eax-8], mm0		;
	dec	edx				;
	jne	.lrest				;
.norest:
	
	emms					;
	test	bl,1				;
	jz	.end				; No--> we are finished
	mov	edx,  [ecx+eax]	; handle the .remaining integer
	xor	edx,  [eax]		;
	mov	[eax],edx			;
.end:
	pop	ebx
	ret	




; void bxor_short_sse(short *inbuf,short* inoutbuf,unsigned int len)
global bxor_short_sse
bxor_short_sse:	
	push	ebx
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of shorts 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT			; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	.remain
		
.al_top:	
	movq	mm1,  [eax]		; Add shorts
	movq	mm0,  [ecx+eax]	; 4 at a time
	add	eax,8
	pxor	mm0, mm1			;
	movq	 [eax-8], mm0		;
	
	dec	edx
	jne	.al_top
.remain:
	and	bl,3
	jz	.startit
.al_rest:
	mov	dx,  [ecx+eax]		; Add .remaining
	xor	dx,  [eax]		; shorts
	mov	 [eax],dx
	add	eax,2
	dec	bl
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 4				; len /= 16
	jz	SHORT .unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		
align 4
.ltop:
	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that adds 4*4 shorts.
	    
	add	eax, 32				
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2,  [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3,  [eax-24]		;
	  
	pxor	mm0, mm1			;
	movq	 [eax-32], mm0		;
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	
	pxor	mm2, mm3			;
	movq	 [eax-24], mm2
	
	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	
	pxor	mm0, mm1			;
	movq	 [eax-16], mm0		;
	
	dec	edx				;
	pxor	mm2, mm3			;
	movq	 [eax-8], mm2		;
	
	jne	SHORT .ltop			;

.unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,15				; calculate .remaining 
	shr	edx,2				; number of shorts
	jz	.norest				; 

.lrest:
	movq	mm0,  [ecx+eax]	; add 4 shorts
	movq	mm1,  [eax]		;
	add	eax,8
	pxor	mm0, mm1			;
	movq	 [eax-8], mm0		;
	dec	edx				;
	jne	.lrest				;
.norest:
	
	emms					; restore FP state
	
	and	bl,3				; calculate .remaining shorts
	jz	.end				; 0 --> we are finished
.lshort:
	mov	dx,  [ecx+eax]		; Add the .remaining integer
	xor	dx,  [eax]		;
	mov	 [eax],dx		;
	add	eax,2
	dec	bl
	jnz	.lshort

.end:
	pop	ebx
	ret	



; void bxor_byte_sse(char *inbuf,char *inoutbuf,unsigned int len)
global bxor_byte_sse
bxor_byte_sse:	
	push	ebx
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	.aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of bytes 
	sub	edx,eax				; that have to be handled	
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value
.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,3				; divide by 8
	jz	.remain
		
.al_top:	
	movq	mm1,  [eax]		; handle bytes
	movq	mm0,  [ecx+eax]	; 8 at a time
	add	eax,8
	pxor	mm0, mm1			;
	movq	 [eax-8], mm0		;
	
	dec	edx
	jne	.al_top
.remain:
	and	bl,7
	jz	.startit

.al_rest:
	mov	dl, [ecx+eax]		; Add .remaining
	xor	dl, [eax]		; bytes
	mov	[eax],dl
	inc	eax
	dec	bl
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 5				; len /= 32
	jz	SHORT .unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		
align 4
.ltop:
	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that adds 4*8 bytes.
	    
	add	eax, 32				
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2,  [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3,  [eax-24]		;
	  
	pxor	mm0, mm1			;
	movq	 [eax-32], mm0		;
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	
	pxor	mm2, mm3			;
	movq	 [eax-24], mm2
	
	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	
	pxor	mm0, mm1			;
	movq	 [eax-16], mm0		;
	
	dec	edx				;
	pxor	mm2, mm3			;
	movq	 [eax-8], mm2		;
	
	jne	SHORT .ltop			;

.unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,31				; calculate .remaining 
	shr	edx,3				; number of bytes
	jz	.norest				; 

.lrest:
	movq	mm0,  [ecx+eax]	; add 8 bytes
	movq	mm1,  [eax]		;
	add	eax,8
	pxor	mm0, mm1			;
	movq	 [eax-8], mm0		;
	dec	edx				;
	jne	.lrest				;
.norest:
	
	emms					; restore FP state
	
	and	bl,7				; calculate .remaining bytes
	jz	.end				; 0 --> we are finished
.lbyte:
	mov	dl,  [ecx+eax]		; Add the .remaining bytes
	xor	dl,  [eax]		;
	mov	 [eax],dl		;
	inc	eax
	dec	bl
	jnz	.lbyte

.end:
	pop	ebx
	ret	




; void lxor_int_sse(int *inbuf,int*inoutbuf,unsigned int len)
global lxor_int_sse
lxor_int_sse:
	push	ebx
	
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	NEAR .aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_INT			; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value

.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]

	sub	ecx, eax			;
	shr	edx,1				; divide by 2
	jz	.remain
	movq	mm6,[oneInt]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	
.al_top:	
	movq	mm1,  [eax]		; handle integers
	movq	mm0,  [ecx+eax]	; 2 at a time
	add	eax,8
	pcmpeqd mm0,mm4				; mm0 == 0?
	pcmpeqd mm1,mm4				; mm1 == 0?
	pxor	mm0,mm1				; res1 ^ res2
	pand	mm0,mm6				; reset all bits except first
	movq	 [eax-8], mm0		;
	
	dec	edx
	jne	.al_top

.remain:
	test	bl,1
	jz	.startit

	mov	edx, [ecx+eax]		; handle .remaining
	test	edx,edx				; integer
	je	.aiszero
	cmp	DWORD [eax],0
	jz	.resture
	xor	edx,edx
	jmp	.result				; false
.aiszero:
	cmp	DWORD [eax],0
	je	.nomove				; false, since edx contains 0
.resture:
	mov	edx,1
.result:
	mov	 [eax],edx
.nomove:	
	add	eax,4
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	movq	mm6,[oneInt]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	NEAR .unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:

	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that adds 4*2 integers.
		
	add	eax, 32				
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2,  [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3,  [eax-24]		;
	  
	pcmpeqd mm0,mm4
	pcmpeqd mm1,mm4
	pxor	mm0,mm1
	pand	mm0,mm6;
	movq	 [eax-32], mm0		;
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	
	pcmpeqd mm2,mm4
	pcmpeqd mm3,mm4
	pxor	mm2,mm3
	pand	mm2,mm6
	movq	 [eax-24], mm2
	
	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	
	pcmpeqd mm0,mm4
	pcmpeqd mm1,mm4
	pxor	mm0,mm1
	pand	mm0,mm6
	movq	 [eax-16], mm0		;
	
	dec	edx				;
	pcmpeqd mm2,mm4
	pcmpeqd mm3,mm4
	pxor	mm2,mm3
	pand	mm2,mm6
	movq	 [eax-8], mm2		;	
	jne	NEAR .ltop			;

.unroll_end:	
	mov	dl, bl				; load original length
	and	dl,7				; calculate .remaining 
	shr	dl,1				; number of integers
	jz	.norest				; 
align 4
.lrest:
	movq	mm0,  [ecx+eax]	; handle two ints
	movq	mm1,  [eax]		;
	add	eax,8
	pcmpeqd mm0,mm4
	pcmpeqd mm1,mm4
	pxor	mm0,mm1
	dec	dl
	pand	mm0,mm6
	movq	 [eax-8], mm0		;
	jne	.lrest				;
.norest:
	
	emms					;
	test	bl,1				;
	jz	.end				; No--> we are finished
	
	mov	edx,  [ecx+eax]	; handle the .remaining integer
	test	edx,edx				
	je	.zero
	cmp	DWORD [eax],0
	jz	.resture1
	xor	edx,edx				; edx = 0
	jmp	.result1				; false
.zero:
	cmp	DWORD [eax],0
	je	.end				; false, since inoutbuf already contains 0
.resture1:
	mov	edx,1
.result1:
	mov	 [eax],edx
.end:
	pop	ebx
	ret	



; void lxor_short_sse(short *inbuf,short *inoutbuf,unsigned int len)
global lxor_short_sse
lxor_short_sse:
	push	ebx
	
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	NEAR .aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT			; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value

.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	.remain
	movq	mm6,[oneShort]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	
.al_top:	
	movq	mm1,  [eax]		; handle shorts
	movq	mm0,  [ecx+eax]	; 4 at a time
	add	eax,8
	pcmpeqw mm0,mm4				; mm0 == 0?
	pcmpeqw mm1,mm4				; mm1 == 0?
	pxor	mm0,mm1				; res1 ^ res2
	pand	mm0,mm6				; reset all bits except first
	movq	 [eax-8], mm0		;
	
	dec	edx
	jne	.al_top

.remain:
	and	bl,3
	jz	.startit

.al_rest:
	mov	dx,  [ecx+eax]		; handle .remaining
	test	dx,dx				; integer
	je	.aiszero
	cmp	WORD [eax],0
	jz	.resture
	xor	dx,dx
	jmp	.result				; false
.aiszero:
	cmp	WORD [eax],0
	je	.nomove				; false, since inoutbuf already contains 0
.resture:
	mov	dx,1
.result:
	mov	 [eax],dx
.nomove:	
	add	eax,2
	dec	bl
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	movq	mm6,[oneShort]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 4				; len /= 16
	jz	NEAR .unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:

	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that handles 4*4 shorts.
		
	add	eax, 32
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2,  [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3,  [eax-24]		;
	  
	pcmpeqw mm0,mm4
	pcmpeqw mm1,mm4
	pxor	mm0,mm1
	pand	mm0,mm6;
	movq	 [eax-32], mm0		;
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	
	pcmpeqw mm2,mm4
	pcmpeqw mm3,mm4
	pxor	mm2,mm3
	pand	mm2,mm6
	movq	 [eax-24], mm2
	
	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	
	pcmpeqw mm0,mm4
	pcmpeqw mm1,mm4
	pxor	mm0,mm1
	pand	mm0,mm6
	movq	 [eax-16], mm0		;
	
	dec	edx				;
	pcmpeqw mm2,mm4
	pcmpeqw mm3,mm4
	pxor	mm2,mm3
	pand	mm2,mm6
	movq	 [eax-8], mm2		;	
	jne	NEAR .ltop			;

.unroll_end:	
	mov	dl, bl				; load original length
	and	dl,15				; calculate .remaining 
	shr	dl,2				; number of shorts
	jz	.norest				; 
.lrest:
	movq	mm0,  [ecx+eax]	; handle 4 shorts
	movq	mm1,  [eax]		;
	add	eax,8
	pcmpeqw mm0,mm4
	pcmpeqw mm1,mm4
	pxor	mm0,mm1
	dec	dl
	pand	mm0,mm6
	movq	 [eax-8], mm0		;
	jne	.lrest				;
.norest:
	
	emms					;
	and	bl,3				; Anything left?
	jz	.end				; No--> we are finished
align 4
.lrest1:
	mov	dx,  [ecx+eax]		; handle the .remaining shorts
	test	dx,dx				
	je	.zero
	cmp	WORD [eax],0
	jz	.resture1
	xor	dx,dx				; edx = 0
	jmp	.result1				; false
.zero:
	cmp	WORD [eax],0
	je	.nomove1				; false, since inoutbuf already contains 0
.resture1:
	mov	dx,1
.result1:
	mov	[eax],dx
.nomove1:
	add	eax,2
	dec	bl
	jnz	.lrest1

.end:
	pop	ebx
	ret	




; void lxor_byte_sse(char *inbuf,char *inoutbuf,unsigned int len)
global lxor_byte_sse
lxor_byte_sse:
	push	ebx
	
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	NEAR .aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
						; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value

.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,3				; divide by 8
	jz	.remain
	movq	mm6,[oneByte]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
align 4	
.al_top:	
	movq	mm1,  [eax]		; handle bytes
	movq	mm0,  [ecx+eax]	; 8 at a time
	add	eax,8
	pcmpeqb mm0,mm4				; mm0 == 0?
	pcmpeqb mm1,mm4				; mm1 == 0?
	pxor	mm0,mm1				; res1 ^ res2
	pand	mm0,mm6				; reset all bits except first
	movq	 [eax-8], mm0		;
	
	dec	edx
	jne	.al_top

.remain:
	and	bl,7
	jz	.startit

.al_rest:
	mov	dl, [ecx+eax]		; handle .remaining
	test	dl,dl				; integer
	je	.aiszero
	cmp	BYTE [eax],0
	jz	.resture
	xor	dl,dl
	jmp	.result				; false
.aiszero:
	cmp	BYTE [eax],0
	je	.nomove				; false, since inoutbuf already 0
.resture:
	mov	dl,1
.result:
	mov	 [eax],dl
.nomove:	
	inc	eax
	dec	bl
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	movq	mm6,[oneByte]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 5				; len /= 32
	jz	NEAR .unroll_end		; len==0 --> less than 32 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:

	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that handles 4*8 bytes.
		
	add	eax, 32
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2,  [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3,  [eax-24]		;
	  
	pcmpeqb mm0,mm4
	pcmpeqb mm1,mm4
	pxor	mm0,mm1
	pand	mm0,mm6;
	movq	 [eax-32], mm0		;
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	
	pcmpeqb mm2,mm4
	pcmpeqb mm3,mm4
	pxor	mm2,mm3
	pand	mm2,mm6
	movq	 [eax-24], mm2
	
	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	
	pcmpeqb mm0,mm4
	pcmpeqb mm1,mm4
	pxor	mm0,mm1
	pand	mm0,mm6
	movq	 [eax-16], mm0		;
	
	dec	edx				;
	pcmpeqb mm2,mm4
	pcmpeqb mm3,mm4
	pxor	mm2,mm3
	pand	mm2,mm6
	movq	 [eax-8], mm2		;	
	jne	NEAR .ltop			;

.unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,31				; calculate .remaining 
	shr	edx,3				; number of shorts
	jz	.norest				; 
.lrest:
	movq	mm0,  [ecx+eax]	; handle 8 bytes
	movq	mm1,  [eax]		;
	add	eax,8
	pcmpeqb mm0,mm4
	pcmpeqb mm1,mm4
	pxor	mm0,mm1
	pand	mm0,mm6
	movq	 [eax-8], mm0		;
	dec	edx				;
	jne	.lrest				;
.norest:
	
	emms					;
	and	bl,7				; Anything left?
	jz	.end				; No--> we are finished
align 4
.lrest1:
	mov	dl,  [ecx+eax]		; handle the .remaining bytes
	test	dl,dl				
	je	.zero
	cmp	BYTE [eax],0
	jz	.resture1
	xor	dl,dl				; edx = 0
	jmp	.result1				; false
.zero:
	cmp	BYTE [eax],0
	je	.nomove1				; false, since inoutbuf already contains 0
.resture1:
	mov	dl,1
.result1:	
	mov	 [eax],dl
.nomove1:
	inc	eax
	dec	bl
	jnz	.lrest1

.end:
	pop	ebx
	ret	




; void land_int_sse(int *inbuf,int*inoutbuf,unsigned int len)
global land_int_sse
land_int_sse:
	push	ebx
	
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	NEAR .aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_INT			; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value

.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]

	sub	ecx, eax			;
	shr	edx,1				; divide by 2
	jz	.remain
	movq	mm6,[oneInt]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	
.al_top:	
	movq	mm1,  [eax]		; handle integers
	movq	mm0,  [ecx+eax]	; 2 at a time
	add	eax,8
	pcmpeqd mm0,mm4				; mm0 == 0?
	pcmpeqd mm1,mm4				; mm1 == 0?
	pandn	mm1,mm6
	pandn	mm0,mm1				; res1 ^ res2
	dec	edx
	
	movq	 [eax-8], mm0		;	
	jne	.al_top

.remain:
	test	bl,1
	jz	.startit

	mov	edx, [ecx+eax]		; handle .remaining
	test	edx,edx				; integer
	je	.result				; false, since edx contains zero
	cmp	DWORD [eax],0
	je	.nomove				; false
	mov	edx,1
.result:
	mov	 [eax],edx	
.nomove:
	add	eax,4
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	movq	mm6,[oneInt]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	NEAR .unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]

align 4
.ltop:

	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that adds 4*2 integers.
		
	add	eax, 32		
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2,  [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3,  [eax-24]		;
	  
	pcmpeqd mm0,mm4
	pcmpeqd mm1,mm4
	pandn	mm1,mm6	
	pandn	mm0,mm1
	movq	 [eax-32], mm0		;
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	
	pcmpeqd mm2,mm4
	pcmpeqd mm3,mm4
	pandn	mm3,mm6
	pandn	mm2,mm3
	
	movq	 [eax-24], mm2
	
	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	
	pcmpeqd mm0,mm4
	pcmpeqd mm1,mm4
	pandn	mm1,mm6
	pandn	mm0,mm1
	
	movq	 [eax-16], mm0		;
	
	dec	edx				;
	pcmpeqd mm2,mm4
	pcmpeqd mm3,mm4
	pandn	mm3,mm6
	pandn	mm2,mm3
	movq	 [eax-8], mm2		;	
	jne	NEAR .ltop			;

.unroll_end:	
	mov	dl, bl				; load original length
	and	dl,7				; calculate .remaining 
	shr	dl,1				; number of integers
	jz	.norest				; 
align 4
.lrest:
	movq	mm0,  [ecx+eax]	; handle two ints
	movq	mm1,  [eax]		;
	add	eax,8
	pcmpeqd mm0,mm4
	pcmpeqd mm1,mm4
	pandn	mm1,mm6	
	pandn	mm0,mm1
	dec	dl
	
	movq	 [eax-8], mm0		;
	jne	.lrest				;
.norest:
	
	emms					;
	test	bl,1				;
	jz	.end				; No--> we are finished
	
	mov	edx,  [ecx+eax]	; handle the .remaining integer
	test	edx,edx				
	je	.result1				; false
	cmp	DWORD [eax],0
	je	.end				; false
	mov	edx,1
.result1:
	mov	 [eax],edx
.end:
	pop	ebx
	ret	



; void land_short_sse(short *inbuf,short *inoutbuf,unsigned int len)
global land_short_sse
land_short_sse:
	push	ebx
	
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	NEAR .aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT			; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value

.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	.remain
	movq	mm6,[oneShort]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	
.al_top:	
	movq	mm1,  [eax]		; handle shorts
	movq	mm0,  [ecx+eax]	; 4 at a time
	add	eax,8
	pcmpeqw mm0,mm4				; mm0 == 0?
	pcmpeqw mm1,mm4				; mm1 == 0?
	pandn	mm1,mm6
	dec	edx
	pandn	mm0,mm1				; 
	movq	 [eax-8], mm0		;
	
	jne	.al_top

.remain:
	and	bl,3
	jz	.startit

.al_rest:
	mov	dx,  [ecx+eax]		; handle .remaining
	test	dx,dx				; integer
	je	.result				; false
	cmp	WORD [eax],0
	je	.nomove				; false
	mov	dx,1				; true
.result:
	mov	 [eax],dx	
.nomove:
	add	eax,2
	dec	bl
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	movq	mm6,[oneShort]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 4				; len /= 16
	jz	NEAR .unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:

	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that handles 4*4 shorts.
		
	add	eax, 32				
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2,  [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3,  [eax-24]		;
	  
	pcmpeqw mm0,mm4
	pcmpeqw mm1,mm4
	pandn	mm1,mm6
	pandn	mm0,mm1
	movq	 [eax-32], mm0		;
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	
	pcmpeqw mm2,mm4
	pcmpeqw mm3,mm4
	pandn	mm3,mm6
	pandn	mm2,mm3
	movq	 [eax-24], mm2
	
	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	
	pcmpeqw mm0,mm4
	pcmpeqw mm1,mm4
	pandn	mm1,mm6
	pandn	mm0,mm1
	
	movq	 [eax-16], mm0		;
	
	dec	edx				;
	pcmpeqw mm3,mm4
	pcmpeqw mm2,mm4
	pandn	mm3,mm6
	pandn	mm2,mm3
	movq	 [eax-8], mm2		;	
	jne	NEAR .ltop			;

.unroll_end:	
	mov	dl, bl				; load original length
	and	dl,15				; calculate .remaining 
	shr	dl,2				; number of shorts
	jz	.norest				; 
.lrest:
	movq	mm0,  [ecx+eax]	; handle 4 shorts
	movq	mm1,  [eax]		;
	add	eax,8
	pcmpeqw mm0,mm4
	pcmpeqw mm1,mm4
	pandn	mm1,mm6

	dec	dl
	pandn	mm0,mm1	
	movq	 [eax-8], mm0		;
	jne	.lrest				;
.norest:
	
	emms					;
	and	bl,3				; Anything left?
	jz	.end				; No--> we are finished
align 4
.lrest1:
	mov	dx,  [ecx+eax]		; handle the .remaining shorts
	test	dx,dx				
	je	.result1				; false
	cmp	WORD [eax],0
	je	.nomove1
	mov	dx,1
.result1:
	mov	 [eax],dx
.nomove1:
	add	eax,2
	dec	bl
	jnz	.lrest1

.end:
	pop	ebx
	ret	



; void land_byte_sse(char *inbuf,char *inoutbuf,unsigned int len) 
global land_byte_sse
land_byte_sse:
	push	ebx
	
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	NEAR .aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
						; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value

.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]
	sub	ecx, eax			;
	shr	edx,3				; divide by 8
	jz	.remain
	movq	mm6,  [oneByte]	; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
align 4	
.al_top:	
	movq	mm1,  [eax]		; handle bytes
	movq	mm0,  [ecx+eax]	; 8 at a time
	add	eax,8
	pcmpeqb mm1,mm4				; mm1 == 0?
	pcmpeqb mm0,mm4				; mm0 == 0?	
			
	pandn	mm1,mm6				; invert mm1
	pandn	mm0,mm1				; 
	dec	edx
	movq	 [eax-8], mm0		;	

	jne	.al_top

.remain:
	and	bl,7
	jz	.startit

.al_rest:
	mov	dl, [ecx+eax]		; handle .remaining
	test	dl,dl				; integer
	je	.result				; false
	cmp	BYTE [eax],0
	je	.nomove				; false
	mov	dl,1
.result:
	mov	 [eax],dl
.nomove:	
	inc	eax
	dec	bl
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	movq	mm6,  [oneByte]	; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 5				; len /= 32
	jz	NEAR .unroll_end		; len==0 --> less than 32 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]
align 4
.ltop:

	movq	mm1,  [ecx+eax]	; This is an unrolled loop
	movq	mm0,  [eax]		; that handles 4*8 bytes.
		
	add	eax, 32
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2,  [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3,  [eax-24]		;
	  
	pcmpeqb mm1,mm4
	pcmpeqb mm0,mm4
	pandn	mm1,mm6
	pandn	mm0,mm1
	movq	 [eax-32], mm0		;
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	
	pcmpeqb mm3,mm4
	pcmpeqb mm2,mm4
	pandn	mm3,mm6
	pandn	mm2,mm3
	movq	 [eax-24], mm2
	
	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	
	pcmpeqb mm1,mm4
	pcmpeqb mm0,mm4
	pandn	mm1,mm6
	pandn	mm0,mm1	
	movq	 [eax-16], mm0		;
	
	pcmpeqb mm2,mm4
	pcmpeqb mm3,mm4
	dec	edx				;
	pandn	mm3,mm6
	pandn	mm2,mm3
	movq	 [eax-8], mm2		;	
	jne	NEAR .ltop			;

.unroll_end:	
	mov	dl, bl				; load original length
	and	dl,31				; calculate .remaining 
	shr	dl,3				; number of shorts
	jz	.norest				; 
.lrest:
	movq	mm0,  [ecx+eax]	; handle 8 bytes
	movq	mm1,  [eax]		;
	add	eax,8
	pcmpeqb mm0,mm4	
	pcmpeqb mm1,mm4
	dec	dl				;
	pandn	mm1,mm6
	pandn	mm0,mm1
	movq	 [eax-8], mm0		;

	jne	.lrest				;
.norest:
	
	emms					;
	and	bl,7				; Anything left?
	jz	.end				; No--> we are finished
align 4
.lrest1:
	mov	dl,  [ecx+eax]		; handle the .remaining bytes
	test	dl,dl				
	je	.result1				; false
	cmp	BYTE [eax],0
	je	.nomove1				; false
	mov	dl,1
.result1:
	mov	 [eax],dl
.nomove1:	
	inc	eax
	dec	bl
	jnz	.lrest1

.end:
	pop	ebx
	ret	




; void lor_int_sse(int *inbuf,int*inoutbuf,unsigned int len)
global lor_int_sse
lor_int_sse:
	push	ebx
	
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	NEAR .aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_INT			; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value

.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]

	sub	ecx, eax			;
	shr	edx,1				; divide by 2
	jz	.remain
	movq	mm6,[oneInt]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	
.al_top:	
	movq	mm1,  [eax]		; handle integers
	movq	mm0,  [ecx+eax]	; 2 at a time
	add	eax,8
	pcmpeqd mm0,mm4				; mm0 == 0?
	pcmpeqd mm1,mm4				; mm1 == 0?
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1				; res1 || res2
	dec	edx
	movq	 [eax-8], mm0		;	
	jne	.al_top

.remain:
	test	bl,1
	jz	.startit

	mov	edx, [ecx+eax]		; handle .remaining
	test	edx,edx				; integer
	jne	.resture				; true
	cmp	DWORD [eax],0
	je	.nomove				; false
.resture:
	mov	DWORD [eax],1	
.nomove:
	add	eax,4
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	movq	mm6,[oneInt]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 3				; len /= 8
	jz	NEAR .unroll_end		; len==0 --> less than 8 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		
align 4
.ltop:

	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that adds 4*2 integers.
		
	add	eax, 32				
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2,  [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3,  [eax-24]		;
	  
	pcmpeqd mm0,mm4
	pcmpeqd mm1,mm4
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1
	movq	 [eax-32], mm0		;
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	
	pcmpeqd mm2,mm4
	pcmpeqd mm3,mm4
	pandn	mm2,mm6				; invert mm2
	pandn	mm3,mm6				; and mm3
	por	mm2,mm3
	movq	 [eax-24], mm2
	
	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	
	pcmpeqd mm0,mm4
	pcmpeqd mm1,mm4
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1
	movq	 [eax-16], mm0		;
	
	dec	edx				;
	pcmpeqd mm2,mm4
	pcmpeqd mm3,mm4
	pandn	mm2,mm6
	pandn	mm3,mm6
	por	mm2,mm3
	movq	 [eax-8], mm2		;	
	jne	NEAR .ltop			;

.unroll_end:	
	mov	dl, bl				; load original length
	and	dl,7				; calculate .remaining 
	shr	dl,1				; number of integers
	jz	.norest				; 
align 4
.lrest:
	movq	mm0,  [ecx+eax]	; handle two ints
	movq	mm1,  [eax]		;
	add	eax,8
	pcmpeqd mm0,mm4
	pcmpeqd mm1,mm4
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1
	dec	dl
	movq	 [eax-8], mm0		;
	jne	.lrest				;
.norest:
	
	emms					;
	test	bl,1				;
	jz	.end				; No--> we are finished
	
	mov	edx,  [ecx+eax]	; handle the .remaining integer
	test	edx,edx				
	jne	.resture1
	cmp	DWORD [eax],0
	je	.end				; false
.resture1:
	mov	DWORD [eax],1
.end:
	pop	ebx
	ret	




; void lor_short_sse(short *inbuf,short *inoutbuf,unsigned int len)
global lor_short_sse
lor_short_sse:
	push	ebx
	
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	NEAR .aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
	shr	edx,LN_SHORT			; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value

.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]

	sub	ecx, eax			;
	shr	edx,2				; divide by 4
	jz	.remain
	movq	mm6,[oneShort]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	
.al_top:	
	movq	mm1,  [eax]		; handle shorts
	movq	mm0,  [ecx+eax]	; 4 at a time
	add	eax,8
	pcmpeqw mm0,mm4				; mm0 == 0?
	pcmpeqw mm1,mm4				; mm1 == 0?
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1				; res1 || res2
	dec	edx
	movq	 [eax-8], mm0		;
	
	jne	.al_top

.remain:
	and	bl,3
	jz	.startit

.al_rest:
	mov	dx,  [ecx+eax]		; handle .remaining
	test	dx,dx				; shorts
	jne	.resture
	cmp	WORD [eax],0
	je	.nomove				; false
.resture:
	mov	WORD [eax],1	
.nomove:
	add	eax,2
	dec	bl
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	movq	mm6,[oneShort]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 4				; len /= 16
	jz	NEAR .unroll_end		; len==0 --> less than 16 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		
align 4
.ltop:

	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that handles 4*4 shorts.
		
	add	eax, 32				
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2,  [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3,  [eax-24]		;
	  
	pcmpeqw mm0,mm4
	pcmpeqw mm1,mm4
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1
	movq	 [eax-32], mm0		;
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	
	pcmpeqw mm2,mm4
	pcmpeqw mm3,mm4
	pandn	mm2,mm6				; invert mm2
	pandn	mm3,mm6				; and mm3
	por	mm2,mm3
	movq	 [eax-24], mm2
	
	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	
	pcmpeqw mm0,mm4
	pcmpeqw mm1,mm4
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1
	movq	 [eax-16], mm0		;
	
	dec	edx				;
	pcmpeqw mm2,mm4
	pcmpeqw mm3,mm4
	pandn	mm2,mm6				; invert mm2
	pandn	mm3,mm6				; and mm3
	por	mm2,mm3
	
	movq	 [eax-8], mm2		;	
	jne	NEAR .ltop			;

.unroll_end:	
	mov	dl, bl				; load original length
	and	dl,15				; calculate .remaining 
	shr	dl,2				; number of shorts
	jz	.norest				; 
.lrest:
	movq	mm0,  [ecx+eax]	; handle 4 shorts
	movq	mm1,  [eax]		;
	add	eax,8
	pcmpeqw mm0,mm4
	pcmpeqw mm1,mm4
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1
	dec	dl
	movq	 [eax-8], mm0		;
	jne	.lrest				;
.norest:
	
	emms					;
	and	bl,3				; Anything left?
	jz	.end				; No--> we are finished
align 4
.lrest1:
	mov	dx,  [ecx+eax]		; handle the .remaining shorts
	test	dx,dx				
	jne	.result1				; false
	cmp	WORD [eax],0
	je	.nomove1
.result1:
	mov	BYTE [eax],1
.nomove1:
	add	eax,2
	dec	bl
	jnz	.lrest1

.end:
	pop	ebx
	ret	




; void lor_byte_sse(char *inbuf,char *inoutbuf,unsigned int len)
global lor_byte_sse
lor_byte_sse:
	push	ebx
	
	mov	eax,  [esp+12]		; load inoutbuf
	mov	ecx,eax				; Save address of inoutbuf

	; Align buffer to hit an ALIGNMENT byte boundary
	
	and	eax,ALIGNEMENT-1		; eax = inoutbuf % ALIGNEMENT	
	jz	NEAR .aligned				; 0 --> Already .aligned
	
	mov	edx,ALIGNEMENT			; Calculate number of ints 
	sub	edx,eax				; that have to be handled
						; to get .aligned addresses
	
	mov	eax,ecx				; restore address of inoutbuf
	prefetchnta [eax+32]

	mov	ecx, [esp+16]		; Load length
	cmp	ecx,edx				; Is it smaller than the calculated number?
	jge	.match				;
	mov	edx,ecx				; yes, use smaller value

.match:
	sub	ecx,edx				; calculate new length
	mov	 [esp+16],ecx		; store new length

	mov	ebx, edx			; Save counter
	mov	ecx,  [esp+8]		; load inbuf
	prefetchnta [ecx+32]

	sub	ecx, eax			;
	shr	edx,3				; divide by 8
	jz	.remain
	movq	mm6,[oneByte]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
align 4	
.al_top:	
	movq	mm1,  [eax]		; handle bytes
	movq	mm0,  [ecx+eax]	; 8 at a time
	add	eax,8
	pcmpeqb mm0,mm4				; mm0 == 0?
	pcmpeqb mm1,mm4				; mm1 == 0?
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1				; res1 ^ res2
	dec	edx
	movq	 [eax-8], mm0		;
	
	jne	.al_top

.remain:
	and	bl,7
	jz	.startit

.al_rest:
	mov	dl, [ecx+eax]		; handle .remaining
	test	dl,dl				; integer
	jne	.resture
	cmp	BYTE [eax],0
	je	.nomove				; false	
.resture:
	mov	BYTE [eax],1
.nomove:	
	inc	eax
	dec	bl
	jnz	.al_rest
	jmp	.startit

.aligned:	
	mov	eax, ecx
	mov	ecx,  [esp+8]		; load inbuf
	sub	ecx, eax			;

.startit:
	movq	mm6,[oneByte]			; load ones to mm6
	pxor	mm4,mm4				; clear mm4 
	mov	edx,  [esp+16]		; Load length
	mov	ebx,edx				; Save it
	shr	edx, 5				; len /= 32
	jz	NEAR .unroll_end		; len==0 --> less than 32 numbers
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		
align 4
.ltop:

	movq	mm0,  [ecx+eax]	; This is an unrolled loop
	movq	mm1,  [eax]		; that handles 4*8 bytes.
		
	add	eax, 32				
	prefetchnta [eax+32]
	prefetchnta [ecx+eax+32]		; We try to interleave some
						; instructions to allow
	movq	mm2,  [ecx+eax-24]	; overlapping load/calculate operations.
	movq	mm3,  [eax-24]		;
	  
	pcmpeqb mm0,mm4
	pcmpeqb mm1,mm4
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1
	movq	 [eax-32], mm0		;
	
	movq	mm0,  [ecx+eax-16]	;
	movq	mm1,  [eax-16]		;
	
	pcmpeqb mm2,mm4
	pcmpeqb mm3,mm4
	pandn	mm2,mm6				; invert mm2
	pandn	mm3,mm6				; and mm3
	por	mm2,mm3
	movq	 [eax-24], mm2
	
	movq	mm2,  [ecx+eax-8]	;
	movq	mm3,  [eax-8]		;
	
	pcmpeqb mm0,mm4
	pcmpeqb mm1,mm4
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1
	movq	 [eax-16], mm0		;
	
	dec	edx				;
	pcmpeqb mm2,mm4
	pcmpeqb mm3,mm4
	pandn	mm2,mm6				; invert mm2
	pandn	mm3,mm6				; and mm3
	por	mm2,mm3
	movq	 [eax-8], mm2		;	
	jne	NEAR .ltop			;

.unroll_end:	
	mov	edx, ebx			; load original length
	and	edx,31				; calculate .remaining 
	shr	edx,3				; number of shorts
	jz	.norest				; 
.lrest:
	movq	mm0,  [ecx+eax]	; handle 8 bytes
	movq	mm1,  [eax]		;
	add	eax,8
	pcmpeqb mm0,mm4
	pcmpeqb mm1,mm4
	pandn	mm0,mm6				; invert mm0
	pandn	mm1,mm6				; and mm1
	por	mm0,mm1
	movq	 [eax-8], mm0		;
	dec	edx				;
	jne	.lrest				;
.norest:
	
	emms					;
	and	bl,7				; Anything left?
	jz	.end				; No--> we are finished
align 4
.lrest1:
	mov	dl,  [ecx+eax]		; handle the .remaining bytes
	test	dl,dl				
	jne	.result1				; true
	cmp	BYTE [eax],0
	je	.nomove1				; false
.result1:
	mov	BYTE [eax],1
.nomove1:	
	inc	eax
	dec	bl
	jnz	.lrest1

.end:
	pop	ebx
	ret	
