/* $Id$ */

/* MMX-optimized reduce functions in GCC inline assembler syntax */

#include "mpiimpl.h"
#include "x86_ops.h"
#include "mpiops.h"

/* This is used to align buffers to
   a given boundary. The algorithms assume that:
   1) Both buffers (in and inout) are equally aligned
   2) They are at least aligned to the size of the data type.
*/

#define ALIGNEMENT 32

#define LN_DOUBLE 3
#define LN_INT 2
#define LN_FLOAT 2
#define LN_SHORT 1

#if CPU_ARCH_IS_X86
/* ---------------------MPI_SUM----------------------------------*/
void add_error_func(void* d1,void*d2,unsigned int l) {
    MPIR_Op_errno = MPIR_ERRCLASS_TO_CODE(MPI_ERR_OP,MPIR_ERR_NOT_DEFINED);
    MPIR_ERROR(MPIR_COMM_WORLD,MPIR_Op_errno, "MPI_SUM" );
}

static void add_int_mmx(int *inbuf,int*inoutbuf,unsigned int len);
static void add_short_mmx(short *inbuf,short *inoutbuf,unsigned int len);
static void add_byte_mmx(char *inbuf,char *inoutbuf,unsigned int len);

static void add_float(float *inbuf,float* inoutbuf,unsigned int len) {
    unsigned int i,u;
    u=len&(~3);   
    for(i=0;i<u;i+=4) {
        inoutbuf[i]+=inbuf[i];
        inoutbuf[i+1]+=inbuf[i+1];
        inoutbuf[i+2]+=inbuf[i+2];
        inoutbuf[i+3]+=inbuf[i+3];
    }
    for(;i<len;++i)
        inoutbuf[i]+=inbuf[i];
}

static void add_double(double *inbuf,double* inoutbuf,unsigned int len) {
    unsigned int i,u;

    u=len&(~3);   
    for(i=0;i<u;i+=4) {
        inoutbuf[i]+=inbuf[i];
        inoutbuf[i+1]+=inbuf[i+1];
        inoutbuf[i+2]+=inbuf[i+2];
        inoutbuf[i+3]+=inbuf[i+3];
    }
    for(;i<len;++i)
        inoutbuf[i]+=inbuf[i];
}

static void add_s_complex(s_complex *inbuf,s_complex *inoutbuf,unsigned int len) {
    unsigned int i,u;
    u=len&(~3);   
    for(i=0;i<u;i+=4) {
        inoutbuf[i].re+=inbuf[i].re;
        inoutbuf[i].im+=inbuf[i].im;
        
        inoutbuf[i+1].re+=inbuf[i+1].re;
        inoutbuf[i+1].im+=inbuf[i+1].im;

        inoutbuf[i+2].re+=inbuf[i+2].re;
        inoutbuf[i+2].im+=inbuf[i+2].im;

        inoutbuf[i+3].re+=inbuf[i+3].re;
        inoutbuf[i+3].im+=inbuf[i+3].im;
    }
    for(;i<len;++i) {
        inoutbuf[i].re+=inbuf[i].re;
        inoutbuf[i].im+=inbuf[i].im;
    }
}

static void add_d_complex(d_complex *inbuf,d_complex *inoutbuf,unsigned int len) {
    unsigned int i,u;
    u=len&(~3);   
    for(i=0;i<u;i+=4) {
        inoutbuf[i].re+=inbuf[i].re;
        inoutbuf[i].im+=inbuf[i].im;
        
        inoutbuf[i+1].re+=inbuf[i+1].re;
        inoutbuf[i+1].im+=inbuf[i+1].im;

        inoutbuf[i+2].re+=inbuf[i+2].re;
        inoutbuf[i+2].im+=inbuf[i+2].im;

        inoutbuf[i+3].re+=inbuf[i+3].re;
        inoutbuf[i+3].im+=inbuf[i+3].im;
    }
    for(;i<len;++i) {
        inoutbuf[i].re+=inbuf[i].re;
        inoutbuf[i].im+=inbuf[i].im;
    }
}

/*
    MPIR_INT=0, MPIR_FLOAT, MPIR_DOUBLE, MPIR_COMPLEX, MPIR_LONG, MPIR_SHORT,
    MPIR_CHAR, MPIR_BYTE, MPIR_UCHAR, MPIR_USHORT, MPIR_ULONG, MPIR_UINT,
    MPIR_CONTIG, MPIR_VECTOR, MPIR_HVECTOR, 
    MPIR_INDEXED,
    MPIR_HINDEXED, MPIR_STRUCT, MPIR_DOUBLE_COMPLEX, MPIR_PACKED, 
        MPIR_UB, MPIR_LB, MPIR_LONGDOUBLE, MPIR_LONGLONGINT, 
    MPIR_LOGICAL, MPIR_FORT_INT 
*/

static const proto_func jmp_add_mmx[] = {
    add_int_mmx,    /*int*/
    add_float,      /*float*/
    add_double,     /*double*/
    add_s_complex,  /*complex*/
    add_int_mmx,    /*long*/
    add_short_mmx,  /*short*/
    add_byte_mmx,   /*char*/
    add_byte_mmx,   /*byte*/
    add_byte_mmx,   /*uchar*/
    add_short_mmx,  /*ushort*/
    add_int_mmx,    /*ulong*/
    add_int_mmx,    /*uint*/
    add_error_func, /*contig*/
    add_error_func, /*vector*/
    add_error_func, /*hvector*/
    add_error_func, /*indexed*/
    add_error_func, /*hindexed*/
    add_error_func, /*struct*/
    add_d_complex,  /*double_complex*/
    add_error_func, /*packed*/
    add_error_func, /*ub*/
    add_error_func, /*lb*/
    add_double,     /*longdouble*/
    add_error_func, /*longlongint*/
    add_error_func, /*logical*/
    add_int_mmx     /*fort_int*/
};

void MPIR_add_mmx(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_add_mmx[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}


static void add_int_mmx(int *inbuf,int*inoutbuf,unsigned int len) {
asm (
        "pushl   %%ebx                \n\t"
        "movl    16(%%esp), %%eax      \n\t"    
        "movl    %%eax, %%ecx          \n\t"      

        "andl    $31, %%eax           \n\t"      
        "jz      aligned_aim             \n\t"      

        "movl    $32, %%edx            \n\t"      
        "subl    %%eax, %%edx          \n\t"      
        "shrl    $2, %%edx             \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_aim               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      
"match_aim:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    %%edx                \n\t"      
        "jz      remain_aim              \n\t"


"al_top_aim:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "paddd   %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "decl    %%edx                \n\t"
        "jne     al_top_aim              \n\t"
"remain_aim:                     \n\t"
        "testb   $1, %%bl             \n\t"
        "jz      startit_aim             \n\t"

        "movl    (%%ecx,%%eax,), %%edx  \n\t"      
        "addl    (%%eax), %%edx        \n\t"      
        "movl    %%edx, (%%eax)        \n\t"
        "addl    $4, %%eax            \n\t"
        "jmp     startit_aim             \n\t"

"aligned_aim:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
"startit_aim:                    \n\t"

        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      unroll_end_aim          \n\t"      
".align 4                    \n\t"
"ltop_aim:                       \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "paddd   %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "paddd   %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "paddd   %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "paddd   %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -8(%%eax)      \n\t"      

        "jne     ltop_aim                \n\t"      
"unroll_end_aim:                 \n\t"
        "movl    %%ebx, %%edx          \n\t"      
        "andl    $7, %%edx            \n\t"      
        "shrl    %%edx                \n\t"      
        "jz      norest_aim              \n\t"      
"lrest_aim:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "paddd   %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "decl    %%edx                \n\t"      
        "jne     lrest_aim               \n\t"      
"norest_aim:                     \n\t"

        "emms                        \n\t"      
        "testb   $1, %%bl             \n\t"      
        "jz      end_aim                 \n\t"      
        "movl    (%%ecx,%%eax,), %%edx  \n\t"      
        "addl    (%%eax), %%edx        \n\t"      
        "movl    %%edx, (%%eax)        \n\t"      
"end_aim:                        \n\t"
        "popl    %%ebx                \n\t"
       
	: /* no output regs */

	: /* we get the parameters on our own */

	: "%eax", "%ecx", "%edx" );
}


static void add_short_mmx(short *inbuf,short* inoutbuf,unsigned int len) {
asm (
        "pushl   %%ebx                \n\t"
        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        "andl    $31, %%eax           \n\t"      
        "jz      aligned_asm             \n\t"      

        "movl    $32, %%edx            \n\t"      
        "subl    %%eax, %%edx          \n\t"      
        "shrl    $1, %%edx             \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_asm               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      
"match_asm:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $2, %%edx            \n\t"      
        "jz      remain_asm              \n\t"

"al_top_asm:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "paddw   %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "decl    %%edx                \n\t"
        "jne     al_top_asm              \n\t"
"remain_asm:                     \n\t"
        "andb    $3, %%bl             \n\t"
        "jz      startit_asm             \n\t"
"al_rest_asm:                    \n\t"
        "movw    (%%ecx,%%eax,), %%dx   \n\t"      
        "addw    (%%eax), %%dx         \n\t"      
        "movw    %%dx, (%%eax)         \n\t"
        "addl    $2, %%eax            \n\t"
        "decb    %%bl                 \n\t"
        "jnz     al_rest_asm             \n\t"
        "jmp     startit_asm             \n\t"

"aligned_asm:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_asm:                    \n\t"
        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $4, %%edx            \n\t"      
        "jz      unroll_end_asm          \n\t"      
".align 4                    \n\t"
"ltop_asm:                       \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "paddw   %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "paddw   %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "paddw   %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "paddw   %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -8(%%eax)      \n\t"      

        "jne     ltop_asm                \n\t"      

"unroll_end_asm:                 \n\t"
        "movl    %%ebx, %%edx          \n\t"      
        "andl    $15, %%edx           \n\t"      
        "shrl    $2, %%edx            \n\t"      
        "jz      norest_asm              \n\t"      

"lrest_asm:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "paddw   %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "decl    %%edx                \n\t"      
        "jne     lrest_asm               \n\t"      
"norest_asm:                     \n\t"

        "emms                        \n\t"      

        "andb    $3, %%bl             \n\t"      
        "jz      end_asm                 \n\t"      
"lshort_asm:                     \n\t"
        "movw    (%%ecx,%%eax,), %%dx   \n\t"      
        "addw    (%%eax), %%dx         \n\t"      
        "movw    %%dx, (%%eax)         \n\t"      
        "addl    $2, %%eax            \n\t"
        "decb    %%bl                 \n\t"
        "jnz     lshort_asm              \n\t"

"end_asm:                        \n\t"
        "popl    %%ebx                \n\t"
       

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void add_byte_mmx(char *inbuf,char *inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_abm             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_abm               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      
"match_abm:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      remain_abm              \n\t"

"al_top_abm:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "paddb   %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "decl    %%edx                \n\t"
        "jne     al_top_abm              \n\t"
"remain_abm:                     \n\t"
        "andb    $7, %%bl             \n\t"
        "jz      startit_abm             \n\t"

"al_rest_abm:                    \n\t"
        "movb    (%%ecx,%%eax,), %%dl   \n\t"      
        "addb    (%%eax), %%dl         \n\t"      
        "movb    %%dl, (%%eax)         \n\t"
        "incl    %%eax                \n\t"
        "decb    %%bl                 \n\t"
        "jnz     al_rest_abm             \n\t"
        "jmp     startit_abm             \n\t"

"aligned_abm:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_abm:                    \n\t"
        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $5, %%edx            \n\t"      
        "jz      unroll_end_abm          \n\t"      
".align 4                    \n\t"
"ltop_abm:                       \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "paddb   %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "paddb   %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "paddb   %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "paddb   %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -8(%%eax)      \n\t"      

        "jne     ltop_abm                \n\t"      

"unroll_end_abm:                 \n\t"
        "movl    %%ebx, %%edx          \n\t"      
        "andl    $31, %%edx           \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      norest_abm              \n\t"      

"lrest_abm:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "paddb   %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "decl    %%edx                \n\t"      
        "jne     lrest_abm               \n\t"      
"norest_abm:                     \n\t"

        "emms                        \n\t"      

        "andb    $7, %%bl             \n\t"      
        "jz      end_abm                 \n\t"      
"lbyte_abm:                      \n\t"
        "movb    (%%ecx,%%eax,), %%dl   \n\t"      
        "addb    (%%eax), %%dl         \n\t"      
        "movb    %%dl, (%%eax)         \n\t"      
        "incl    %%eax                \n\t"
        "decb    %%bl                 \n\t"
        "jnz     lbyte_abm               \n\t"

"end_abm:                        \n\t"
        "popl    %%ebx                \n\t"
        
       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


/* ---------------------MPI_PROD----------------------------------*/
void prod_error_func(void* d1,void*d2,unsigned int l) {
    MPIR_Op_errno = MPIR_ERRCLASS_TO_CODE(MPI_ERR_OP,MPIR_ERR_NOT_DEFINED);
    MPIR_ERROR(MPIR_COMM_WORLD,MPIR_Op_errno, "MPI_PROD" );
}

static void prod_short_mmx(short *inbuf,short *inoutbuf,unsigned int len);

static void prod_int(int *inbuf,int*inoutbuf,unsigned int len) {
    unsigned int i,u;
    u=len&(~3);   
    for(i=0;i<u;i+=4) {
        inoutbuf[i]*=inbuf[i];
        inoutbuf[i+1]*=inbuf[i+1];
        inoutbuf[i+2]*=inbuf[i+2];
        inoutbuf[i+3]*=inbuf[i+3];
    }
    for(;i<len;++i)
        inoutbuf[i]*=inbuf[i];
}

static void prod_byte(char *inbuf,char *inoutbuf,unsigned int len) {
    unsigned int i,u;
    u=len&(~3);   
    for(i=0;i<u;i+=4) {
        inoutbuf[i]*=inbuf[i];
        inoutbuf[i+1]*=inbuf[i+1];
        inoutbuf[i+2]*=inbuf[i+2];
        inoutbuf[i+3]*=inbuf[i+3];
    }
    for(;i<len;++i)
        inoutbuf[i]*=inbuf[i];
}

static void prod_float(float *inbuf,float* inoutbuf,unsigned int len) {
    unsigned int i,u;
    u=len&(~3);   
    for(i=0;i<u;i+=4) {
        inoutbuf[i]*=inbuf[i];
        inoutbuf[i+1]*=inbuf[i+1];
        inoutbuf[i+2]*=inbuf[i+2];
        inoutbuf[i+3]*=inbuf[i+3];
    }
    for(;i<len;++i)
        inoutbuf[i]*=inbuf[i];
}

static void prod_double(double *inbuf,double* inoutbuf,unsigned int len) {
    unsigned int i,u;

    u=len&(~3);   
    for(i=0;i<u;i+=4) {
        inoutbuf[i]*=inbuf[i];
        inoutbuf[i+1]*=inbuf[i+1];
        inoutbuf[i+2]*=inbuf[i+2];
        inoutbuf[i+3]*=inbuf[i+3];
    }
    for(;i<len;++i)
        inoutbuf[i]*=inbuf[i];
}

static void prod_s_complex(s_complex *inbuf,s_complex *inoutbuf,unsigned int len) {
    unsigned int i,u;
    float c;
    u=len&(~3);

    for(i=0;i<u;i+=4) {
        c = inoutbuf[i].re;
        inoutbuf[i].re = inoutbuf[i].re*inbuf[i].re - inoutbuf[i].im*inbuf[i].im;
        inoutbuf[i].im = c*inbuf[i].im+inoutbuf[i].im*inbuf[i].re ;
        
        c = inoutbuf[i+1].re;
        inoutbuf[i+1].re = inoutbuf[i+1].re*inbuf[i+1].re - inoutbuf[i+1].im*inbuf[i+1].im;
        inoutbuf[i+1].im = c*inbuf[i+1].im+inoutbuf[i+1].im*inbuf[i+1].re ;

        c = inoutbuf[i+2].re;
        inoutbuf[i+2].re = inoutbuf[i+2].re*inbuf[i+2].re - inoutbuf[i+2].im*inbuf[i+2].im;
        inoutbuf[i+2].im = c*inbuf[i+2].im+inoutbuf[i+2].im*inbuf[i+2].re ;

        c = inoutbuf[i+3].re;
        inoutbuf[i+3].re = inoutbuf[i+3].re*inbuf[i+3].re - inoutbuf[i+3].im*inbuf[i+3].im;
        inoutbuf[i+3].im = c*inbuf[i+3].im+inoutbuf[i+3].im*inbuf[i+3].re ;
    }
    for(;i<len;++i) {
        c = inoutbuf[i].re;
        inoutbuf[i].re = inoutbuf[i].re*inbuf[i].re - inoutbuf[i].im*inbuf[i].im;
        inoutbuf[i].im = c*inbuf[i].im+inoutbuf[i].im*inbuf[i].re ;
    }
}

static void prod_d_complex(d_complex *inbuf,d_complex *inoutbuf,unsigned int len) {
    unsigned int i,u;
    double c;
    
    u=len&(~3);
    for(i=0;i<u;i+=4) {
        c = inoutbuf[i].re;
        inoutbuf[i].re = inoutbuf[i].re*inbuf[i].re - inoutbuf[i].im*inbuf[i].im;
        inoutbuf[i].im = c*inbuf[i].im+inoutbuf[i].im*inbuf[i].re ;
        
        c = inoutbuf[i+1].re;
        inoutbuf[i+1].re = inoutbuf[i+1].re*inbuf[i+1].re - inoutbuf[i+1].im*inbuf[i+1].im;
        inoutbuf[i+1].im = c*inbuf[i+1].im+inoutbuf[i+1].im*inbuf[i+1].re ;

        c = inoutbuf[i+2].re;
        inoutbuf[i+2].re = inoutbuf[i+2].re*inbuf[i+2].re - inoutbuf[i+2].im*inbuf[i+2].im;
        inoutbuf[i+2].im = c*inbuf[i+2].im+inoutbuf[i+2].im*inbuf[i+2].re ;

        c = inoutbuf[i+3].re;
        inoutbuf[i+3].re = inoutbuf[i+3].re*inbuf[i+3].re - inoutbuf[i+3].im*inbuf[i+3].im;
        inoutbuf[i+3].im = c*inbuf[i+3].im+inoutbuf[i+3].im*inbuf[i+3].re ;
    }
    for(;i<len;++i) {
        c = inoutbuf[i].re;
        inoutbuf[i].re = inoutbuf[i].re*inbuf[i].re - inoutbuf[i].im*inbuf[i].im;
        inoutbuf[i].im = c*inbuf[i].im+inoutbuf[i].im*inbuf[i].re ;
    }
}

static const proto_func jmp_prod_mmx[] = {
    prod_int,       /*int*/
    prod_float,     /*float*/
    prod_double,    /*double*/
    prod_s_complex, /*complex*/
    prod_int,       /*long*/
    prod_short_mmx, /*short*/
    prod_byte,      /*char*/
    prod_byte,      /*byte*/
    prod_byte,      /*uchar*/
    prod_short_mmx, /*ushort*/
    prod_int,       /*ulong*/
    prod_int,       /*uint*/
    prod_error_func,/*contig*/
    prod_error_func,/*vector*/
    prod_error_func,/*hvector*/
    prod_error_func,/*indexed*/
    prod_error_func,/*hindexed*/
    prod_error_func,/*struct*/
    prod_d_complex, /*double_complex*/
    prod_error_func,/*packed*/
    prod_error_func,/*ub*/
    prod_error_func,/*lb*/
    prod_double,    /*longdouble*/
    prod_error_func,/*longlongint*/
    prod_error_func,/*logical*/
    prod_int        /*fort_int*/
};

void MPIR_prod_mmx(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_prod_mmx[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}


static void prod_short_mmx(short *inbuf,short* inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax           \n\t"      
        "jz      aligned_psm             \n\t"      

        "movl    $32, %%edx            \n\t"      
        "subl    %%eax, %%edx          \n\t"      
        "shrl    $1, %%edx             \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_psm               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      
"match_psm:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $2, %%edx            \n\t"      
        "jz      remain_psm              \n\t"

"al_top_psm:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "pmullw  %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "decl    %%edx                \n\t"
        "jne     al_top_psm              \n\t"
"remain_psm:                     \n\t"
        "andb    $3, %%bl             \n\t"
        "jz      startit_psm             \n\t"
"al_rest_psm:                    \n\t"
        "movw    (%%ecx,%%eax,), %%dx   \n\t"      
        "imulw   (%%eax), %%dx         \n\t"      
        "movw    %%dx, (%%eax)         \n\t"
        "addl    $2, %%eax            \n\t"
        "decb    %%bl                 \n\t"
        "jnz     al_rest_psm             \n\t"
        "jmp     startit_psm             \n\t"

"aligned_psm:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_psm:                    \n\t"
        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $4, %%edx            \n\t"      
        "jz      unroll_end_psm          \n\t"      
".align 4                    \n\t"
"ltop_psm:                       \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "pmullw  %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "pmullw  %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "pmullw  %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "pmullw  %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -8(%%eax)      \n\t"      

        "jne     ltop_psm                \n\t"      

"unroll_end_psm:                 \n\t"
        "movl    %%ebx, %%edx          \n\t"      
        "andl    $15, %%edx           \n\t"      
        "shrl    $2, %%edx            \n\t"      
        "jz      norest_psm              \n\t"      

"lrest_psm:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "pmullw  %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "decl    %%edx                \n\t"      
        "jne     lrest_psm               \n\t"      
"norest_psm:                     \n\t"

        "emms                        \n\t"      

        "andb    $3, %%bl             \n\t"      
        "jz      end_psm                 \n\t"      
"lshort_psm:                     \n\t"
        "movw    (%%ecx,%%eax,), %%dx   \n\t"      
        "imulw   (%%eax), %%dx         \n\t"      
        "movw    %%dx, (%%eax)         \n\t"      
        "addl    $2, %%eax            \n\t"
        "decb    %%bl                 \n\t"
        "jnz     lshort_psm              \n\t"

"end_psm:                        \n\t"
        "popl    %%ebx                \n\t"
       

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


/* ---------------------MPI_MAX----------------------------------*/
void max_error_func(void* d1,void*d2,unsigned int l) {
    MPIR_Op_errno = MPIR_ERRCLASS_TO_CODE(MPI_ERR_OP,MPIR_ERR_NOT_DEFINED);
    MPIR_ERROR(MPIR_COMM_WORLD,MPIR_Op_errno, "MPI_MAX" );
}

static void max_sint_mmx(int *inbuf,int*inoutbuf,unsigned int len);
static void max_sshort_mmx(short *inbuf,short *inoutbuf,unsigned int len);
static void max_sbyte_mmx(signed char *inbuf,signed char *inoutbuf,unsigned int len);

static void max_float_cmov(float *inbuf,float *inoutbuf,unsigned int len);
static void max_double_cmov(double *inbuf,double *inoutbuf,unsigned int len);
static void max_uint_cmov(unsigned int *inbuf,unsigned int*inoutbuf,unsigned int len);
static void max_ushort_cmov(unsigned short *inbuf,unsigned short *inoutbuf,unsigned int len);
static void max_ubyte_cmov(unsigned char *inbuf,unsigned char *inoutbuf,unsigned int len);


static const proto_func jmp_max_mmx_cmov[] = {
    max_sint_mmx,   /*int*/
    max_float_cmov, /*float*/
    max_double_cmov,/*double*/
    max_error_func, /*complex*/
    max_sint_mmx,   /*long*/
    max_sshort_mmx, /*short*/
    max_sbyte_mmx,  /*char*/
    max_ubyte_cmov, /*byte*/
    max_ubyte_cmov, /*uchar*/
    max_ushort_cmov,/*ushort*/
    max_uint_cmov,  /*ulong*/
    max_uint_cmov,  /*uint*/
    max_error_func, /*contig*/
    max_error_func, /*vector*/
    max_error_func, /*hvector*/
    max_error_func, /*indexed*/
    max_error_func, /*hindexed*/
    max_error_func, /*struct*/
    max_error_func, /*double_complex*/
    max_error_func, /*packed*/
    max_error_func, /*ub*/
    max_error_func, /*lb*/
    max_double_cmov, /*longdouble*/
    max_error_func, /*longlongint*/
    max_error_func, /*logical*/
    max_sint_mmx    /*fort_int*/
};

void MPIR_max_mmx_cmov(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_max_mmx_cmov[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}




static void max_sint_mmx(int *inbuf,int*inoutbuf,unsigned int len) {
asm (
        "pushl   %%ebx                \n\t"
        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax           \n\t"      
        "jz      aligned_msm             \n\t"      

        "movl    $32, %%edx            \n\t"      
        "subl    %%eax, %%edx          \n\t"      
        "shrl    $2, %%edx             \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_msm               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      
"match_msm:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    %%edx                \n\t"      
        "jz      remain_msm              \n\t"


"al_top_msm:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "movq    %%mm0, %%mm4          \n\t"
        "pcmpgtd %%mm1, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "decl    %%edx                \n\t"
        "jne     al_top_msm              \n\t"
"remain_msm:                     \n\t"
        "testb   $1, %%bl             \n\t"
        "jz      startit_msm             \n\t"

        "movl    (%%ecx,%%eax,), %%edx  \n\t"      
        "movl    (%%eax), %%ebx        \n\t"      
        "cmpl    %%edx, %%ebx          \n\t"
        "cmovgl  %%ebx, %%edx          \n\t"
        "movl    %%edx, (%%eax)        \n\t"
        "addl    $4, %%eax            \n\t"
        "jmp     startit_msm             \n\t"

"aligned_msm:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_msm:                    \n\t"
        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      unroll_end_msm          \n\t"      
".align 4                    \n\t"
"ltop_msm:                       \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "movq    %%mm0, %%mm4          \n\t"
        "pcmpgtd %%mm1, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "movq    %%mm2, %%mm4          \n\t"
        "pcmpgtd %%mm3, %%mm4          \n\t"
        "pand    %%mm4, %%mm2          \n\t"
        "pandn   %%mm3, %%mm4          \n\t"
        "por     %%mm4, %%mm2          \n\t"      
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "movq    %%mm0, %%mm4          \n\t"
        "pcmpgtd %%mm1, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "movq    %%mm2, %%mm4          \n\t"
        "pcmpgtd %%mm3, %%mm4          \n\t"
        "pand    %%mm4, %%mm2          \n\t"
        "pandn   %%mm3, %%mm4          \n\t"
        "por     %%mm4, %%mm2          \n\t"
        "movq    %%mm2, -8(%%eax)      \n\t"      

        "jne     ltop_msm                \n\t"      
"unroll_end_msm:                 \n\t"
        "movl    %%ebx, %%edx          \n\t"      
        "andl    $7, %%edx            \n\t"      
        "shrl    %%edx                \n\t"      
        "jz      norest_msm              \n\t"      
"lrest_msm:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "movq    %%mm0, %%mm4          \n\t"
        "pcmpgtd %%mm1, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "decl    %%edx                \n\t"      
        "jne     lrest_msm               \n\t"      
"norest_msm:                     \n\t"

        "emms                        \n\t"      
        "testb   $1, %%bl             \n\t"      
        "jz      end_msm                 \n\t"      
        "movl    (%%ecx,%%eax,), %%edx  \n\t"      
        "movl    (%%eax), %%ebx        \n\t"      
        "cmpl    %%edx, %%ebx          \n\t"
        "cmovgl  %%ebx, %%edx          \n\t"
        "movl    %%edx, (%%eax)        \n\t"
"end_msm:                        \n\t"
        "popl    %%ebx                \n\t"
      
       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void max_sshort_mmx(short *inbuf,short* inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_maxsm             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      
        "shrl    $1, %%edx                \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_maxsm               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      
"match_maxsm:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $2, %%edx            \n\t"      
        "jz      remain_maxsm              \n\t"

"al_top_maxsm:                     \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "movq    %%mm0, %%mm4          \n\t"
        "pcmpgtw %%mm1, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "decl    %%edx                \n\t"
        "jne     al_top_maxsm              \n\t"
"remain_maxsm:                     \n\t"
        "andb    $3, %%bl             \n\t"
        "jz      startit_maxsm             \n\t"
"al_rest_maxsm:                    \n\t"
        "movw    (%%ecx,%%eax,), %%dx   \n\t"      
        "cmpw    (%%eax), %%dx         \n\t"      
        "cmovlw  (%%eax), %%dx         \n\t"
        "movw    %%dx, (%%eax)         \n\t"
        "addl    $2, %%eax            \n\t"
        "decb    %%bl                 \n\t"
        "jnz     al_rest_maxsm             \n\t"
        "jmp     startit_maxsm             \n\t"

"aligned_maxsm:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_maxsm:                    \n\t"
        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $4, %%edx            \n\t"      
        "jz      unroll_end_maxsm          \n\t"      
".align 4                    \n\t"
"ltop_maxsm:                       \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "movq    %%mm0, %%mm4          \n\t"
        "pcmpgtw %%mm1, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "movq    %%mm2, %%mm4          \n\t"
        "pcmpgtw %%mm3, %%mm4          \n\t"
        "pand    %%mm4, %%mm2          \n\t"
        "pandn   %%mm3, %%mm4          \n\t"
        "por     %%mm4, %%mm2          \n\t"
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "movq    %%mm0, %%mm4          \n\t"
        "pcmpgtw %%mm1, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "movq    %%mm2, %%mm4          \n\t"
        "pcmpgtw %%mm3, %%mm4          \n\t"
        "pand    %%mm4, %%mm2          \n\t"
        "pandn   %%mm3, %%mm4          \n\t"
        "por     %%mm4, %%mm2          \n\t"
        "movq    %%mm2, -8(%%eax)      \n\t"      

        "jne     ltop_maxsm                \n\t"      

"unroll_end_maxsm:                 \n\t"
        "movl    %%ebx, %%edx          \n\t"      
        "andl    $15, %%edx           \n\t"      
        "shrl    $2, %%edx            \n\t"      
        "jz      norest_maxsm              \n\t"      

"lrest_maxsm:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "movq    %%mm0, %%mm4          \n\t"
        "pcmpgtw %%mm1, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "decl    %%edx                \n\t"      
        "jne     lrest_maxsm               \n\t"      
"norest_maxsm:                     \n\t"

        "emms                        \n\t"      

        "andb    $3, %%bl             \n\t"      
        "jz      end_maxsm                 \n\t"      
"lshort_maxsm:                     \n\t"
        "movw    (%%ecx,%%eax,), %%dx   \n\t"      
        "addl    $2, %%eax            \n\t"
        "cmpw    -2(%%eax), %%dx       \n\t"      
        "cmovlw  -2(%%eax), %%dx       \n\t"      
        "decb    %%bl                 \n\t"
        "movw    %%dx, -2(%%eax)       \n\t"      
        "jnz     lshort_maxsm              \n\t"

"end_maxsm:                        \n\t"
        "popl    %%ebx                \n\t"
       
       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void max_sbyte_mmx(signed char *inbuf, signed char *inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "pushl   %%esi                \n\t"
        "movl    20(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_msbm             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    24(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_msbm               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      
"match_msbm:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 24(%%esp)      \n\t"      

        "movl    %%edx, %%esi          \n\t"      
        "movl    16(%%esp), %%ecx      \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      remain_msbm              \n\t"

"al_top_msbm:                     \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "decl    %%edx                \n\t"
        "movq    %%mm0, %%mm4          \n\t"
        "pcmpgtb %%mm1, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"
        "movq    %%mm0, -8(%%eax)      \n\t"      


        "jne     al_top_msbm              \n\t"
"remain_msbm:                     \n\t"
        "andw    $7, %%si             \n\t"
        "jz      startit_msbm             \n\t"

"al_rest_msbm:                    \n\t"
        "movb    (%%ecx,%%eax,), %%dl   \n\t"      
        "movb    (%%eax), %%bl         \n\t"
        "incl    %%eax                \n\t"
        "cmpb    %%bl, %%dl            \n\t"      
        "cmovlw  %%bx, %%dx            \n\t"
        "decw    %%si                 \n\t"
        "movb    %%dl, -1(%%eax)       \n\t"
        "jnz     al_rest_msbm             \n\t"
        "jmp     startit_msbm             \n\t"

"aligned_msbm:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    16(%%esp), %%ecx      \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_msbm:                    \n\t"
        "movl    24(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%esi          \n\t"      
        "shrl    $5, %%edx            \n\t"      
        "jz      unroll_end_msbm          \n\t"      
".align 4                    \n\t"
"ltop_msbm:                       \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "movq    %%mm0, %%mm4          \n\t"
        "pcmpgtb %%mm1, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "movq    %%mm2, %%mm4          \n\t"
        "pcmpgtb %%mm3, %%mm4          \n\t"
        "pand    %%mm4, %%mm2          \n\t"
        "pandn   %%mm3, %%mm4          \n\t"
        "por     %%mm4, %%mm2          \n\t"
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "movq    %%mm0, %%mm4          \n\t"
        "pcmpgtb %%mm1, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      

        "movq    %%mm2, %%mm4          \n\t"
        "pcmpgtb %%mm3, %%mm4          \n\t"
        "pand    %%mm4, %%mm2          \n\t"
        "pandn   %%mm3, %%mm4          \n\t"
        "por     %%mm4, %%mm2          \n\t"
        "movq    %%mm2, -8(%%eax)      \n\t"      

        "jne     ltop_msbm                \n\t"      

"unroll_end_msbm:                 \n\t"
        "movl    %%esi, %%edx          \n\t"      
        "andl    $31, %%edx           \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      norest_msbm              \n\t"      

"lrest_msbm:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "movq    %%mm0, %%mm4          \n\t"
        "pcmpgtb %%mm1, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "decl    %%edx                \n\t"      
        "jne     lrest_msbm               \n\t"      
"norest_msbm:                     \n\t"

        "emms                        \n\t"      

        "andw    $7, %%si             \n\t"      
        "jz      end_msbm                 \n\t"      
".align 4                    \n\t"
"lbyte_msbm:                      \n\t"
        "movb    (%%ecx,%%eax,), %%dl   \n\t"      
        "movb    (%%eax), %%bl         \n\t"      
        "incl    %%eax                \n\t"
        "cmpb    %%bl, %%dl            \n\t"      
        "cmovlw  %%bx, %%dx            \n\t"
        "decw    %%si                 \n\t"
        "movb    %%dl, -1(%%eax)       \n\t"      
        "jnz     lbyte_msbm               \n\t"

"end_msbm:                        \n\t"
        "popl    %%esi                \n\t"
        "popl    %%ebx                \n\t"
       

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void max_uint_cmov(unsigned int *inbuf,unsigned int*inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "pushl   %%esi                \n\t"
        "movl    24(%%esp), %%esi      \n\t"      
        "testl   %%esi, %%esi          \n\t"
        "jz      end_muc                 \n\t"
        "movl    20(%%esp), %%eax      \n\t"      
        "movl    16(%%esp), %%ecx      \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $2, %%esi            \n\t"      
        "jz      unroll_end_muc          \n\t"      

".align 4                    \n\t"
"ltop_muc:                       \n\t"
        "movl    (%%ecx,%%eax,), %%ebx  \n\t"      
        "movl    (%%eax), %%edx        \n\t"      
        "addl    $16, %%eax           \n\t"
        "cmpl    %%edx, %%ebx          \n\t"
        "cmoval  %%ebx, %%edx          \n\t"
        "movl    %%edx, -16(%%eax)     \n\t"

        "movl    -12(%%ecx,%%eax,), %%ebx \n\t"
        "movl    -12(%%eax), %%edx     \n\t"
        "cmpl    %%edx, %%ebx          \n\t"
        "cmoval  %%ebx, %%edx          \n\t"
        "movl    %%edx, -12(%%eax)     \n\t"

        "movl    -8(%%ecx,%%eax,), %%ebx \n\t"
        "movl    -8(%%eax), %%edx      \n\t"
        "cmpl    %%edx, %%ebx          \n\t"
        "cmoval  %%ebx, %%edx          \n\t"
        "movl    %%edx, -8(%%eax)      \n\t"

        "movl    -4(%%ecx,%%eax,), %%ebx \n\t"
        "movl    -4(%%eax), %%edx      \n\t"
        "cmpl    %%edx, %%ebx          \n\t"
        "cmoval  %%ebx, %%edx          \n\t"
        "decl    %%esi                \n\t"
        "movl    %%edx, -4(%%eax)      \n\t"

        "jne     ltop_muc                \n\t"

"unroll_end_muc:                 \n\t"
        "movl    24(%%esp), %%esi      \n\t"      
        "andw    $3, %%si             \n\t"      
        "jz      end_muc                 \n\t"

"lrest_muc:                      \n\t"
        "movl    (%%ecx,%%eax,), %%edx  \n\t"      
        "movl    (%%eax), %%ebx        \n\t"      
        "addl    $4, %%eax            \n\t"
        "cmpl    %%edx, %%ebx          \n\t"
        "cmoval  %%ebx, %%edx          \n\t"
        "decw    %%si                 \n\t"
        "movl    %%edx, -4(%%eax)      \n\t"
        "jnz     lrest_muc               \n\t"
"end_muc:                        \n\t"
        "popl    %%esi                \n\t"
        "popl    %%ebx                \n\t"
      
       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void max_ushort_cmov(unsigned short *inbuf,unsigned short *inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "pushl   %%esi                \n\t"
        "movl    24(%%esp), %%esi      \n\t"      
        "testl   %%esi, %%esi          \n\t"
        "jz      end_maxuc           \n\t"
        "movl    20(%%esp), %%eax      \n\t"      
        "movl    16(%%esp), %%ecx      \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $2, %%esi            \n\t"      
        "jz      unroll_end_maxuc          \n\t"      

".align 4                    \n\t"
"ltop_maxuc:                       \n\t"
        "movw    (%%ecx,%%eax,), %%bx   \n\t"      
        "movw    (%%eax), %%dx         \n\t"      
        "addl    $8, %%eax            \n\t"
        "cmpw    %%dx, %%bx            \n\t"
        "cmovaw  %%bx, %%dx            \n\t"
        "movw    %%dx, -8(%%eax)       \n\t"

        "movw    -6(%%ecx,%%eax,), %%bx \n\t"
        "movw    -6(%%eax), %%dx       \n\t"
        "cmpw    %%dx, %%bx            \n\t"
        "cmovaw  %%bx, %%dx            \n\t"
        "movw    %%dx, -6(%%eax)       \n\t"

        "movw    -4(%%ecx,%%eax,), %%bx \n\t"
        "movw    -4(%%eax), %%dx       \n\t"
        "cmpw    %%dx, %%bx            \n\t"
        "cmovaw  %%bx, %%dx            \n\t"
        "movw    %%dx, -4(%%eax)       \n\t"

        "movw    -2(%%ecx,%%eax,), %%bx \n\t"
        "movw    -2(%%eax), %%dx       \n\t"
        "cmpw    %%dx, %%bx            \n\t"
        "cmovaw  %%bx, %%dx            \n\t"
        "decl    %%esi                \n\t"
        "movw    %%dx, -2(%%eax)       \n\t"

        "jnz     ltop_maxuc                \n\t"

"unroll_end_maxuc:                 \n\t"
        "movl    24(%%esp), %%esi      \n\t"      
        "andw    $3, %%si             \n\t"      
        "jz      end_maxuc                 \n\t"

"lrest_maxuc:                      \n\t"
        "movw    (%%ecx,%%eax,), %%dx   \n\t"      
        "movw    (%%eax), %%bx         \n\t"      
        "addl    $2, %%eax            \n\t"
        "cmpw    %%dx, %%bx            \n\t"
        "cmovaw  %%bx, %%dx            \n\t"
        "decw    %%si                 \n\t"
        "movw    %%dx, -2(%%eax)       \n\t"
        "jnz     lrest_maxuc               \n\t"
"end_maxuc:                        \n\t"
        "popl    %%esi                \n\t"
        "popl    %%ebx                \n\t"
        
       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void max_ubyte_cmov(unsigned char *inbuf,unsigned char* inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "pushl   %%esi                \n\t"
        "movl    24(%%esp), %%esi      \n\t"      
        "testl   %%esi, %%esi          \n\t"
        "jz      end_maxubc          \n\t"
        "movl    20(%%esp), %%eax      \n\t"      
        "movl    16(%%esp), %%ecx      \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $2, %%esi            \n\t"      
        "jz      unroll_end_maxubc          \n\t"      

".align 4                    \n\t"
"ltop_maxubc:                       \n\t"
        "movb    (%%ecx,%%eax,), %%bl   \n\t"      
        "movb    (%%eax), %%dl         \n\t"      
        "addl    $4, %%eax            \n\t"
        "cmpb    %%dl, %%bl            \n\t"
        "cmovaw  %%bx, %%dx            \n\t"
        "movb    %%dl, -4(%%eax)       \n\t"

        "movb    -3(%%ecx,%%eax,), %%bl \n\t"
        "movb    -3(%%eax), %%dl       \n\t"
        "cmpb    %%dl, %%bl            \n\t"
        "cmovaw  %%bx, %%dx            \n\t"
        "movb    %%dl, -3(%%eax)       \n\t"

        "movb    -2(%%ecx,%%eax,), %%bl \n\t"
        "movb    -2(%%eax), %%dl       \n\t"
        "cmpb    %%dl, %%bl            \n\t"
        "cmovaw  %%bx, %%dx            \n\t"
        "movb    %%dl, -2(%%eax)       \n\t"

        "movb    -1(%%ecx,%%eax,), %%bl \n\t"
        "movb    -1(%%eax), %%dl       \n\t"
        "cmpb    %%dl, %%bl            \n\t"
        "cmovaw  %%bx, %%dx            \n\t"
        "decl    %%esi                \n\t"
        "movb    %%dl, -1(%%eax)       \n\t"

        "jnz     ltop_maxubc                \n\t"

"unroll_end_maxubc:                 \n\t"
        "movl    24(%%esp), %%esi      \n\t"      
        "andl    $3, %%esi             \n\t"      
        "jz      end_maxubc                 \n\t"

"lrest_maxubc:                      \n\t"
        "movb    (%%ecx,%%eax,), %%dl   \n\t"      
        "movb    (%%eax), %%bl         \n\t"      
        "incl    %%eax                \n\t"
        "cmpb    %%dl, %%bl            \n\t"
        "cmovaw  %%bx, %%dx            \n\t"
        "decw    %%si                 \n\t"
        "movb    %%dl, -1(%%eax)       \n\t"
        "jnz     lrest_maxubc               \n\t"
"end_maxubc:                        \n\t"
        "popl    %%esi                \n\t"
        "popl    %%ebx                \n\t"

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void max_float_cmov(float *inbuf,float* inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "pushl   %%esi                \n\t"
        "movl    24(%%esp), %%esi      \n\t"      
        "testl   %%esi, %%esi          \n\t"
        "jz      end_mfc                 \n\t"
        "movl    20(%%esp), %%eax      \n\t"      
        "movl    16(%%esp), %%ecx      \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $2, %%esi            \n\t"      
        "jz      unroll_end_mfc          \n\t"      

".align 4                    \n\t"
"ltop_mfc:                       \n\t"
        "flds    (%%ecx,%%eax,)        \n\t"      
        "flds    (%%eax)              \n\t"      
        "addl    $16, %%eax           \n\t"
        "fucomi  %%st(1), %%st(0)      \n\t"
        "fcmovb  %%st(1), %%st(0)      \n\t"
        "fstps   -16(%%eax)           \n\t"
        "ffree   %%st(0)              \n\t"

        "flds    -12(%%ecx,%%eax,)     \n\t"
        "flds    -12(%%eax)           \n\t"
        "fucomi  %%st(1), %%st(0)      \n\t"
        "fcmovb  %%st(1), %%st(0)      \n\t"
        "fstps   -12(%%eax)           \n\t"
        "ffree   %%st(0)              \n\t"

        "flds    -8(%%ecx,%%eax,)      \n\t"
        "flds    -8(%%eax)            \n\t"
        "fucomi  %%st(1), %%st(0)      \n\t"
        "fcmovb  %%st(1), %%st(0)      \n\t"
        "fstps   -8(%%eax)            \n\t"
        "ffree   %%st(0)              \n\t"

        "flds    -4(%%ecx,%%eax,)      \n\t"
        "flds    -4(%%eax)            \n\t"
        "fucomi  %%st(1), %%st(0)      \n\t"
        "fcmovb  %%st(1), %%st(0)      \n\t"
        "decl    %%esi                \n\t"
        "fstps   -4(%%eax)            \n\t"
        "ffree   %%st(0)              \n\t"
        "jne     ltop_mfc                \n\t"

"unroll_end_mfc:                 \n\t"
        "movl    24(%%esp), %%esi      \n\t"      
        "andw    $3, %%si             \n\t"      
        "jz      end_mfc                 \n\t"

"lrest_mfc:                      \n\t"
        "flds    (%%ecx,%%eax,)        \n\t"      
        "flds    (%%eax)              \n\t"      
        "addl    $4, %%eax            \n\t"
        "fucomi  %%st(1), %%st(0)      \n\t"
        "fcmovb  %%st(1), %%st(0)      \n\t"
        "decw    %%si                 \n\t"
        "fstps   -4(%%eax)            \n\t"
        "ffree   %%st(0)              \n\t"
        "jnz     lrest_mfc               \n\t"
"end_mfc:                        \n\t"
        "popl    %%esi                \n\t"
        "popl    %%ebx                \n\t"
        

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void max_double_cmov(double *inbuf,double* inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "pushl   %%esi                \n\t"
        "movl    24(%%esp), %%esi      \n\t"      
        "testl   %%esi, %%esi          \n\t"
        "jz      end_mdc                 \n\t"
        "movl    20(%%esp), %%eax      \n\t"      
        "movl    16(%%esp), %%ecx      \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $2, %%esi            \n\t"      
        "jz      unroll_end_mdc          \n\t"      

".align 4                    \n\t"
"ltop_mdc:                       \n\t"
        "fldl   (%%ecx,%%eax,)        \n\t"      
        "fldl   (%%eax)              \n\t"      
        "addl    $32, %%eax           \n\t"
        "fucomi  %%st(1), %%st(0)      \n\t"
        "fcmovb  %%st(1), %%st(0)      \n\t"
        "fstpl  -32(%%eax)           \n\t"
        "ffree   %%st(0)              \n\t"

        "fldl   -24(%%ecx,%%eax,)     \n\t"
        "fldl   -24(%%eax)           \n\t"
        "fucomi  %%st(1), %%st(0)      \n\t"
        "fcmovb  %%st(1), %%st(0)      \n\t"
        "fstpl  -24(%%eax)           \n\t"
        "ffree   %%st(0)              \n\t"

        "fldl   -16(%%ecx,%%eax,)     \n\t"
        "fldl   -16(%%eax)           \n\t"
        "fucomi  %%st(1), %%st(0)      \n\t"
        "fcmovb  %%st(1), %%st(0)      \n\t"
        "fstpl  -16(%%eax)           \n\t"
        "ffree   %%st(0)              \n\t"

        "fldl   -8(%%ecx,%%eax,)      \n\t"
        "fldl   -8(%%eax)            \n\t"
        "fucomi  %%st(1), %%st(0)      \n\t"
        "fcmovb  %%st(1), %%st(0)      \n\t"
        "decl    %%esi                \n\t"
        "fstpl  -8(%%eax)            \n\t"
        "ffree   %%st(0)              \n\t"
        "jne     ltop_mdc                \n\t"

"unroll_end_mdc:                 \n\t"
        "movl    24(%%esp), %%esi      \n\t"      
        "andw    $3, %%si             \n\t"      
        "jz      end_mdc                 \n\t"

"lrest_mdc:                      \n\t"
        "fldl   (%%ecx,%%eax,)        \n\t"      
        "fldl   (%%eax)              \n\t"      
        "addl    $8, %%eax            \n\t"
        "fucomi  %%st(1), %%st(0)      \n\t"
        "fcmovb  %%st(1), %%st(0)      \n\t"
        "decw    %%si                 \n\t"
        "fstpl  -8(%%eax)            \n\t"
        "ffree   %%st(0)              \n\t"
        "jnz     lrest_mdc               \n\t"
"end_mdc:                        \n\t"
        "popl    %%esi                \n\t"
        "popl    %%ebx                \n\t"
        
       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


/* ---------------------MPI_MIN----------------------------------*/
void min_error_func(void* d1,void*d2,unsigned int l) {
    MPIR_Op_errno = MPIR_ERRCLASS_TO_CODE(MPI_ERR_OP,MPIR_ERR_NOT_DEFINED);
    MPIR_ERROR(MPIR_COMM_WORLD,MPIR_Op_errno, "MPI_MIN" );
}

static void min_sint_mmx(int *inbuf,int*inoutbuf,unsigned int len);
static void min_sshort_mmx(short *inbuf,short *inoutbuf,unsigned int len);
static void min_sbyte_mmx(signed char *inbuf,signed char *inoutbuf,unsigned int len);

static void min_float_cmov(float *inbuf,float *inoutbuf,unsigned int len);
static void min_double_cmov(double *inbuf,double *inoutbuf,unsigned int len);
static void min_uint_cmov(unsigned int *inbuf,unsigned int*inoutbuf,unsigned int len);
static void min_ushort_cmov(unsigned short *inbuf,unsigned short *inoutbuf,unsigned int len);
static void min_ubyte_cmov(unsigned char *inbuf,unsigned char *inoutbuf,unsigned int len);


static const proto_func jmp_min_mmx_cmov[] = {
    min_sint_mmx,   /*int*/
    min_float_cmov, /*float*/
    min_double_cmov,/*double*/
    min_error_func, /*complex*/
    min_sint_mmx,   /*long*/
    min_sshort_mmx, /*short*/
    min_sbyte_mmx,  /*char*/
    min_ubyte_cmov, /*byte*/
    min_ubyte_cmov, /*uchar*/
    min_ushort_cmov,/*ushort*/
    min_uint_cmov,  /*ulong*/
    min_uint_cmov,  /*uint*/
    min_error_func, /*contig*/
    min_error_func, /*vector*/
    min_error_func, /*hvector*/
    min_error_func, /*indexed*/
    min_error_func, /*hindexed*/
    min_error_func, /*struct*/
    min_error_func, /*double_complex*/
    min_error_func, /*packed*/
    min_error_func, /*ub*/
    min_error_func, /*lb*/
    min_double_cmov, /*longdouble*/
    min_error_func, /*longlongint*/
    min_error_func, /*logical*/
    min_sint_mmx    /*fort_int*/
};

void MPIR_min_mmx_cmov(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_min_mmx_cmov[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}




static void min_sint_mmx(int *inbuf,int*inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_minsm             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      
        "shrl    $2, %%edx                \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_minsm               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      
"match_minsm:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    %%edx                \n\t"      
        "jz      remain_minsm              \n\t"


"al_top_minsm:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "movq    %%mm1, %%mm4          \n\t"
        "pcmpgtd %%mm0, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "decl    %%edx                \n\t"
        "jne     al_top_minsm              \n\t"
"remain_minsm:                     \n\t"
        "testb   $1, %%bl             \n\t"
        "jz      startit_minsm             \n\t"

        "movl    (%%ecx,%%eax,), %%edx  \n\t"      
        "movl    (%%eax), %%ebx        \n\t"      
        "cmpl    %%ebx, %%edx          \n\t"
        "cmovgl  %%ebx, %%edx          \n\t"
        "movl    %%edx, (%%eax)        \n\t"
        "addl    $4, %%eax            \n\t"
        "jmp     startit_minsm             \n\t"

"aligned_minsm:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_minsm:                    \n\t"
        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      unroll_end_minsm          \n\t"      
".align 4                    \n\t"
"ltop_minsm:                       \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "movq    %%mm1, %%mm4          \n\t"
        "pcmpgtd %%mm0, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "movq    %%mm3, %%mm4          \n\t"
        "pcmpgtd %%mm2, %%mm4          \n\t"
        "pand    %%mm4, %%mm2          \n\t"
        "pandn   %%mm3, %%mm4          \n\t"
        "por     %%mm4, %%mm2          \n\t"      
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "movq    %%mm1, %%mm4          \n\t"
        "pcmpgtd %%mm0, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "movq    %%mm3, %%mm4          \n\t"
        "pcmpgtd %%mm2, %%mm4          \n\t"
        "pand    %%mm4, %%mm2          \n\t"
        "pandn   %%mm3, %%mm4          \n\t"
        "por     %%mm4, %%mm2          \n\t"
        "movq    %%mm2, -8(%%eax)      \n\t"      

        "jne     ltop_minsm                \n\t"      
"unroll_end_minsm:                 \n\t"
        "movl    %%ebx, %%edx          \n\t"      
        "andl    $7, %%edx            \n\t"      
        "shrl    %%edx                \n\t"      
        "jz      norest_minsm              \n\t"      
"lrest_minsm:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "movq    %%mm1, %%mm4          \n\t"
        "pcmpgtd %%mm0, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "decl    %%edx                \n\t"      
        "jne     lrest_minsm               \n\t"      
"norest_minsm:                     \n\t"

        "emms                        \n\t"      
        "testb   $1, %%bl             \n\t"      
        "jz      end_minsm                 \n\t"      
        "movl    (%%ecx,%%eax,), %%edx  \n\t"      
        "movl    (%%eax), %%ebx        \n\t"      
        "cmpl    %%ebx, %%edx          \n\t"
        "cmovgl  %%ebx, %%edx          \n\t"
        "movl    %%edx, (%%eax)        \n\t"
"end_minsm:                        \n\t"
        "popl    %%ebx                \n\t"
       
       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void min_sshort_mmx(short *inbuf,short* inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_minssm             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      
        "shrl    $1, %%edx                \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_minssm               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      
"match_minssm:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $2, %%edx            \n\t"      
        "jz      remain_minssm              \n\t"

"al_top_minssm:                     \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "movq    %%mm1, %%mm4          \n\t"
        "pcmpgtw %%mm0, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "decl    %%edx                \n\t"
        "jne     al_top_minssm              \n\t"
"remain_minssm:                     \n\t"
        "andb    $3, %%bl             \n\t"
        "jz      startit_minssm             \n\t"
"al_rest_minssm:                    \n\t"
        "movw    (%%ecx,%%eax,), %%dx   \n\t"      
        "cmpw    (%%eax), %%dx         \n\t"      
        "cmovgw  (%%eax), %%dx         \n\t"
        "movw    %%dx, (%%eax)         \n\t"
        "addl    $2, %%eax            \n\t"
        "decb    %%bl                 \n\t"
        "jnz     al_rest_minssm             \n\t"
        "jmp     startit_minssm             \n\t"

"aligned_minssm:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_minssm:                    \n\t"
        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $4, %%edx            \n\t"      
        "jz      unroll_end_minssm          \n\t"      
".align 4                    \n\t"
"ltop_minssm:                       \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "movq    %%mm1, %%mm4          \n\t"
        "pcmpgtw %%mm0, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "movq    %%mm3, %%mm4          \n\t"
        "pcmpgtw %%mm2, %%mm4          \n\t"
        "pand    %%mm4, %%mm2          \n\t"
        "pandn   %%mm3, %%mm4          \n\t"
        "por     %%mm4, %%mm2          \n\t"
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "movq    %%mm1, %%mm4          \n\t"
        "pcmpgtw %%mm0, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "movq    %%mm3, %%mm4          \n\t"
        "pcmpgtw %%mm2, %%mm4          \n\t"
        "pand    %%mm4, %%mm2          \n\t"
        "pandn   %%mm3, %%mm4          \n\t"
        "por     %%mm4, %%mm2          \n\t"
        "movq    %%mm2, -8(%%eax)      \n\t"      

        "jne     ltop_minssm                \n\t"      

"unroll_end_minssm:                 \n\t"
        "movl    %%ebx, %%edx          \n\t"      
        "andl    $15, %%edx           \n\t"      
        "shrl    $2, %%edx            \n\t"      
        "jz      norest_minssm              \n\t"      

"lrest_minssm:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "movq    %%mm1, %%mm4          \n\t"
        "pcmpgtw %%mm0, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "decl    %%edx                \n\t"      
        "jne     lrest_minssm               \n\t"      
"norest_minssm:                     \n\t"

        "emms                        \n\t"      

        "andb    $3, %%bl             \n\t"      
        "jz      end_minssm                 \n\t"      
"lshort_minssm:                     \n\t"
        "movw    (%%ecx,%%eax,), %%dx   \n\t"      
        "addl    $2, %%eax            \n\t"
        "cmpw    -2(%%eax), %%dx       \n\t"      
        "cmovgw  -2(%%eax), %%dx       \n\t"      
        "decb    %%bl                 \n\t"
        "movw    %%dx, -2(%%eax)       \n\t"      
        "jnz     lshort_minssm              \n\t"

"end_minssm:                        \n\t"
        "popl    %%ebx                \n\t"
       

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void min_sbyte_mmx(signed char *inbuf, signed char *inoutbuf, unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "pushl   %%esi                \n\t"
        "movl    20(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_mism             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    24(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_mism               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      
"match_mism:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 24(%%esp)      \n\t"      

        "movl    %%edx, %%esi          \n\t"      
        "movl    16(%%esp), %%ecx      \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      remain_mism              \n\t"

"al_top_mism:                     \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "decl    %%edx                \n\t"
        "movq    %%mm1, %%mm4          \n\t"
        "pcmpgtb %%mm0, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"
        "movq    %%mm0, -8(%%eax)      \n\t"      


        "jne     al_top_mism              \n\t"
"remain_mism:                     \n\t"
        "andw    $7, %%si             \n\t"
        "jz      startit_mism             \n\t"

"al_rest_mism:                    \n\t"
        "movb    (%%ecx,%%eax,), %%dl   \n\t"      
        "movb    (%%eax), %%bl         \n\t"
        "incl    %%eax                \n\t"
        "cmpb    %%bl, %%dl            \n\t"      
        "cmovgw  %%bx, %%dx            \n\t"
        "decw    %%si                 \n\t"
        "movb    %%dl, -1(%%eax)       \n\t"
        "jnz     al_rest_mism             \n\t"
        "jmp     startit_mism             \n\t"

"aligned_mism:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    16(%%esp), %%ecx      \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_mism:                    \n\t"
        "movl    24(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%esi          \n\t"      
        "shrl    $5, %%edx            \n\t"      
        "jz      unroll_end_mism          \n\t"      
".align 4                    \n\t"
"ltop_mism:                       \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "movq    %%mm1, %%mm4          \n\t"
        "pcmpgtb %%mm0, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "movq    %%mm3, %%mm4          \n\t"
        "pcmpgtb %%mm2, %%mm4          \n\t"
        "pand    %%mm4, %%mm2          \n\t"
        "pandn   %%mm3, %%mm4          \n\t"
        "por     %%mm4, %%mm2          \n\t"
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "movq    %%mm1, %%mm4          \n\t"
        "pcmpgtb %%mm0, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      

        "movq    %%mm3, %%mm4          \n\t"
        "pcmpgtb %%mm2, %%mm4          \n\t"
        "pand    %%mm4, %%mm2          \n\t"
        "pandn   %%mm3, %%mm4          \n\t"
        "por     %%mm4, %%mm2          \n\t"
        "movq    %%mm2, -8(%%eax)      \n\t"      

        "jne     ltop_mism                \n\t"      

"unroll_end_mism:                 \n\t"
        "movl    %%esi, %%edx          \n\t"      
        "andl    $31, %%edx           \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      norest_mism              \n\t"      

"lrest_mism:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "movq    %%mm1, %%mm4          \n\t"
        "pcmpgtb %%mm0, %%mm4          \n\t"
        "pand    %%mm4, %%mm0          \n\t"
        "pandn   %%mm1, %%mm4          \n\t"
        "por     %%mm4, %%mm0          \n\t"
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "decl    %%edx                \n\t"      
        "jne     lrest_mism               \n\t"      
"norest_mism:                     \n\t"

        "emms                        \n\t"      

        "andw    $7, %%si             \n\t"      
        "jz      end_mism                 \n\t"      
".align 4                    \n\t"
"lbyte_mism:                      \n\t"
        "movb    (%%ecx,%%eax,), %%dl   \n\t"      
        "movb    (%%eax), %%bl         \n\t"      
        "incl    %%eax                \n\t"
        "cmpb    %%bl, %%dl            \n\t"      
        "cmovgw  %%bx, %%dx            \n\t"
        "decw    %%si                 \n\t"
        "movb    %%dl, -1(%%eax)       \n\t"      
        "jnz     lbyte_mism               \n\t"

"end_mism:                        \n\t"
        "popl    %%esi                \n\t"
        "popl    %%ebx                \n\t"
       
       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void min_uint_cmov(unsigned int *inbuf,unsigned int*inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "pushl   %%esi                \n\t"
        "movl    24(%%esp), %%esi      \n\t"      
        "testl   %%esi, %%esi          \n\t"
        "jz      end_muic                 \n\t"
        "movl    20(%%esp), %%eax      \n\t"      
        "movl    16(%%esp), %%ecx      \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $2, %%esi            \n\t"      
        "jz      unroll_end_muic          \n\t"      

".align 4                    \n\t"
"ltop_muic:                       \n\t"
        "movl    (%%ecx,%%eax,), %%ebx  \n\t"      
        "movl    (%%eax), %%edx        \n\t"      
        "addl    $16, %%eax           \n\t"
        "cmpl    %%edx, %%ebx          \n\t"
        "cmovbl  %%ebx, %%edx          \n\t"
        "movl    %%edx, -16(%%eax)     \n\t"

        "movl    -12(%%ecx,%%eax,), %%ebx \n\t"
        "movl    -12(%%eax), %%edx     \n\t"
        "cmpl    %%edx, %%ebx          \n\t"
        "cmovbl  %%ebx, %%edx          \n\t"
        "movl    %%edx, -12(%%eax)     \n\t"

        "movl    -8(%%ecx,%%eax,), %%ebx \n\t"
        "movl    -8(%%eax), %%edx      \n\t"
        "cmpl    %%edx, %%ebx          \n\t"
        "cmovbl  %%ebx, %%edx          \n\t"
        "movl    %%edx, -8(%%eax)      \n\t"

        "movl    -4(%%ecx,%%eax,), %%ebx \n\t"
        "movl    -4(%%eax), %%edx      \n\t"
        "cmpl    %%edx, %%ebx          \n\t"
        "cmovbl  %%ebx, %%edx          \n\t"
        "decl    %%esi                \n\t"
        "movl    %%edx, -4(%%eax)      \n\t"

        "jne     ltop_muic                \n\t"

"unroll_end_muic:                 \n\t"
        "movl    24(%%esp), %%esi      \n\t"      
        "andw    $3, %%si             \n\t"      
        "jz      end_muic                 \n\t"

"lrest_muic:                      \n\t"
        "movl    (%%ecx,%%eax,), %%edx  \n\t"      
        "movl    (%%eax), %%ebx        \n\t"      
        "addl    $4, %%eax            \n\t"
        "cmpl    %%edx, %%ebx          \n\t"
        "cmovbl  %%ebx, %%edx          \n\t"
        "decw    %%si                 \n\t"
        "movl    %%edx, -4(%%eax)      \n\t"
        "jnz     lrest_muic               \n\t"
"end_muic:                        \n\t"
        "popl    %%esi                \n\t"
        "popl    %%ebx                \n\t"
       

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void min_ushort_cmov(unsigned short *inbuf,unsigned short *inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "pushl   %%esi                \n\t"
        "movl    24(%%esp), %%esi      \n\t"      
        "testl   %%esi, %%esi          \n\t"
        "jz      end_minc                 \n\t"
        "movl    20(%%esp), %%eax      \n\t"      
        "movl    16(%%esp), %%ecx      \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $2, %%esi            \n\t"      
        "jz      unroll_end_minc          \n\t"      

".align 4                    \n\t"
"ltop_minc:                       \n\t"
        "movw    (%%ecx,%%eax,), %%bx   \n\t"      
        "movw    (%%eax), %%dx         \n\t"      
        "addl    $8, %%eax            \n\t"
        "cmpw    %%dx, %%bx            \n\t"
        "cmovbw  %%bx, %%dx            \n\t"
        "movw    %%dx, -8(%%eax)       \n\t"

        "movw    -6(%%ecx,%%eax,), %%bx \n\t"
        "movw    -6(%%eax), %%dx       \n\t"
        "cmpw    %%dx, %%bx            \n\t"
        "cmovbw  %%bx, %%dx            \n\t"
        "movw    %%dx, -6(%%eax)       \n\t"

        "movw    -4(%%ecx,%%eax,), %%bx \n\t"
        "movw    -4(%%eax), %%dx       \n\t"
        "cmpw    %%dx, %%bx            \n\t"
        "cmovbw  %%bx, %%dx            \n\t"
        "movw    %%dx, -4(%%eax)       \n\t"

        "movw    -2(%%ecx,%%eax,), %%bx \n\t"
        "movw    -2(%%eax), %%dx       \n\t"
        "cmpw    %%dx, %%bx            \n\t"
        "cmovbw  %%bx, %%dx            \n\t"
        "decl    %%esi                \n\t"
        "movw    %%dx, -2(%%eax)       \n\t"

        "jnz     ltop_minc                \n\t"

"unroll_end_minc:                 \n\t"
        "movl    24(%%esp), %%esi      \n\t"      
        "andw    $3, %%si             \n\t"      
        "jz      end_minc                 \n\t"

"lrest_minc:                      \n\t"
        "movw    (%%ecx,%%eax,), %%dx   \n\t"      
        "movw    (%%eax), %%bx         \n\t"      
        "addl    $2, %%eax            \n\t"
        "cmpw    %%dx, %%bx            \n\t"
        "cmovbw  %%bx, %%dx            \n\t"
        "decw    %%si                 \n\t"
        "movw    %%dx, -2(%%eax)       \n\t"
        "jnz     lrest_minc               \n\t"
"end_minc:                        \n\t"
        "popl    %%esi                \n\t"
        "popl    %%ebx                \n\t"
       

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void min_ubyte_cmov(unsigned char *inbuf,unsigned char* inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "pushl   %%esi                \n\t"
        "movl    24(%%esp), %%esi      \n\t"      
        "testl   %%esi, %%esi          \n\t"
        "jz      end_minuc                 \n\t"
        "movl    20(%%esp), %%eax      \n\t"      
        "movl    16(%%esp), %%ecx      \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $2, %%esi            \n\t"      
        "jz      unroll_end_minuc          \n\t"      

".align 4                    \n\t"
"ltop_minuc:                       \n\t"
        "movb    (%%ecx,%%eax,), %%bl   \n\t"      
        "movb    (%%eax), %%dl         \n\t"      
        "addl    $4, %%eax            \n\t"
        "cmpb    %%dl, %%bl            \n\t"
        "cmovbw  %%bx, %%dx            \n\t"
        "movb    %%dl, -4(%%eax)       \n\t"

        "movb    -3(%%ecx,%%eax,), %%bl \n\t"
        "movb    -3(%%eax), %%dl       \n\t"
        "cmpb    %%dl, %%bl            \n\t"
        "cmovbw  %%bx, %%dx            \n\t"
        "movb    %%dl, -3(%%eax)       \n\t"

        "movb    -2(%%ecx,%%eax,), %%bl \n\t"
        "movb    -2(%%eax), %%dl       \n\t"
        "cmpb    %%dl, %%bl            \n\t"
        "cmovbw  %%bx, %%dx            \n\t"
        "movb    %%dl, -2(%%eax)       \n\t"

        "movb    -1(%%ecx,%%eax,), %%bl \n\t"
        "movb    -1(%%eax), %%dl       \n\t"
        "cmpb    %%dl, %%bl            \n\t"
        "cmovbw  %%bx, %%dx            \n\t"
        "decl    %%esi                \n\t"
        "movb    %%dl, -1(%%eax)       \n\t"

        "jnz     ltop_minuc                \n\t"

"unroll_end_minuc:                 \n\t"
        "movl    24(%%esp), %%esi      \n\t"      
        "andw    $3, %%si             \n\t"      
        "jz      end_minuc                 \n\t"

"lrest_minuc:                      \n\t"
        "movb    (%%ecx,%%eax,), %%dl   \n\t"      
        "movb    (%%eax), %%bl         \n\t"      
        "incl    %%eax                \n\t"
        "cmpb    %%dl, %%bl            \n\t"
        "cmovbw  %%bx, %%dx            \n\t"
        "decw    %%si                 \n\t"
        "movb    %%dl, -1(%%eax)       \n\t"
        "jnz     lrest_minuc               \n\t"
"end_minuc:                        \n\t"
        "popl    %%esi                \n\t"
        "popl    %%ebx                \n\t"
       

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void min_float_cmov(float *inbuf,float* inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "pushl   %%esi                \n\t"
        "movl    24(%%esp), %%esi      \n\t"      
        "testl   %%esi, %%esi          \n\t"
        "jz      end_mflc                 \n\t"
        "movl    20(%%esp), %%eax      \n\t"      
        "movl    16(%%esp), %%ecx      \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $2, %%esi            \n\t"      
        "jz      unroll_end_mflc          \n\t"      

".align 4                    \n\t"
"ltop_mflc:                       \n\t"
        "flds    (%%ecx,%%eax,)        \n\t"      
        "flds    (%%eax)              \n\t"      
        "addl    $16, %%eax           \n\t"
        "fucomi  %%st(1), %%st(0)      \n\t"
        "fcmovnbe        %%st(1), %%st(0) \n\t"
        "fstps   -16(%%eax)           \n\t"
        "ffree   %%st(0)              \n\t"

        "flds    -12(%%ecx,%%eax,)     \n\t"
        "flds    -12(%%eax)           \n\t"
        "fucomi  %%st(1), %%st(0)      \n\t"
        "fcmovnbe        %%st(1), %%st(0) \n\t"
        "fstps   -12(%%eax)           \n\t"
        "ffree   %%st(0)              \n\t"

        "flds    -8(%%ecx,%%eax,)      \n\t"
        "flds    -8(%%eax)            \n\t"
        "fucomi  %%st(1), %%st(0)      \n\t"
        "fcmovnbe        %%st(1), %%st(0) \n\t"
        "fstps   -8(%%eax)            \n\t"
        "ffree   %%st(0)              \n\t"

        "flds    -4(%%ecx,%%eax,)      \n\t"
        "flds    -4(%%eax)            \n\t"
        "fucomi  %%st(1), %%st(0)      \n\t"
        "fcmovnbe        %%st(1), %%st(0) \n\t"
        "decl    %%esi                \n\t"
        "fstps   -4(%%eax)            \n\t"
        "ffree   %%st(0)              \n\t"
        "jne     ltop_mflc                \n\t"

"unroll_end_mflc:                 \n\t"
        "movl    24(%%esp), %%esi      \n\t"      
        "andw    $3, %%si             \n\t"      
        "jz      end_mflc                 \n\t"

"lrest_mflc:                      \n\t"
        "flds    (%%ecx,%%eax,)        \n\t"      
        "flds    (%%eax)              \n\t"      
        "addl    $4, %%eax            \n\t"
        "fucomi  %%st(1), %%st(0)      \n\t"
        "fcmovnbe        %%st(1), %%st(0) \n\t"
        "decw    %%si                 \n\t"
        "fstps   -4(%%eax)            \n\t"
        "ffree   %%st(0)              \n\t"
        "jnz     lrest_mflc               \n\t"
"end_mflc:                        \n\t"
        "popl    %%esi                \n\t"
        "popl    %%ebx                \n\t"
        

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void min_double_cmov(double *inbuf,double* inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "pushl   %%esi                \n\t"
        "movl    24(%%esp), %%esi      \n\t"      
        "testl   %%esi, %%esi          \n\t"
        "jz      end_mdoc                 \n\t"
        "movl    20(%%esp), %%eax      \n\t"      
        "movl    16(%%esp), %%ecx      \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $2, %%esi            \n\t"      
        "jz      unroll_end_mdoc          \n\t"      

".align 4                    \n\t"
"ltop_mdoc:                       \n\t"
        "fldl   (%%ecx,%%eax,)        \n\t"      
        "fldl   (%%eax)              \n\t"      
        "addl    $32, %%eax           \n\t"
        "fucomi  %%st(1), %%st(0)      \n\t"
        "fcmovnbe        %%st(1), %%st(0) \n\t"
        "fstpl  -32(%%eax)           \n\t"
        "ffree   %%st(0)              \n\t"

        "fldl   -24(%%ecx,%%eax,)     \n\t"
        "fldl   -24(%%eax)           \n\t"
        "fucomi  %%st(1), %%st(0)      \n\t"
        "fcmovnbe        %%st(1), %%st(0) \n\t"
        "fstpl  -24(%%eax)           \n\t"
        "ffree   %%st(0)              \n\t"

        "fldl   -16(%%ecx,%%eax,)     \n\t"
        "fldl   -16(%%eax)           \n\t"
        "fucomi  %%st(1), %%st(0)      \n\t"
        "fcmovnbe        %%st(1), %%st(0) \n\t"
        "fstpl  -16(%%eax)           \n\t"
        "ffree   %%st(0)              \n\t"

        "fldl   -8(%%ecx,%%eax,)      \n\t"
        "fldl   -8(%%eax)            \n\t"
        "fucomi  %%st(1), %%st(0)      \n\t"
        "fcmovnbe        %%st(1), %%st(0) \n\t"
        "decl    %%esi                \n\t"
        "fstpl  -8(%%eax)            \n\t"
        "ffree   %%st(0)              \n\t"
        "jne     ltop_mdoc                \n\t"

"unroll_end_mdoc:                 \n\t"
        "movl    24(%%esp), %%esi      \n\t"      
        "andw    $3, %%si             \n\t"      
        "jz      end_mdoc                 \n\t"

"lrest_mdoc:                      \n\t"
        "fldl   (%%ecx,%%eax,)        \n\t"      
        "fldl   (%%eax)              \n\t"      
        "addl    $8, %%eax            \n\t"
        "fucomi  %%st(1), %%st(0)      \n\t"
        "fcmovnbe        %%st(1), %%st(0) \n\t"
        "decw    %%si                 \n\t"
        "fstpl  -8(%%eax)            \n\t"
        "ffree   %%st(0)              \n\t"
        "jnz     lrest_mdoc               \n\t"
"end_mdoc:                        \n\t"
        "popl    %%esi                \n\t"
        "popl    %%ebx                \n\t"
       

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


/* ---------------------MPI_band----------------------------------*/
void band_error_func(void* d1,void*d2,unsigned int l) {
    MPIR_Op_errno = MPIR_ERRCLASS_TO_CODE(MPI_ERR_OP,MPIR_ERR_NOT_DEFINED);
    MPIR_ERROR(MPIR_COMM_WORLD,MPIR_Op_errno, "MPI_BAND" );
}

static void band_int_mmx(int *inbuf,int*inoutbuf,unsigned int len);
static void band_short_mmx(short *inbuf,short *inoutbuf,unsigned int len);
static void band_byte_mmx(char *inbuf, char *inoutbuf, unsigned int len);


static const proto_func jmp_band_mmx[] = {
    band_int_mmx,   /*int*/
    band_error_func, /*float*/
    band_error_func,/*double*/
    band_error_func, /*complex*/
    band_int_mmx,   /*long*/
    band_short_mmx, /*short*/
    band_byte_mmx,  /*char*/
    band_byte_mmx, /*byte*/
    band_byte_mmx, /*uchar*/
    band_short_mmx,/*ushort*/
    band_int_mmx,  /*ulong*/
    band_int_mmx,  /*uint*/
    band_error_func, /*contig*/
    band_error_func, /*vector*/
    band_error_func, /*hvector*/
    band_error_func, /*indexed*/
    band_error_func, /*hindexed*/
    band_error_func, /*struct*/
    band_error_func, /*double_complex*/
    band_error_func, /*packed*/
    band_error_func, /*ub*/
    band_error_func, /*lb*/
    band_error_func, /*longdouble*/
    band_error_func, /*longlongint*/
    band_int_mmx,   /*logical*/
    band_int_mmx    /*fort_int*/
};

void MPIR_band_mmx(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_band_mmx[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}


static void band_int_mmx(int *inbuf,int*inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_bim             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      
        "shrl    $2, %%edx                \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_bim               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      
"match_bim:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    %%edx                \n\t"      
        "jz      remain_bim              \n\t"


"al_top_bim:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "pand    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "decl    %%edx                \n\t"
        "jne     al_top_bim              \n\t"
"remain_bim:                     \n\t"
        "testb   $1, %%bl             \n\t"
        "jz      startit_bim             \n\t"

        "movl    (%%ecx,%%eax,), %%edx  \n\t"      
        "andl    (%%eax), %%edx        \n\t"      
        "movl    %%edx, (%%eax)        \n\t"
        "addl    $4, %%eax            \n\t"
        "jmp     startit_bim             \n\t"

"aligned_bim:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
"startit_bim:                    \n\t"

        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      unroll_end_bim          \n\t"      
".align 4                    \n\t"
"ltop_bim:                       \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "pand    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "pand    %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "pand    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "pand    %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -8(%%eax)      \n\t"      

        "jne     ltop_bim                \n\t"      
"unroll_end_bim:                 \n\t"
        "movl    %%ebx, %%edx          \n\t"      
        "andl    $7, %%edx            \n\t"      
        "shrl    %%edx                \n\t"      
        "jz      norest_bim              \n\t"      
"lrest_bim:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "pand    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "decl    %%edx                \n\t"      
        "jne     lrest_bim               \n\t"      
"norest_bim:                     \n\t"

        "emms                        \n\t"      
        "testb   $1, %%bl             \n\t"      
        "jz      end_bim                 \n\t"      
        "movl    (%%ecx,%%eax,), %%edx  \n\t"      
        "andl    (%%eax), %%edx        \n\t"      
        "movl    %%edx, (%%eax)        \n\t"      
"end_bim:                        \n\t"
        "popl    %%ebx                \n\t"
       

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void band_short_mmx(short *inbuf,short* inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_bsmm             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      
        "shrl    $1, %%edx                \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_bsmm               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      
"match_bsmm:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $2, %%edx            \n\t"      
        "jz      remain_bsmm              \n\t"

"al_top_bsmm:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "pand    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "decl    %%edx                \n\t"
        "jne     al_top_bsmm              \n\t"
"remain_bsmm:                     \n\t"
        "andb    $3, %%bl             \n\t"
        "jz      startit_bsmm             \n\t"
"al_rest_bsmm:                    \n\t"
        "movw    (%%ecx,%%eax,), %%dx   \n\t"      
        "andw    (%%eax), %%dx         \n\t"      
        "movw    %%dx, (%%eax)         \n\t"
        "addl    $2, %%eax            \n\t"
        "decb    %%bl                 \n\t"
        "jnz     al_rest_bsmm             \n\t"
        "jmp     startit_bsmm             \n\t"

"aligned_bsmm:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_bsmm:                    \n\t"
        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $4, %%edx            \n\t"      
        "jz      unroll_end_bsmm          \n\t"      
".align 4                    \n\t"
"ltop_bsmm:                       \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "pand    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "pand    %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "pand    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "pand    %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -8(%%eax)      \n\t"      

        "jne     ltop_bsmm                \n\t"      

"unroll_end_bsmm:                 \n\t"
        "movl    %%ebx, %%edx          \n\t"      
        "andl    $15, %%edx           \n\t"      
        "shrl    $2, %%edx            \n\t"      
        "jz      norest_bsmm              \n\t"      

"lrest_bsmm:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "pand    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "decl    %%edx                \n\t"      
        "jne     lrest_bsmm               \n\t"      
"norest_bsmm:                     \n\t"

        "emms                        \n\t"      

        "andb    $3, %%bl             \n\t"      
        "jz      end_bsmm                 \n\t"      
"lshort_bsmm:                     \n\t"
        "movw    (%%ecx,%%eax,), %%dx   \n\t"      
        "andw    (%%eax), %%dx         \n\t"      
        "movw    %%dx, (%%eax)         \n\t"      
        "addl    $2, %%eax            \n\t"
        "decb    %%bl                 \n\t"
        "jnz     lshort_bsmm              \n\t"

"end_bsmm:                        \n\t"
        "popl    %%ebx                \n\t"
        

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void band_byte_mmx(char *inbuf,char *inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_bbm             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_bbm               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      
"match_bbm:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      remain_bbm              \n\t"

"al_top_bbm:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "pand    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "decl    %%edx                \n\t"
        "jne     al_top_bbm              \n\t"
"remain_bbm:                     \n\t"
        "andb    $7, %%bl             \n\t"
        "jz      startit_bbm             \n\t"

"al_rest_bbm:                    \n\t"
        "movb    (%%ecx,%%eax,), %%dl   \n\t"      
        "andb    (%%eax), %%dl         \n\t"      
        "movb    %%dl, (%%eax)         \n\t"
        "incl    %%eax                \n\t"
        "decb    %%bl                 \n\t"
        "jnz     al_rest_bbm             \n\t"
        "jmp     startit_bbm             \n\t"

"aligned_bbm:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_bbm:                    \n\t"
        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $5, %%edx            \n\t"      
        "jz      unroll_end_bbm          \n\t"      
".align 4                    \n\t"
"ltop_bbm:                       \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "pand    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "pand    %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "pand    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "pand    %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -8(%%eax)      \n\t"      

        "jne     ltop_bbm                \n\t"      

"unroll_end_bbm:                 \n\t"
        "movl    %%ebx, %%edx          \n\t"      
        "andl    $31, %%edx           \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      norest_bbm              \n\t"      

"lrest_bbm:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "pand    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "decl    %%edx                \n\t"      
        "jne     lrest_bbm               \n\t"      
"norest_bbm:                     \n\t"

        "emms                        \n\t"      

        "andb    $7, %%bl             \n\t"      
        "jz      end_bbm                 \n\t"      
"lbyte_bbm:                      \n\t"
        "movb    (%%ecx,%%eax,), %%dl   \n\t"      
        "andb    (%%eax), %%dl         \n\t"      
        "movb    %%dl, (%%eax)         \n\t"      
        "incl    %%eax                \n\t"
        "decb    %%bl                 \n\t"
        "jnz     lbyte_bbm               \n\t"

"end_bbm:                        \n\t"
        "popl    %%ebx                \n\t"
       
       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}

/* ---------------------MPI_BOR----------------------------------*/
void bor_error_func(void* d1,void*d2,unsigned int l) {
    MPIR_Op_errno = MPIR_ERRCLASS_TO_CODE(MPI_ERR_OP,MPIR_ERR_NOT_DEFINED);
    MPIR_ERROR(MPIR_COMM_WORLD,MPIR_Op_errno, "MPI_BOR" );
}

static void bor_int_mmx(int *inbuf,int*inoutbuf,unsigned int len);
static void bor_short_mmx(short *inbuf,short *inoutbuf,unsigned int len);
static void bor_byte_mmx(char *inbuf, char *inoutbuf,unsigned int len);


static const proto_func jmp_bor_mmx[] = {
    bor_int_mmx,   /*int*/
    bor_error_func, /*float*/
    bor_error_func,/*double*/
    bor_error_func, /*complex*/
    bor_int_mmx,   /*long*/
    bor_short_mmx, /*short*/
    bor_byte_mmx,  /*char*/
    bor_byte_mmx, /*byte*/
    bor_byte_mmx, /*uchar*/
    bor_short_mmx,/*ushort*/
    bor_int_mmx,  /*ulong*/
    bor_int_mmx,  /*uint*/
    bor_error_func, /*contig*/
    bor_error_func, /*vector*/
    bor_error_func, /*hvector*/
    bor_error_func, /*indexed*/
    bor_error_func, /*hindexed*/
    bor_error_func, /*struct*/
    bor_error_func, /*double_complex*/
    bor_error_func, /*packed*/
    bor_error_func, /*ub*/
    bor_error_func, /*lb*/
    bor_error_func, /*longdouble*/
    bor_error_func, /*longlongint*/
    bor_int_mmx,   /*logical*/
    bor_int_mmx    /*fort_int*/
};

void MPIR_bor_mmx(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_bor_mmx[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}


static void bor_int_mmx(int *inbuf,int*inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_bimm             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      
        "shrl    $2, %%edx                \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_bimm               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      
"match_bimm:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    %%edx                \n\t"      
        "jz      remain_bimm              \n\t"


"al_top_bimm:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "por     %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "decl    %%edx                \n\t"
        "jne     al_top_bimm              \n\t"
"remain_bimm:                     \n\t"
        "testb   $1, %%bl             \n\t"
        "jz      startit_bimm             \n\t"

        "movl    (%%ecx,%%eax,), %%edx  \n\t"      
        "orl     (%%eax), %%edx        \n\t"      
        "movl    %%edx, (%%eax)        \n\t"
        "addl    $4, %%eax            \n\t"
        "jmp     startit_bimm             \n\t"

"aligned_bimm:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
"startit_bimm:                    \n\t"

        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      unroll_end_bimm          \n\t"      
".align 4                    \n\t"
"ltop_bimm:                       \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "por     %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "por     %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "por     %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "por     %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -8(%%eax)      \n\t"      

        "jne     ltop_bimm                \n\t"      
"unroll_end_bimm:                 \n\t"
        "movl    %%ebx, %%edx          \n\t"      
        "andl    $7, %%edx            \n\t"      
        "shrl    %%edx                \n\t"      
        "jz      norest_bimm              \n\t"      
"lrest_bimm:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "por     %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "decl    %%edx                \n\t"      
        "jne     lrest_bimm               \n\t"      
"norest_bimm:                     \n\t"

        "emms                        \n\t"      
        "testb   $1, %%bl             \n\t"      
        "jz      end_bimm                 \n\t"      
        "movl    (%%ecx,%%eax,), %%edx  \n\t"      
        "orl     (%%eax), %%edx        \n\t"      
        "movl    %%edx, (%%eax)        \n\t"      
"end_bimm:                        \n\t"
        "popl    %%ebx                \n\t"
        

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void bor_short_mmx(short *inbuf,short* inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_bom             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      
        "shrl    $1, %%edx                \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_bom               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      
"match_bom:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $2, %%edx            \n\t"      
        "jz      remain_bom              \n\t"

"al_top_bom:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "por     %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "decl    %%edx                \n\t"
        "jne     al_top_bom              \n\t"
"remain_bom:                     \n\t"
        "andb    $3, %%bl             \n\t"
        "jz      startit_bom             \n\t"
"al_rest_bom:                    \n\t"
        "movw    (%%ecx,%%eax,), %%dx   \n\t"      
        "orw     (%%eax), %%dx         \n\t"      
        "movw    %%dx, (%%eax)         \n\t"
        "addl    $2, %%eax            \n\t"
        "decb    %%bl                 \n\t"
        "jnz     al_rest_bom             \n\t"
        "jmp     startit_bom             \n\t"

"aligned_bom:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_bom:                    \n\t"
        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $4, %%edx            \n\t"      
        "jz      unroll_end_bom          \n\t"      
".align 4                    \n\t"
"ltop_bom:                       \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "por     %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "por     %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "por     %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "por     %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -8(%%eax)      \n\t"      

        "jne     ltop_bom                \n\t"      

"unroll_end_bom:                 \n\t"
        "movl    %%ebx, %%edx          \n\t"      
        "andl    $15, %%edx           \n\t"      
        "shrl    $2, %%edx            \n\t"      
        "jz      norest_bom              \n\t"      

"lrest_bom:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "por     %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "decl    %%edx                \n\t"      
        "jne     lrest_bom               \n\t"      
"norest_bom:                     \n\t"

        "emms                        \n\t"      

        "andb    $3, %%bl             \n\t"      
        "jz      end_bom                 \n\t"      
"lshort_bom:                     \n\t"
        "movw    (%%ecx,%%eax,), %%dx   \n\t"      
        "orw     (%%eax), %%dx         \n\t"      
        "movw    %%dx, (%%eax)         \n\t"      
        "addl    $2, %%eax            \n\t"
        "decb    %%bl                 \n\t"
        "jnz     lshort_bom              \n\t"

"end_bom:                        \n\t"
        "popl    %%ebx                \n\t"
       
       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void bor_byte_mmx(char *inbuf,char *inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_bbmm             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_bbmm               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      
"match_bbmm:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      remain_bbmm              \n\t"

"al_top_bbmm:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "por     %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "decl    %%edx                \n\t"
        "jne     al_top_bbmm              \n\t"
"remain_bbmm:                     \n\t"
        "andb    $7, %%bl             \n\t"
        "jz      startit_bbmm             \n\t"

"al_rest_bbmm:                    \n\t"
        "movb    (%%ecx,%%eax,), %%dl   \n\t"      
        "orb     (%%eax), %%dl         \n\t"      
        "movb    %%dl, (%%eax)         \n\t"
        "incl    %%eax                \n\t"
        "decb    %%bl                 \n\t"
        "jnz     al_rest_bbmm             \n\t"
        "jmp     startit_bbmm             \n\t"

"aligned_bbmm:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_bbmm:                    \n\t"
        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $5, %%edx            \n\t"      
        "jz      unroll_end_bbmm          \n\t"      
".align 4                    \n\t"
"ltop_bbmm:                       \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "por     %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "por     %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "por     %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "por     %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -8(%%eax)      \n\t"      

        "jne     ltop_bbmm                \n\t"      

"unroll_end_bbmm:                 \n\t"
        "movl    %%ebx, %%edx          \n\t"      
        "andl    $31, %%edx           \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      norest_bbmm              \n\t"      

"lrest_bbmm:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "por     %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "decl    %%edx                \n\t"      
        "jne     lrest_bbmm               \n\t"      
"norest_bbmm:                     \n\t"

        "emms                        \n\t"      

        "andb    $7, %%bl             \n\t"      
        "jz      end_bbmm                 \n\t"      
"lbyte_bbmm:                      \n\t"
        "movb    (%%ecx,%%eax,), %%dl   \n\t"      
        "orb     (%%eax), %%dl         \n\t"      
        "movb    %%dl, (%%eax)         \n\t"      
        "incl    %%eax                \n\t"
        "decb    %%bl                 \n\t"
        "jnz     lbyte_bbmm               \n\t"

"end_bbmm:                        \n\t"
        "popl    %%ebx                \n\t"
        
       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}

/* ---------------------MPI_BXOR----------------------------------*/
void bxor_error_func(void* d1,void*d2,unsigned int l) {
    MPIR_Op_errno = MPIR_ERRCLASS_TO_CODE(MPI_ERR_OP,MPIR_ERR_NOT_DEFINED);
    MPIR_ERROR(MPIR_COMM_WORLD,MPIR_Op_errno, "MPI_BXOR" );
}

static void bxor_int_mmx(int *inbuf,int*inoutbuf,unsigned int len);
static void bxor_short_mmx(short *inbuf,short *inoutbuf,unsigned int len);
static void bxor_byte_mmx(char *inbuf, char *inoutbuf, unsigned int len);


static const proto_func jmp_bxor_mmx[] = {
    bxor_int_mmx,   /*int*/
    bxor_error_func, /*float*/
    bxor_error_func,/*double*/
    bxor_error_func, /*complex*/
    bxor_int_mmx,   /*long*/
    bxor_short_mmx, /*short*/
    bxor_byte_mmx,  /*char*/
    bxor_byte_mmx, /*byte*/
    bxor_byte_mmx, /*uchar*/
    bxor_short_mmx,/*ushort*/
    bxor_int_mmx,  /*ulong*/
    bxor_int_mmx,  /*uint*/
    bxor_error_func, /*contig*/
    bxor_error_func, /*vector*/
    bxor_error_func, /*hvector*/
    bxor_error_func, /*indexed*/
    bxor_error_func, /*hindexed*/
    bxor_error_func, /*struct*/
    bxor_error_func, /*double_complex*/
    bxor_error_func, /*packed*/
    bxor_error_func, /*ub*/
    bxor_error_func, /*lb*/
    bxor_error_func, /*longdouble*/
    bxor_error_func, /*longlongint*/
    bxor_int_mmx,   /*logical*/
    bxor_int_mmx    /*fort_int*/
};

void MPIR_bxor_mmx(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_bxor_mmx[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}


static void bxor_int_mmx(int *inbuf,int*inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_bxim             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      
        "shrl    $2, %%edx                \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_bxim               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      
"match_bxim:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    %%edx                \n\t"      
        "jz      remain_bxim              \n\t"


"al_top_bxim:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "pxor    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "decl    %%edx                \n\t"
        "jne     al_top_bxim              \n\t"
"remain_bxim:                     \n\t"
        "testb   $1, %%bl             \n\t"
        "jz      startit_bxim             \n\t"

        "movl    (%%ecx,%%eax,), %%edx  \n\t"      
        "xorl    (%%eax), %%edx        \n\t"      
        "movl    %%edx, (%%eax)        \n\t"
        "addl    $4, %%eax            \n\t"
        "jmp     startit_bxim             \n\t"

"aligned_bxim:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
"startit_bxim:                    \n\t"

        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      unroll_end_bxim          \n\t"      
".align 4                    \n\t"
"ltop_bxim:                       \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "pxor    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "pxor    %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "pxor    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "pxor    %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -8(%%eax)      \n\t"      

        "jne     ltop_bxim                \n\t"      
"unroll_end_bxim:                 \n\t"
        "movl    %%ebx, %%edx          \n\t"      
        "andl    $7, %%edx            \n\t"      
        "shrl    %%edx                \n\t"      
        "jz      norest_bxim              \n\t"      
"lrest_bxim:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "pxor    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "decl    %%edx                \n\t"      
        "jne     lrest_bxim               \n\t"      
"norest_bxim:                     \n\t"

        "emms                        \n\t"      
        "testb   $1, %%bl             \n\t"      
        "jz      end_bxim                 \n\t"      
        "movl    (%%ecx,%%eax,), %%edx  \n\t"      
        "xorl    (%%eax), %%edx        \n\t"      
        "movl    %%edx, (%%eax)        \n\t"      
"end_bxim:                        \n\t"
        "popl    %%ebx                \n\t"
        

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void bxor_short_mmx(short *inbuf,short* inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_bxsm             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      
        "shrl    $1, %%edx                \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_bxsm               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      
"match_bxsm:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $2, %%edx            \n\t"      
        "jz      remain_bxsm              \n\t"

"al_top_bxsm:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "pxor    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "decl    %%edx                \n\t"
        "jne     al_top_bxsm              \n\t"
"remain_bxsm:                     \n\t"
        "andb    $3, %%bl             \n\t"
        "jz      startit_bxsm             \n\t"
"al_rest_bxsm:                    \n\t"
        "movw    (%%ecx,%%eax,), %%dx   \n\t"      
        "xorw    (%%eax), %%dx         \n\t"      
        "movw    %%dx, (%%eax)         \n\t"
        "addl    $2, %%eax            \n\t"
        "decb    %%bl                 \n\t"
        "jnz     al_rest_bxsm             \n\t"
        "jmp     startit_bxsm             \n\t"

"aligned_bxsm:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_bxsm:                    \n\t"
        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $4, %%edx            \n\t"      
        "jz      unroll_end_bxsm          \n\t"      
".align 4                    \n\t"
"ltop_bxsm:                       \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "pxor    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "pxor    %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "pxor    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "pxor    %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -8(%%eax)      \n\t"      

        "jne     ltop_bxsm                \n\t"      

"unroll_end_bxsm:                 \n\t"
        "movl    %%ebx, %%edx          \n\t"      
        "andl    $15, %%edx           \n\t"      
        "shrl    $2, %%edx            \n\t"      
        "jz      norest_bxsm              \n\t"      

"lrest_bxsm:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "pxor    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "decl    %%edx                \n\t"      
        "jne     lrest_bxsm               \n\t"      
"norest_bxsm:                     \n\t"

        "emms                        \n\t"      

        "andb    $3, %%bl             \n\t"      
        "jz      end_bxsm                 \n\t"      
"lshort_bxsm:                     \n\t"
        "movw    (%%ecx,%%eax,), %%dx   \n\t"      
        "xorw    (%%eax), %%dx         \n\t"      
        "movw    %%dx, (%%eax)         \n\t"      
        "addl    $2, %%eax            \n\t"
        "decb    %%bl                 \n\t"
        "jnz     lshort_bxsm              \n\t"

"end_bxsm:                        \n\t"
        "popl    %%ebx                \n\t"
      
       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void bxor_byte_mmx(char *inbuf,char *inoutbuf,unsigned int len) {
asm (


        "pushl   %%ebx                \n\t"
        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_bxbm             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_bxbm               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      
"match_bxbm:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      remain_bxbm              \n\t"

"al_top_bxbm:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "pxor    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "decl    %%edx                \n\t"
        "jne     al_top_bxbm              \n\t"
"remain_bxbm:                     \n\t"
        "andb    $7, %%bl             \n\t"
        "jz      startit_bxbm             \n\t"

"al_rest_bxbm:                    \n\t"
        "movb    (%%ecx,%%eax,), %%dl   \n\t"      
        "xorb    (%%eax), %%dl         \n\t"      
        "movb    %%dl, (%%eax)         \n\t"
        "incl    %%eax                \n\t"
        "decb    %%bl                 \n\t"
        "jnz     al_rest_bxbm             \n\t"
        "jmp     startit_bxbm             \n\t"

"aligned_bxbm:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_bxbm:                    \n\t"
        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $5, %%edx            \n\t"      
        "jz      unroll_end_bxbm          \n\t"      
".align 4                    \n\t"
"ltop_bxbm:                       \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "pxor    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "pxor    %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "pxor    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "pxor    %%mm3, %%mm2          \n\t"      
        "movq    %%mm2, -8(%%eax)      \n\t"      

        "jne     ltop_bxbm                \n\t"      

"unroll_end_bxbm:                 \n\t"
        "movl    %%ebx, %%edx          \n\t"      
        "andl    $31, %%edx           \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      norest_bxbm              \n\t"      

"lrest_bxbm:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "pxor    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "decl    %%edx                \n\t"      
        "jne     lrest_bxbm               \n\t"      
"norest_bxbm:                     \n\t"

        "emms                        \n\t"      

        "andb    $7, %%bl             \n\t"      
        "jz      end_bxbm                 \n\t"      
"lbyte_bxbm:                      \n\t"
        "movb    (%%ecx,%%eax,), %%dl   \n\t"      
        "xorb    (%%eax), %%dl         \n\t"      
        "movb    %%dl, (%%eax)         \n\t"      
        "incl    %%eax                \n\t"
        "decb    %%bl                 \n\t"
        "jnz     lbyte_bxbm               \n\t"

"end_bxbm:                        \n\t"
        "popl    %%ebx                \n\t"
        

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}

static const int oneInt[2] = {1,1};

static const short oneShort[4] = {1,1,1,1};

static const char oneByte[8] = {1,1,1,1,1,1,1,1};

/* ---------------------MPI_LXOR----------------------------------*/
void lxor_error_func(void* d1,void*d2,unsigned int l) {
    MPIR_Op_errno = MPIR_ERRCLASS_TO_CODE(MPI_ERR_OP,MPIR_ERR_NOT_DEFINED);
    MPIR_ERROR(MPIR_COMM_WORLD,MPIR_Op_errno, "MPI_lxor" );
}

static void lxor_int_mmx(int *inbuf,int*inoutbuf,unsigned int len);
static void lxor_short_mmx(short *inbuf,short *inoutbuf,unsigned int len);
static void lxor_byte_mmx(char *inbuf, char *inoutbuf,unsigned int len);


static const proto_func jmp_lxor_mmx[] = {
    lxor_int_mmx,   /*int*/
    lxor_error_func, /*float*/
    lxor_error_func,/*double*/
    lxor_error_func, /*complex*/
    lxor_int_mmx,   /*long*/
    lxor_short_mmx, /*short*/
    lxor_byte_mmx,  /*char*/
    lxor_byte_mmx, /*byte*/
    lxor_byte_mmx, /*uchar*/
    lxor_short_mmx,/*ushort*/
    lxor_int_mmx,  /*ulong*/
    lxor_int_mmx,  /*uint*/
    lxor_error_func, /*contig*/
    lxor_error_func, /*vector*/
    lxor_error_func, /*hvector*/
    lxor_error_func, /*indexed*/
    lxor_error_func, /*hindexed*/
    lxor_error_func, /*struct*/
    lxor_error_func, /*double_complex*/
    lxor_error_func, /*packed*/
    lxor_error_func, /*ub*/
    lxor_error_func, /*lb*/
    lxor_error_func, /*longdouble*/
    lxor_error_func, /*longlongint*/
    lxor_int_mmx,   /*logical*/
    lxor_int_mmx    /*fort_int*/
};

void MPIR_lxor_mmx(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_lxor_mmx[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}


static void lxor_int_mmx(int *inbuf,int*inoutbuf,unsigned int len) {
asm (
        "pushl   %%ebx                \n\t"

        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_lim             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      
        "shrl    $2, %%edx                \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_lim               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      

"match_lim:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    %%edx                \n\t"      
        "jz      remain_lim              \n\t"
        "movq    oneInt, %%mm6        \n\t"      
        "pxor    %%mm4, %%mm4          \n\t"      

"al_top_lim:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "pcmpeqd %%mm4, %%mm0          \n\t"      
        "pcmpeqd %%mm4, %%mm1          \n\t"      
        "pxor    %%mm1, %%mm0          \n\t"      
        "pand    %%mm6, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "decl    %%edx                \n\t"
        "jne     al_top_lim              \n\t"

"remain_lim:                     \n\t"
        "testb   $1, %%bl             \n\t"
        "jz      startit_lim             \n\t"

        "movl    (%%ecx,%%eax,), %%edx  \n\t"      
        "testl   %%edx, %%edx          \n\t"      
        "je      aiszero_lim             \n\t"
        "cmpl    $0, (%%eax)          \n\t"
        "jz      restrue_lim             \n\t"
        "xorl    %%edx, %%edx          \n\t"
        "jmp     result_lim              \n\t"      
"aiszero_lim:                    \n\t"
        "cmpl    $0, (%%eax)          \n\t"
        "je      nomove_lim              \n\t"      
"restrue_lim:                    \n\t"
        "movl    $1, %%edx            \n\t"
"result_lim:                     \n\t"
        "movl    %%edx, (%%eax)        \n\t"
"nomove_lim:                     \n\t"
        "addl    $4, %%eax            \n\t"
        "jmp     startit_lim             \n\t"

"aligned_lim:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_lim:                    \n\t"
        "movq    oneInt, %%mm6        \n\t"      
        "pxor    %%mm4, %%mm4          \n\t"      
        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      unroll_end_lim          \n\t"      
".align 4                    \n\t"
"ltop_lim:                       \n\t"

        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "pcmpeqd %%mm4, %%mm0          \n\t"
        "pcmpeqd %%mm4, %%mm1          \n\t"
        "pxor    %%mm1, %%mm0          \n\t"
        "pand    %%mm6, %%mm0          \n\t" 
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "pcmpeqd %%mm4, %%mm2          \n\t"
        "pcmpeqd %%mm4, %%mm3          \n\t"
        "pxor    %%mm3, %%mm2          \n\t"
        "pand    %%mm6, %%mm2          \n\t"
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "pcmpeqd %%mm4, %%mm0          \n\t"
        "pcmpeqd %%mm4, %%mm1          \n\t"
        "pxor    %%mm1, %%mm0          \n\t"
        "pand    %%mm6, %%mm0          \n\t"
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "pcmpeqd %%mm4, %%mm2          \n\t"
        "pcmpeqd %%mm4, %%mm3          \n\t"
        "pxor    %%mm3, %%mm2          \n\t"
        "pand    %%mm6, %%mm2          \n\t"
        "movq    %%mm2, -8(%%eax)      \n\t"      
        "jne     ltop_lim                \n\t"      

"unroll_end_lim:                 \n\t"
        "movb    %%bl, %%dl            \n\t"      
        "andb    $7, %%dl             \n\t"      
        "shrb    %%dl                 \n\t"      
        "jz      norest_lim              \n\t"      
".align 4                    \n\t"
"lrest_lim:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "pcmpeqd %%mm4, %%mm0          \n\t"
        "pcmpeqd %%mm4, %%mm1          \n\t"
        "pxor    %%mm1, %%mm0          \n\t"
        "decb    %%dl                 \n\t"
        "pand    %%mm6, %%mm0          \n\t"
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "jne     lrest_lim               \n\t"      
"norest_lim:                     \n\t"

        "emms                        \n\t"      
        "testb   $1, %%bl             \n\t"      
        "jz      end_lim                 \n\t"      

        "movl    (%%ecx,%%eax,), %%edx  \n\t"      
        "testl   %%edx, %%edx          \n\t"
        "je      zero_lim                \n\t"
        "cmpl    $0, (%%eax)          \n\t"
        "jz      restrue1_lim            \n\t"
        "xorl    %%edx, %%edx          \n\t"      
        "jmp     result1_lim             \n\t"      
"zero_lim:                       \n\t"
        "cmpl    $0, (%%eax)          \n\t"
        "je      end_lim                 \n\t"      
"restrue1_lim:                   \n\t"
        "movl    $1, %%edx            \n\t"
"result1_lim:                    \n\t"
        "movl    %%edx, (%%eax)        \n\t"
"end_lim:                        \n\t"
        "popl    %%ebx                \n\t"
        

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void lxor_short_mmx(short *inbuf,short *inoutbuf,unsigned int len) {
asm (
        "pushl   %%ebx                \n\t"

        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_lsm             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      
        "shrl    $1, %%edx                \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_lsm               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      

"match_lsm:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $2, %%edx            \n\t"      
        "jz      remain_lsm              \n\t"
        "movq    oneShort, %%mm6      \n\t"      
        "pxor    %%mm4, %%mm4          \n\t"      

"al_top_lsm:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "pcmpeqw %%mm4, %%mm0          \n\t"      
        "pcmpeqw %%mm4, %%mm1          \n\t"      
        "pxor    %%mm1, %%mm0          \n\t"      
        "pand    %%mm6, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "decl    %%edx                \n\t"
        "jne     al_top_lsm              \n\t"

"remain_lsm:                     \n\t"
        "andb    $3, %%bl             \n\t"
        "jz      startit_lsm             \n\t"

"al_rest_lsm:                    \n\t"
        "movw    (%%ecx,%%eax,), %%dx   \n\t"      
        "testw   %%dx, %%dx            \n\t"      
        "je      aiszero_lsm             \n\t"
        "cmpw    $0, (%%eax)          \n\t"
        "jz      restrue_lsm             \n\t"
        "xorw    %%dx, %%dx            \n\t"
        "jmp     result_lsm              \n\t"      
"aiszero_lsm:                    \n\t"
        "cmpw    $0, (%%eax)          \n\t"
        "je      nomove_lsm              \n\t"      
"restrue_lsm:                    \n\t"
        "movw    $1, %%dx             \n\t"
"result_lsm:                     \n\t"
        "movw    %%dx, (%%eax)         \n\t"
"nomove_lsm:                     \n\t"
        "addl    $2, %%eax            \n\t"
        "decb    %%bl                 \n\t"
        "jnz     al_rest_lsm             \n\t"
        "jmp     startit_lsm             \n\t"

"aligned_lsm:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_lsm:                    \n\t"
        "movq    oneShort, %%mm6      \n\t"      
        "pxor    %%mm4, %%mm4          \n\t"      
        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $4, %%edx            \n\t"      
        "jz      unroll_end_lsm          \n\t"      
".align 4                    \n\t"
"ltop_lsm:                       \n\t"

        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "pcmpeqw %%mm4, %%mm0          \n\t"
        "pcmpeqw %%mm4, %%mm1          \n\t"
        "pxor    %%mm1, %%mm0          \n\t"
        "pand    %%mm6, %%mm0          \n\t" 
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "pcmpeqw %%mm4, %%mm2          \n\t"
        "pcmpeqw %%mm4, %%mm3          \n\t"
        "pxor    %%mm3, %%mm2          \n\t"
        "pand    %%mm6, %%mm2          \n\t"
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "pcmpeqw %%mm4, %%mm0          \n\t"
        "pcmpeqw %%mm4, %%mm1          \n\t"
        "pxor    %%mm1, %%mm0          \n\t"
        "pand    %%mm6, %%mm0          \n\t"
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "pcmpeqw %%mm4, %%mm2          \n\t"
        "pcmpeqw %%mm4, %%mm3          \n\t"
        "pxor    %%mm3, %%mm2          \n\t"
        "pand    %%mm6, %%mm2          \n\t"
        "movq    %%mm2, -8(%%eax)      \n\t"      
        "jne     ltop_lsm                \n\t"      

"unroll_end_lsm:                 \n\t"
        "movb    %%bl, %%dl            \n\t"      
        "andb    $15, %%dl            \n\t"      
        "shrb    $2, %%dl             \n\t"      
        "jz      norest_lsm              \n\t"      
"lrest_lsm:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "pcmpeqw %%mm4, %%mm0          \n\t"
        "pcmpeqw %%mm4, %%mm1          \n\t"
        "pxor    %%mm1, %%mm0          \n\t"
        "decb    %%dl                 \n\t"
        "pand    %%mm6, %%mm0          \n\t"
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "jne     lrest_lsm               \n\t"      
"norest_lsm:                     \n\t"

        "emms                        \n\t"      
        "andb    $3, %%bl             \n\t"      
        "jz      end_lsm                 \n\t"      
".align 4                    \n\t"
"lrest1_lsm:                     \n\t"
        "movw    (%%ecx,%%eax,), %%dx   \n\t"      
        "testw   %%dx, %%dx            \n\t"
        "je      zero_lsm                \n\t"
        "cmpw    $0, (%%eax)          \n\t"
        "jz      restrue1_lsm            \n\t"
        "xorw    %%dx, %%dx            \n\t"      
        "jmp     result1_lsm             \n\t"      
"zero_lsm:                       \n\t"
        "cmpw    $0, (%%eax)          \n\t"
        "je      nomove1_lsm             \n\t"      
"restrue1_lsm:                   \n\t"
        "movw    $1, %%dx             \n\t"
"result1_lsm:                    \n\t"
        "movw    %%dx, (%%eax)         \n\t"
"nomove1_lsm:                    \n\t"
        "addl    $2, %%eax            \n\t"
        "decb    %%bl                 \n\t"
        "jnz     lrest1_lsm              \n\t"

"end_lsm:                        \n\t"
        "popl    %%ebx                \n\t"
        

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


void lxor_byte_mmx(char *inbuf,char *inoutbuf,unsigned int len) {
asm (
        "pushl   %%ebx                \n\t"

        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_lbm             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      
                                                

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_lbm               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      

"match_lbm:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      remain_lbm              \n\t"
        "movq    oneByte, %%mm6       \n\t"      
        "pxor    %%mm4, %%mm4          \n\t"      
".align 4                    \n\t"
"al_top_lbm:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "pcmpeqb %%mm4, %%mm0          \n\t"      
        "pcmpeqb %%mm4, %%mm1          \n\t"      
        "pxor    %%mm1, %%mm0          \n\t"      
        "pand    %%mm6, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "decl    %%edx                \n\t"
        "jne     al_top_lbm              \n\t"

"remain_lbm:                     \n\t"
        "andb    $7, %%bl             \n\t"
        "jz      startit_lbm             \n\t"

"al_rest_lbm:                    \n\t"
        "movb    (%%ecx,%%eax,), %%dl   \n\t"      
        "testb   %%dl, %%dl            \n\t"      
        "je      aiszero_lbm             \n\t"
        "cmpb    $0, (%%eax)          \n\t"
        "jz      restrue_lbm             \n\t"
        "xorb    %%dl, %%dl            \n\t"
        "jmp     result_lbm              \n\t"      
"aiszero_lbm:                    \n\t"
        "cmpb    $0, (%%eax)          \n\t"
        "je      nomove_lbm              \n\t"      
"restrue_lbm:                    \n\t"
        "movb    $1, %%dl             \n\t"
"result_lbm:                     \n\t"
        "movb    %%dl, (%%eax)         \n\t"
"nomove_lbm:                     \n\t"
        "incl    %%eax                \n\t"
        "decb    %%bl                 \n\t"
        "jnz     al_rest_lbm             \n\t"
        "jmp     startit_lbm             \n\t"

"aligned_lbm:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_lbm:                    \n\t"
        "movq    oneByte, %%mm6       \n\t"      
        "pxor    %%mm4, %%mm4          \n\t"      
        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $5, %%edx            \n\t"      
        "jz      unroll_end_lbm          \n\t"      
".align 4                    \n\t"
"ltop_lbm:                       \n\t"

        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "pcmpeqb %%mm4, %%mm0          \n\t"
        "pcmpeqb %%mm4, %%mm1          \n\t"
        "pxor    %%mm1, %%mm0          \n\t"
        "pand    %%mm6, %%mm0          \n\t" 
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "pcmpeqb %%mm4, %%mm2          \n\t"
        "pcmpeqb %%mm4, %%mm3          \n\t"
        "pxor    %%mm3, %%mm2          \n\t"
        "pand    %%mm6, %%mm2          \n\t"
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "pcmpeqb %%mm4, %%mm0          \n\t"
        "pcmpeqb %%mm4, %%mm1          \n\t"
        "pxor    %%mm1, %%mm0          \n\t"
        "pand    %%mm6, %%mm0          \n\t"
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "pcmpeqb %%mm4, %%mm2          \n\t"
        "pcmpeqb %%mm4, %%mm3          \n\t"
        "pxor    %%mm3, %%mm2          \n\t"
        "pand    %%mm6, %%mm2          \n\t"
        "movq    %%mm2, -8(%%eax)      \n\t"      
        "jne     ltop_lbm                \n\t"      

"unroll_end_lbm:                 \n\t"
        "movl    %%ebx, %%edx          \n\t"      
        "andl    $31, %%edx           \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      norest_lbm              \n\t"      
"lrest_lbm:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "pcmpeqb %%mm4, %%mm0          \n\t"
        "pcmpeqb %%mm4, %%mm1          \n\t"
        "pxor    %%mm1, %%mm0          \n\t"
        "pand    %%mm6, %%mm0          \n\t"
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "decl    %%edx                \n\t"      
        "jne     lrest_lbm               \n\t"      
"norest_lbm:                     \n\t"

        "emms                        \n\t"      
        "andb    $7, %%bl             \n\t"      
        "jz      end_lbm                 \n\t"      
".align 4                    \n\t"
"lrest1_lbm:                     \n\t"
        "movb    (%%ecx,%%eax,), %%dl   \n\t"      
        "testb   %%dl, %%dl            \n\t"
        "je      zero_lbm                \n\t"
        "cmpb    $0, (%%eax)          \n\t"
        "jz      restrue1_lbm            \n\t"
        "xorb    %%dl, %%dl            \n\t"      
        "jmp     result1_lbm             \n\t"      
"zero_lbm:                       \n\t"
        "cmpb    $0, (%%eax)          \n\t"
        "je      nomove1_lbm             \n\t"      
"restrue1_lbm:                   \n\t"
        "movb    $1, %%dl             \n\t"
"result1_lbm:                    \n\t"
        "movb    %%dl, (%%eax)         \n\t"
"nomove1_lbm:                    \n\t"
        "incl    %%eax                \n\t"
        "decb    %%bl                 \n\t"
        "jnz     lrest1_lbm              \n\t"

"end_lbm:                        \n\t"
        "popl    %%ebx                \n\t"
       

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}

/* ---------------------MPI_LAND----------------------------------*/
void land_error_func(void* d1,void*d2,unsigned int l) {
    MPIR_Op_errno = MPIR_ERRCLASS_TO_CODE(MPI_ERR_OP,MPIR_ERR_NOT_DEFINED);
    MPIR_ERROR(MPIR_COMM_WORLD,MPIR_Op_errno, "MPI_LAND" );
}

static void land_int_mmx(int *inbuf,int*inoutbuf,unsigned int len);
static void land_short_mmx(short *inbuf,short *inoutbuf,unsigned int len);
static void land_byte_mmx(char *inbuf, char *inoutbuf,unsigned int len);


static const  proto_func jmp_land_mmx[] = {
    land_int_mmx,   /*int*/
    land_error_func, /*float*/
    land_error_func,/*double*/
    land_error_func, /*complex*/
    land_int_mmx,   /*long*/
    land_short_mmx, /*short*/
    land_byte_mmx,  /*char*/
    land_byte_mmx, /*byte*/
    land_byte_mmx, /*uchar*/
    land_short_mmx,/*ushort*/
    land_int_mmx,  /*ulong*/
    land_int_mmx,  /*uint*/
    land_error_func, /*contig*/
    land_error_func, /*vector*/
    land_error_func, /*hvector*/
    land_error_func, /*indexed*/
    land_error_func, /*hindexed*/
    land_error_func, /*struct*/
    land_error_func, /*double_complex*/
    land_error_func, /*packed*/
    land_error_func, /*ub*/
    land_error_func, /*lb*/
    land_error_func, /*longdouble*/
    land_error_func, /*longlongint*/
    land_int_mmx,   /*logical*/
    land_int_mmx    /*fort_int*/
};

void MPIR_land_mmx(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_land_mmx[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}


void land_int_mmx(int *inbuf,int*inoutbuf,unsigned int len) {
asm (
        "pushl   %%ebx                \n\t"

       
        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_limm             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      
        "shrl    $2, %%edx                \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t" 
             
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_limm               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      

	"match_limm:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    %%edx                \n\t"      
        "jz      remain_limm              \n\t"
        "movq    (oneInt), %%mm6        \n\t"      
        "pxor    %%mm4, %%mm4          \n\t"      

	"al_top_limm:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "pcmpeqd %%mm4, %%mm0          \n\t"      
        "pcmpeqd %%mm4, %%mm1          \n\t" 
        "pandn   %%mm6, %%mm1          \n\t"     
        "pand    %%mm1, %%mm0          \n\t"      
        "decl    %%edx                \n\t"
          
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "jne     al_top_limm              \n\t"

	"remain_limm:                     \n\t"
        "testb   $1, %%bl             \n\t"
        "jz      startit_limm             \n\t"

        "movl    (%%ecx,%%eax,), %%edx  \n\t"      
        "testl   %%edx, %%edx          \n\t"      
        "je      result_limm              \n\t"      
        "cmpl    $0, (%%eax)          \n\t"
        "je      nomove_limm              \n\t"      
        "movl    $1, %%edx            \n\t"
	"result_limm:                     \n\t"
        "movl    %%edx, (%%eax)        \n\t"
	"nomove_limm:                     \n\t"
        "addl    $4, %%eax            \n\t"
        "jmp     startit_limm             \n\t"

"aligned_limm:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_limm:                    \n\t"
        "movq    (oneInt), %%mm6        \n\t"      
        "pxor    %%mm4, %%mm4          \n\t"      
        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      unroll_end_limm          \n\t"      
".align 4                    \n\t"
"ltop_limm:                       \n\t"

        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "pcmpeqd %%mm4, %%mm0          \n\t"
        "pcmpeqd %%mm4, %%mm1          \n\t"
        "pandn    %%mm6, %%mm1          \n\t"
        "pandn    %%mm1, %%mm0          \n\t" 
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "pcmpeqd %%mm4, %%mm2          \n\t"
        "pcmpeqd %%mm4, %%mm3          \n\t"
        "pandn    %%mm6, %%mm3          \n\t"
        "pandn    %%mm3, %%mm2          \n\t"
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "pcmpeqd %%mm4, %%mm0          \n\t"
        "pcmpeqd %%mm4, %%mm1          \n\t"
        "pandn    %%mm6, %%mm1          \n\t"
        "pandn    %%mm1, %%mm0          \n\t"
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "pcmpeqd %%mm4, %%mm2          \n\t"
        "pcmpeqd %%mm4, %%mm3          \n\t"
        "pandn    %%mm6, %%mm3          \n\t"
        "pandn    %%mm3, %%mm2          \n\t"
        "movq    %%mm2, -8(%%eax)      \n\t"      
        "jne     ltop_limm                \n\t"      

"unroll_end_limm:                 \n\t"
        "movb    %%bl, %%dl            \n\t"    
        "andb    $7, %%dl             \n\t"      
        "shrb    $1,%%dl                 \n\t"      
        "jz      norest_limm              \n\t"      
".align 4                    \n\t"
"lrest_limm:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "pcmpeqd %%mm4, %%mm0          \n\t"
        "pcmpeqd %%mm4, %%mm1          \n\t"
        "pandn    %%mm6, %%mm1          \n\t"
        "pandn    %%mm1, %%mm0          \n\t"
        "decb    %%dl                 \n\t"
       
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "jne     lrest_limm               \n\t"      
"norest_limm:                     \n\t"

        "emms                        \n\t"      
        "testb   $1, %%bl             \n\t"      
        "jz      end_limm                 \n\t"      

        "movl    (%%ecx,%%eax,), %%edx  \n\t"      
        "testl   %%edx, %%edx          \n\t"
        "je      result1_limm             \n\t"      
        "cmpl    $0, (%%eax)          \n\t"
        "je      end_limm                 \n\t"      
        "movl    $1, %%edx            \n\t"
"result1_limm:                    \n\t"
        "movl    %%edx, (%%eax)        \n\t"
"end_limm:                        \n\t"
       
        "popl    %%ebx                \n\t"
       

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


void land_short_mmx(short *inbuf,short *inoutbuf,unsigned int len) {
asm (
        "pushl   %%ebx                \n\t"

        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_lsmm             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      
        "shrl    $1, %%edx                \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_lsmm               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      

"match_lsmm:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $2, %%edx            \n\t"      
        "jz      remain_lsmm              \n\t"
        "movq    oneShort, %%mm6      \n\t"      
        "pxor    %%mm4, %%mm4          \n\t"      

"al_top_lsmm:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "pcmpeqw %%mm4, %%mm0          \n\t"      
        "pcmpeqw %%mm4, %%mm1          \n\t"      
        "pandn    %%mm6, %%mm1          \n\t"      
        "decl    %%edx                \n\t"
        "pandn    %%mm1, %%mm0          \n\t"      
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "jne     al_top_lsmm              \n\t"

"remain_lsmm:                     \n\t"
        "andb    $3, %%bl             \n\t"
        "jz      startit_lsmm             \n\t"

"al_rest_lsmm:                    \n\t"
        "movw    (%%ecx,%%eax,), %%dx   \n\t"      
        "testw   %%dx, %%dx            \n\t"      
        "je      result_lsmm              \n\t"      
        "cmpw    $0, (%%eax)          \n\t"
        "je      nomove_lsmm              \n\t"      
        "movw    $1, %%dx             \n\t"      
"result_lsmm:                     \n\t"
        "movw    %%dx, (%%eax)         \n\t"
"nomove_lsmm:                     \n\t"
        "addl    $2, %%eax            \n\t"
        "decb    %%bl                 \n\t"
        "jnz     al_rest_lsmm             \n\t"
        "jmp     startit_lsmm             \n\t"

"aligned_lsmm:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_lsmm:                    \n\t"
        "movq    oneShort, %%mm6      \n\t"      
        "pxor    %%mm4, %%mm4          \n\t"      
        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $4, %%edx            \n\t"      
        "jz      unroll_end_lsmm          \n\t"      
".align 4                    \n\t"
"ltop_lsmm:                       \n\t"

        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "pcmpeqw %%mm4, %%mm0          \n\t"
        "pcmpeqw %%mm4, %%mm1          \n\t"
        "pandn    %%mm6, %%mm1          \n\t"
        "pandn    %%mm1, %%mm0          \n\t" 
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "pcmpeqw %%mm4, %%mm2          \n\t"
        "pcmpeqw %%mm4, %%mm3          \n\t"
        "pandn    %%mm6, %%mm3          \n\t"
        "pandn    %%mm3, %%mm2          \n\t"
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "pcmpeqw %%mm4, %%mm0          \n\t"
        "pcmpeqw %%mm4, %%mm1          \n\t"
        "pandn    %%mm6, %%mm1          \n\t"
        "pandn    %%mm1, %%mm0          \n\t"
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        
        "pcmpeqw %%mm4, %%mm3          \n\t"
        "pcmpeqw    %%mm4, %%mm2          \n\t"
        "pandn    %%mm6, %%mm3          \n\t"
	"pandn    %%mm3, %%mm2          \n\t"
        "movq    %%mm2, -8(%%eax)      \n\t"      
        "jne     ltop_lsmm                \n\t"      

"unroll_end_lsmm:                 \n\t"
        "movb    %%bl, %%dl            \n\t"      
        "andb    $15, %%dl            \n\t"      
        "shrb    $2, %%dl             \n\t"      
        "jz      norest_lsmm              \n\t"      
"lrest_lsmm:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "pcmpeqw %%mm4, %%mm0          \n\t"
        "pcmpeqw %%mm4, %%mm1          \n\t"
        "pandn    %%mm6, %%mm1          \n\t"
        "decb    %%dl                 \n\t"
        "pandn    %%mm1, %%mm0          \n\t"
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "jne     lrest_lsmm               \n\t"      
"norest_lsmm:                     \n\t"

        "emms                        \n\t"      
        "andb    $3, %%bl             \n\t"      
        "jz      end_lsmm                 \n\t"      
".align 4                    \n\t"
"lrest1_lsmm:                     \n\t"
        "movw    (%%ecx,%%eax,), %%dx   \n\t"      
        "testw   %%dx, %%dx            \n\t"
        "je      result1_lsmm             \n\t"      
        "cmpw    $0, (%%eax)          \n\t"
        "je      nomove1_lsmm             \n\t"
        "movw    $1, %%dx             \n\t"
"result1_lsmm:                    \n\t"
        "movw    %%dx, (%%eax)         \n\t"
"nomove1_lsmm:                    \n\t"
        "addl    $2, %%eax            \n\t"
        "decb    %%bl                 \n\t"
        "jnz     lrest1_lsmm              \n\t"

"end_lsmm:                        \n\t"
        "popl    %%ebx                \n\t"
       

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


void land_byte_mmx(char *inbuf,char *inoutbuf,unsigned int len) {
asm (
        "pushl   %%ebx                \n\t"

        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_lbmm             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      
                                                

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_lbmm               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      

"match_lbmm:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      remain_lbmm              \n\t"
        "movq    oneByte, %%mm6       \n\t"      
        "pxor    %%mm4, %%mm4          \n\t"      
".align 4                    \n\t"
"al_top_lbmm:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
           
        "pcmpeqb %%mm4, %%mm1          \n\t"   
        "pcmpeqb %%mm4, %%mm0          \n\t"      
        "pand    %%mm6, %%mm1          \n\t"
        "pand    %%mm1, %%mm0          \n\t"        
        "decl    %%edx                \n\t"
       
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "jne     al_top_lbmm              \n\t"

"remain_lbmm:                     \n\t"
        "andb    $7, %%bl             \n\t"
        "jz      startit_lbmm             \n\t"

"al_rest_lbmm:                    \n\t"
        "movb    (%%ecx,%%eax,), %%dl   \n\t"      
        "testb   %%dl, %%dl            \n\t"      
        "je      result_lbmm              \n\t"      
        "cmpb    $0, (%%eax)          \n\t"
        "je      nomove_lbmm              \n\t"      
        "movb    $1, %%dl             \n\t"
"result_lbmm:                     \n\t"
        "movb    %%dl, (%%eax)         \n\t"
"nomove_lbmm:                     \n\t"
        "incl    %%eax                \n\t"
        "decb    %%bl                 \n\t"
        "jnz     al_rest_lbmm             \n\t"
        "jmp     startit_lbmm             \n\t"

"aligned_lbmm:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_lbmm:                    \n\t"
        "movq    oneByte, %%mm6       \n\t"      
        "pxor    %%mm4, %%mm4          \n\t"      
        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $5, %%edx            \n\t"      
        "jz      unroll_end_lbmm          \n\t"      
".align 4                    \n\t"
"ltop_lbmm:                       \n\t"

        "movq    (%%ecx,%%eax,), %%mm1  \n\t"      
        "movq    (%%eax), %%mm0        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        
        "pcmpeqb %%mm4, %%mm1          \n\t"
	"pcmpeqb %%mm4, %%mm0          \n\t"
        "pandn    %%mm6, %%mm1          \n\t"
        "pandn    %%mm1, %%mm0          \n\t" 
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        
        "pcmpeqb %%mm4, %%mm3          \n\t"
	"pcmpeqb %%mm4, %%mm2          \n\t"
        "pandn    %%mm6, %%mm3          \n\t"
        "pandn    %%mm3, %%mm2          \n\t"
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

       
        "pcmpeqb %%mm4, %%mm1          \n\t"
        "pcmpeqb %%mm4, %%mm0          \n\t"	
        "pandn    %%mm6, %%mm1          \n\t"
        "pandn    %%mm1, %%mm0          \n\t"
        "movq    %%mm0, -16(%%eax)     \n\t"      

         
        "pcmpeqb %%mm4, %%mm2          \n\t"
        "pcmpeqb %%mm4, %%mm3          \n\t"
        "decl    %%edx                \n\t"     
        "pandn    %%mm6, %%mm3          \n\t"
        "pandn    %%mm3, %%mm2          \n\t"
        "movq    %%mm2, -8(%%eax)      \n\t"      
        "jne     ltop_lbmm                \n\t"      

"unroll_end_lbmm:                 \n\t"
        "movb    %%bl, %%dl          \n\t"      
        "andb    $31, %%dl           \n\t"      
        "shrb    $3, %%dl            \n\t"      
        "jz      norest_lbmm              \n\t"      
"lrest_lbmm:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "pcmpeqb %%mm4, %%mm0          \n\t"
        "pcmpeqb %%mm4, %%mm1          \n\t"
        "decb    %%dl                \n\t" 
        "pandn    %%mm6, %%mm1          \n\t"
        "pandn    %%mm1, %%mm0          \n\t"
        "movq    %%mm0, -8(%%eax)      \n\t"      
             
        "jne     lrest_lbmm               \n\t"      
"norest_lbmm:                     \n\t"

        "emms                        \n\t"      
        "andb    $7, %%bl             \n\t"      
        "jz      end_lbmm                 \n\t"      
".align 4                    \n\t"
"lrest1_lbmm:                     \n\t"
        "movb    (%%ecx,%%eax,), %%dl   \n\t"      
        "testb   %%dl, %%dl            \n\t"
        "je      result1_lbmm             \n\t"      
        "cmpb    $0, (%%eax)          \n\t"
        "je      nomove1_lbmm             \n\t"      
        "movb    $1, %%dl             \n\t"
"result1_lbmm:                    \n\t"
        "movb    %%dl, (%%eax)         \n\t"
"nomove1_lbmm:                    \n\t"
        "incl    %%eax                \n\t"
        "decb    %%bl                 \n\t"
        "jnz     lrest1_lbmm              \n\t"

"end_lbmm:                        \n\t"
        "popl    %%ebx                \n\t"
        

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}

/* ---------------------MPI_LOR----------------------------------*/
void lor_error_func(void* d1,void*d2,unsigned int l) {
    MPIR_Op_errno = MPIR_ERRCLASS_TO_CODE(MPI_ERR_OP,MPIR_ERR_NOT_DEFINED);
    MPIR_ERROR(MPIR_COMM_WORLD,MPIR_Op_errno, "MPI_LOR" );
}

static void lor_int_mmx(int *inbuf,int*inoutbuf,unsigned int len);
static void lor_short_mmx(short *inbuf,short *inoutbuf,unsigned int len);
static void lor_byte_mmx(char *inbuf, char *inoutbuf,unsigned int len);


static const proto_func jmp_lor_mmx[] = {
    lor_int_mmx,   /*int*/
    lor_error_func, /*float*/
    lor_error_func,/*double*/
    lor_error_func, /*complex*/
    lor_int_mmx,   /*long*/
    lor_short_mmx, /*short*/
    lor_byte_mmx,  /*char*/
    lor_byte_mmx, /*byte*/
    lor_byte_mmx, /*uchar*/
    lor_short_mmx,/*ushort*/
    lor_int_mmx,  /*ulong*/
    lor_int_mmx,  /*uint*/
    lor_error_func, /*contig*/
    lor_error_func, /*vector*/
    lor_error_func, /*hvector*/
    lor_error_func, /*indexed*/
    lor_error_func, /*hindexed*/
    lor_error_func, /*struct*/
    lor_error_func, /*double_complex*/
    lor_error_func, /*packed*/
    lor_error_func, /*ub*/
    lor_error_func, /*lb*/
    lor_error_func, /*longdouble*/
    lor_error_func, /*longlongint*/
    lor_int_mmx,   /*logical*/
    lor_int_mmx    /*fort_int*/
};

void MPIR_lor_mmx(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_lor_mmx[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}


static void lor_int_mmx(int *inbuf,int*inoutbuf,unsigned int len) {
asm (
        "pushl   %%ebx                \n\t"

        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_limmx             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      
        "shrl    $2, %%edx                \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_limmx               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      

"match_limmx:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    %%edx                \n\t"      
        "jz      remain_limmx              \n\t"
        "movq    oneInt, %%mm6        \n\t"      
        "pxor    %%mm4, %%mm4          \n\t"      

"al_top_limmx:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "pcmpeqd %%mm4, %%mm0          \n\t"      
        "pcmpeqd %%mm4, %%mm1          \n\t"
	"pandn    %%mm6, %%mm0          \n\t"      
	"pandn    %%mm6, %%mm1          \n\t"      
        "por     %%mm1, %%mm0          \n\t"      
        "decl    %%edx                \n\t"
        
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "jne     al_top_limmx              \n\t"

"remain_limmx:                     \n\t"
        "testb   $1, %%bl             \n\t"
        "jz      startit_limmx             \n\t"

        "movl    (%%ecx,%%eax,), %%edx  \n\t"      
        "testl   %%edx, %%edx          \n\t"      
        "jne     restrue_limmx             \n\t"      
        "cmpl    $0, (%%eax)          \n\t"
        "je      nomove_limmx              \n\t"      
"restrue_limmx:                    \n\t"
        "movl    $1, (%%eax)          \n\t"
"nomove_limmx:                     \n\t"
        "addl    $4, %%eax            \n\t"
        "jmp     startit_limmx             \n\t"

"aligned_limmx:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_limmx:                    \n\t"
        "movq    oneInt, %%mm6        \n\t"      
        "pxor    %%mm4, %%mm4          \n\t"      
        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      unroll_end_limmx          \n\t"      
".align 4                    \n\t"
"ltop_limmx:                       \n\t"

        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "pcmpeqd %%mm4, %%mm0          \n\t"
        "pcmpeqd %%mm4, %%mm1          \n\t"
	"pandn    %%mm6, %%mm0          \n\t" 
	"pandn    %%mm6, %%mm1          \n\t" 
        "por     %%mm1, %%mm0          \n\t"
        
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "pcmpeqd %%mm4, %%mm2          \n\t"
        "pcmpeqd %%mm4, %%mm3          \n\t"
	"pandn    %%mm6, %%mm2          \n\t"
	"pandn    %%mm6, %%mm3          \n\t"
        "por     %%mm3, %%mm2          \n\t"
        
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "pcmpeqd %%mm4, %%mm0          \n\t"
        "pcmpeqd %%mm4, %%mm1          \n\t"
	"pandn    %%mm6, %%mm0          \n\t"
	"pandn    %%mm6, %%mm1          \n\t"
        "por     %%mm1, %%mm0          \n\t"
        
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "pcmpeqd %%mm4, %%mm2          \n\t"
        "pcmpeqd %%mm4, %%mm3          \n\t"
	"pandn    %%mm6, %%mm2          \n\t"
	"pandn    %%mm6, %%mm3          \n\t"

        "por     %%mm3, %%mm2          \n\t"
       
        "movq    %%mm2, -8(%%eax)      \n\t"      
        "jne     ltop_limmx                \n\t"      

"unroll_end_limmx:                 \n\t"
        "movb    %%bl, %%dl            \n\t"      
        "andb    $7, %%dl             \n\t"      
        "shrb    %%dl                 \n\t"      
        "jz      norest_limmx              \n\t"      
".align 4                    \n\t"
"lrest_limmx:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "pcmpeqd %%mm4, %%mm0          \n\t"
        "pcmpeqd %%mm4, %%mm1          \n\t"
	"pandn    %%mm6, %%mm0          \n\t"
	"pandn    %%mm6, %%mm1          \n\t"
        "por     %%mm1, %%mm0          \n\t"
        "decb    %%dl                 \n\t"
        
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "jne     lrest_limmx               \n\t"      
"norest_limmx:                     \n\t"

        "emms                        \n\t"      
        "testb   $1, %%bl             \n\t"      
        "jz      end_limmx                 \n\t"      

        "movl    (%%ecx,%%eax,), %%edx  \n\t"      
        "testl   %%edx, %%edx          \n\t"
        "jne     restrue1_limmx            \n\t"
        "cmpl    $0, (%%eax)          \n\t"
        "je      end_limmx                 \n\t"      
"restrue1_limmx:                   \n\t"
        "movl    $1, (%%eax)          \n\t"
"end_limmx:                        \n\t"
        "popl    %%ebx                \n\t"
        

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void lor_short_mmx(short *inbuf,short *inoutbuf,unsigned int len) {
asm (
        "pushl   %%ebx                \n\t"

        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_lsmmx             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      
        "shrl    $1, %%edx                \n\t"      

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_lsmmx               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      

"match_lsmmx:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $2, %%edx            \n\t"      
        "jz      remain_lsmmx              \n\t"
        "movq    oneShort, %%mm6      \n\t"      
        "pxor    %%mm4, %%mm4          \n\t"      

"al_top_lsmmx:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "pcmpeqw %%mm4, %%mm0          \n\t"      
        "pcmpeqw %%mm4, %%mm1          \n\t"
	"pandn    %%mm6, %%mm0          \n\t"  
        "pandn    %%mm6, %%mm1          \n\t"  	
        "por     %%mm1, %%mm0          \n\t"      
        "decl    %%edx                \n\t"
       
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "jne     al_top_lsmmx              \n\t"

"remain_lsmmx:                     \n\t"
        "andb    $3, %%bl             \n\t"
        "jz      startit_lsmmx             \n\t"

"al_rest_lsmmx:                    \n\t"
        "movw    (%%ecx,%%eax,), %%dx   \n\t"      
        "testw   %%dx, %%dx            \n\t"      
        "jne     restrue_lsmmx             \n\t"
        "cmpw    $0, (%%eax)          \n\t"
        "je      nomove_lsmmx              \n\t"      
"restrue_lsmmx:                    \n\t"
        "movw    $1, (%%eax)          \n\t"
"nomove_lsmmx:                     \n\t"
        "addl    $2, %%eax            \n\t"
        "decb    %%bl                 \n\t"
        "jnz     al_rest_lsmmx             \n\t"
        "jmp     startit_lsmmx             \n\t"

"aligned_lsmmx:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_lsmmx:                    \n\t"
        "movq    oneShort, %%mm6      \n\t"      
        "pxor    %%mm4, %%mm4          \n\t"      
        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $4, %%edx            \n\t"      
        "jz      unroll_end_lsmmx          \n\t"      
".align 4                    \n\t"
"ltop_lsmmx:                       \n\t"

        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "pcmpeqw %%mm4, %%mm0          \n\t"
        "pcmpeqw %%mm4, %%mm1          \n\t"
	"pandn    %%mm6, %%mm0          \n\t"
        "pandn    %%mm6, %%mm1          \n\t"	
        "por     %%mm1, %%mm0          \n\t"
        
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "pcmpeqw %%mm4, %%mm2          \n\t"
        "pcmpeqw %%mm4, %%mm3          \n\t"
	"pandn    %%mm6, %%mm2          \n\t" 
	"pandn    %%mm6, %%mm3          \n\t" 
        "por     %%mm3, %%mm2          \n\t"
       
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "pcmpeqw %%mm4, %%mm0          \n\t"
        "pcmpeqw %%mm4, %%mm1          \n\t"
	"pandn    %%mm6, %%mm0          \n\t"
        "pandn    %%mm6, %%mm1          \n\t"	
        "por     %%mm1, %%mm0          \n\t"
        
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "pcmpeqw %%mm4, %%mm2          \n\t"
        "pcmpeqw %%mm4, %%mm3          \n\t"
	"pandn    %%mm6, %%mm2          \n\t"
	"pandn    %%mm6, %%mm3          \n\t"
        "por     %%mm3, %%mm2          \n\t"
        
        "movq    %%mm2, -8(%%eax)      \n\t"      
        "jne     ltop_lsmmx                \n\t"      

"unroll_end_lsmmx:                 \n\t"
        "movb    %%bl, %%dl            \n\t"      
        "andb    $15, %%dl            \n\t"      
        "shrb    $2, %%dl             \n\t"      
        "jz      norest_lsmmx              \n\t"      
"lrest_lsmmx:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "pcmpeqw %%mm4, %%mm0          \n\t"
        "pcmpeqw %%mm4, %%mm1          \n\t"
        "por     %%mm1, %%mm0          \n\t"
        "decb    %%dl                 \n\t"
        "pand    %%mm6, %%mm0          \n\t"
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "jne     lrest_lsmmx               \n\t"      
"norest_lsmmx:                     \n\t"

        "emms                        \n\t"      
        "andb    $3, %%bl             \n\t"      
        "jz      end_lsmmx                 \n\t"      
".align 4                    \n\t"
"lrest1_lsmmx:                     \n\t"
        "movw    (%%ecx,%%eax,), %%dx   \n\t"      
        "testw   %%dx, %%dx            \n\t"
        "jne     result1_lsmmx             \n\t"      
        "cmpw    $0, (%%eax)          \n\t"
        "je      nomove1_lsmmx             \n\t"
"result1_lsmmx:                    \n\t"
        "movw    $1, (%%eax)          \n\t"
"nomove1_lsmmx:                    \n\t"
        "addl    $2, %%eax            \n\t"
        "decb    %%bl                 \n\t"
        "jnz     lrest1_lsmmx              \n\t"

"end_lsmmx:                        \n\t"
        "popl    %%ebx                \n\t"
        
       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}


static void lor_byte_mmx(char *inbuf,char *inoutbuf,unsigned int len) {
asm (
        "pushl   %%ebx                \n\t"

        "movl    16(%%esp), %%eax      \n\t"      
        "movl    %%eax, %%ecx          \n\t"      

        

        "andl    $31, %%eax                \n\t"      
        "jz      aligned_lbmmx             \n\t"      

        "movl    $32, %%edx                \n\t"      
        "subl    %%eax, %%edx          \n\t"      
                                                

        "movl    %%ecx, %%eax          \n\t"      

        "movl    20(%%esp), %%ecx      \n\t"      
        "cmpl    %%edx, %%ecx          \n\t"      
        "jge     match_lbmmx               \n\t"      
        "movl    %%ecx, %%edx          \n\t"      

"match_lbmmx:                      \n\t"
        "subl    %%edx, %%ecx          \n\t"      
        "movl    %%ecx, 20(%%esp)      \n\t"      

        "movl    %%edx, %%ebx          \n\t"      
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      remain_lbmmx              \n\t"
        "movq    oneByte, %%mm6       \n\t"      
        "pxor    %%mm4, %%mm4          \n\t"      
".align 4                    \n\t"
"al_top_lbmmx:                     \n\t"
        "movq    (%%eax), %%mm1        \n\t"      
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "addl    $8, %%eax            \n\t"
        "pcmpeqb %%mm4, %%mm0          \n\t"      
        "pcmpeqb %%mm4, %%mm1          \n\t"
	"pandn    %%mm6, %%mm0          \n\t"
	"pandn    %%mm6, %%mm1          \n\t"
        "por     %%mm1, %%mm0          \n\t"      
        "decl    %%edx                \n\t"
        
        "movq    %%mm0, -8(%%eax)      \n\t"      

        "jne     al_top_lbmmx              \n\t"

"remain_lbmmx:                     \n\t"
        "andb    $7, %%bl             \n\t"
        "jz      startit_lbmmx             \n\t"

"al_rest_lbmmx:                    \n\t"
        "movb    (%%ecx,%%eax,), %%dl   \n\t"      
        "testb   %%dl, %%dl            \n\t"      
        "jne     restrue_lbmmx             \n\t"
        "cmpb    $0, (%%eax)          \n\t"
        "je      nomove_lbmmx              \n\t"      
"restrue_lbmmx:                    \n\t"
        "movb    $1, (%%eax)          \n\t"
"nomove_lbmmx:                     \n\t"
        "incl    %%eax                \n\t"
        "decb    %%bl                 \n\t"
        "jnz     al_rest_lbmmx             \n\t"
        "jmp     startit_lbmmx             \n\t"

"aligned_lbmmx:                    \n\t"
        "movl    %%ecx, %%eax          \n\t"
        "movl    12(%%esp), %%ecx       \n\t"      
        "subl    %%eax, %%ecx          \n\t"      

"startit_lbmmx:                    \n\t"
        "movq    oneByte, %%mm6       \n\t"      
        "pxor    %%mm4, %%mm4          \n\t"      
        "movl    20(%%esp), %%edx      \n\t"      
        "movl    %%edx, %%ebx          \n\t"      
        "shrl    $5, %%edx            \n\t"      
        "jz      unroll_end_lbmmx          \n\t"      
".align 4                    \n\t"
"ltop_lbmmx:                       \n\t"

        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      

        "addl    $32, %%eax           \n\t"      
                                                
        "movq    -24(%%ecx,%%eax,), %%mm2 \n\t"    
        "movq    -24(%%eax), %%mm3     \n\t"      

        "pcmpeqb %%mm4, %%mm0          \n\t"
        "pcmpeqb %%mm4, %%mm1          \n\t"
	"pandn    %%mm6, %%mm0          \n\t"
	"pandn    %%mm6, %%mm1          \n\t" 
	
        "por     %%mm1, %%mm0          \n\t"
        
        "movq    %%mm0, -32(%%eax)     \n\t"      

        "movq    -16(%%ecx,%%eax,), %%mm0 \n\t"    
        "movq    -16(%%eax), %%mm1     \n\t"      

        "pcmpeqb %%mm4, %%mm2          \n\t"
        "pcmpeqb %%mm4, %%mm3          \n\t"
	"pand    %%mm6, %%mm2          \n\t"
	"pand    %%mm6, %%mm3          \n\t"
        "por     %%mm3, %%mm2          \n\t"
       
        "movq    %%mm2, -24(%%eax)     \n\t"

        "movq    -8(%%ecx,%%eax,), %%mm2 \n\t"     
        "movq    -8(%%eax), %%mm3      \n\t"      

        "pcmpeqb %%mm4, %%mm0          \n\t"
        "pcmpeqb %%mm4, %%mm1          \n\t"
	"pandn    %%mm6, %%mm0          \n\t"
	"pandn    %%mm6, %%mm1          \n\t"
        "por     %%mm1, %%mm0          \n\t"
        
        "movq    %%mm0, -16(%%eax)     \n\t"      

        "decl    %%edx                \n\t"      
        "pcmpeqb %%mm4, %%mm2          \n\t"
        "pcmpeqb %%mm4, %%mm3          \n\t"
	"pandn    %%mm6, %%mm2          \n\t"
	"pandn    %%mm6, %%mm3          \n\t"
        "por     %%mm3, %%mm2          \n\t"
       
        "movq    %%mm2, -8(%%eax)      \n\t"      
        "jne     ltop_lbmmx                \n\t"      

"unroll_end_lbmmx:                 \n\t"
        "movl    %%ebx, %%edx          \n\t"      
        "andl    $31, %%edx           \n\t"      
        "shrl    $3, %%edx            \n\t"      
        "jz      norest_lbmmx              \n\t"      
"lrest_lbmmx:                      \n\t"
        "movq    (%%ecx,%%eax,), %%mm0  \n\t"      
        "movq    (%%eax), %%mm1        \n\t"      
        "addl    $8, %%eax            \n\t"
        "pcmpeqb %%mm4, %%mm0          \n\t"
        "pcmpeqb %%mm4, %%mm1          \n\t"
	"pandn    %%mm6, %%mm0          \n\t"
	"pandn    %%mm6, %%mm1          \n\t"
        "por     %%mm1, %%mm0          \n\t"
        
        "movq    %%mm0, -8(%%eax)      \n\t"      
        "decl    %%edx                \n\t"      
        "jne     lrest_lbmmx               \n\t"      
"norest_lbmmx:                     \n\t"

        "emms                        \n\t"      
        "andb    $7, %%bl             \n\t"      
        "jz      end_lbmmx                 \n\t"      
".align 4                    \n\t"
"lrest1_lbmmx:                     \n\t"
        "movb    (%%ecx,%%eax,), %%dl   \n\t"      
        "testb   %%dl, %%dl            \n\t"
        "jne     result1_lbmmx             \n\t"      
        "cmpb    $0, (%%eax)          \n\t"
        "je      nomove1_lbmmx             \n\t"      
"result1_lbmmx:                    \n\t"
        "movb    $1, (%%eax)          \n\t"
"nomove1_lbmmx:                    \n\t"
        "incl    %%eax                \n\t"
        "decb    %%bl                 \n\t"
        "jnz     lrest1_lbmmx              \n\t"

"end_lbmmx:                        \n\t"
        "popl    %%ebx                \n\t"

       : // FIXASM: output regs/vars go here, e.g.:  "=m" (memory_var)

       : // FIXASM: input regs, e.g.:  "c" (count), "S" (src), "D" (dest)

       : "%eax", "%ecx", "%edx"
    );
}

#endif
