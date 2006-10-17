/* $Id: sse_ops_gcc.c,v 1.5 2001/10/11 12:29:36 stef Exp $ */

#if ( ! defined MPI_LINUX && ! defined MPI_solaris86 )
#error sse_ops_gcc.c is for Linux/Solaris86 only
#else

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

#define _mm_prefetch(addr, hint) asm volatile ("prefetchnta 0x00(%0)\n\t" : : "r" (addr))
#define _MM_HINT_NTA 0


/* ---------------------MPI_SUM----------------------------------*/

static void add_int_sse(void* inbuf, void* inoutbuf, unsigned int len);
static void add_short_sse(void *inbuf, void *inoutbuf,unsigned int len);
static void add_byte_sse(void *inbuf, void *inoutbuf,unsigned int len);
static void add_float_sse(void *inbuf, void* inoutbuf,unsigned int len);
static void add_double_sse(double *inbuf, double* inoutbuf,unsigned int len);
static void add_d_complex_sse(d_complex *inbuf,d_complex *inoutbuf,unsigned int len);
extern void add_error_func(void* d1,void*d2,unsigned int l) ;

static void add_double_sse(double *inbuf,double* inoutbuf,unsigned int len) {
    unsigned int i,u;

    if(!len) return;
    _mm_prefetch((char*)(inbuf+4),_MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+4),_MM_HINT_NTA);
    u=len&(~3);   
    for(i=0;i<u;i+=4) {
	_mm_prefetch((char*)(inbuf+i+8),_MM_HINT_NTA);
	_mm_prefetch((char*)(inoutbuf+i+8),_MM_HINT_NTA);
	inoutbuf[i]+=inbuf[i];
	inoutbuf[i+1]+=inbuf[i+1];
	inoutbuf[i+2]+=inbuf[i+2];
	inoutbuf[i+3]+=inbuf[i+3];
    }
    for(;i<len;++i)
	inoutbuf[i]+=inbuf[i];
}

static void add_s_complex_sse(s_complex *inbuf,s_complex *inoutbuf,unsigned int len);/* {
    unsigned int i,u;

    if(!len) return;
    _mm_prefetch((char*)(inbuf+4),_MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+4),_MM_HINT_NTA);
    u=len&(~3);   
    for(i=0;i<u;i+=4) {
	_mm_prefetch((char*)(inbuf+i+8),_MM_HINT_NTA);
	_mm_prefetch((char*)(inoutbuf+i+8),_MM_HINT_NTA);
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
*/
static void add_d_complex_sse(d_complex *inbuf,d_complex *inoutbuf,unsigned int len) {
    unsigned int i,u;
    if(!len) return;
    _mm_prefetch((char*)(inbuf+2),_MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+2),_MM_HINT_NTA);
    u=len&(~3);   
    for(i=0;i<u;i+=2) {
	_mm_prefetch((char*)(inbuf+i+4),_MM_HINT_NTA);
	_mm_prefetch((char*)(inoutbuf+i+4),_MM_HINT_NTA);
	inoutbuf[i].re+=inbuf[i].re;
	inoutbuf[i].im+=inbuf[i].im;
	
	inoutbuf[i+1].re+=inbuf[i+1].re;
	inoutbuf[i+1].im+=inbuf[i+1].im;
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

static const proto_func jmp_add_sse[] = {
    add_int_sse,    /*int*/
    add_float_sse,    /*float*/
    add_double_sse,    /*double*/
    add_s_complex_sse,  /*complex*/
    add_int_sse,    /*long*/
    add_short_sse,  /*short*/
    add_byte_sse,   /*char*/
    add_byte_sse,   /*byte*/
    add_byte_sse,   /*uchar*/
    add_short_sse,  /*ushort*/
    add_int_sse,    /*ulong*/
    add_int_sse,    /*uint*/
    add_error_func, /*contig*/
    add_error_func, /*vector*/
    add_error_func, /*hvector*/
    add_error_func, /*indexed*/
    add_error_func, /*hindexed*/
    add_error_func, /*struct*/
    add_d_complex_sse,  /*double_complex*/
    add_error_func, /*packed*/
    add_error_func, /*ub*/
    add_error_func, /*lb*/
    add_double_sse,	    /*longdouble*/
    add_error_func, /*longlongint*/
    add_error_func, /*logical*/
    add_int_sse	    /*fort_int*/
};

void MPIR_add_sse(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_add_sse[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}




/* ---------------------MPI_PROD----------------------------------*/
extern void prod_error_func(void* d1,void*d2,unsigned int l) ;

static void prod_short_sse(short *inbuf,short *inoutbuf,unsigned int len);

static void prod_int_sse(int *inbuf,int*inoutbuf,unsigned int len) {
    unsigned int i,u;

    if(!len) return;
    _mm_prefetch((char*)(inbuf+8),_MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+8),_MM_HINT_NTA);
    u=len&(~7);   
    for(i=0;i<u;i+=8) {
	_mm_prefetch((char*)(inbuf+i+16),_MM_HINT_NTA);
	_mm_prefetch((char*)(inoutbuf+i+16),_MM_HINT_NTA);
	inoutbuf[i]*=inbuf[i];
	inoutbuf[i+1]*=inbuf[i+1];
	inoutbuf[i+2]*=inbuf[i+2];
	inoutbuf[i+3]*=inbuf[i+3];
	inoutbuf[i+4]*=inbuf[i+4];
	inoutbuf[i+5]*=inbuf[i+5];
	inoutbuf[i+6]*=inbuf[i+6];
	inoutbuf[i+7]*=inbuf[i+7];
    }
    for(;i<len;++i)
	inoutbuf[i]*=inbuf[i];
}

static void prod_byte_sse(char *inbuf,char *inoutbuf,unsigned int len) {
    unsigned int i,u;

    if(!len) return;
    _mm_prefetch((char*)(inbuf+32),_MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+32),_MM_HINT_NTA);
    u=len&(~31);   
    for(i=0;i<u;i+=32) {
	_mm_prefetch((char*)(inbuf+i+64),_MM_HINT_NTA);
	_mm_prefetch((char*)(inoutbuf+i+64),_MM_HINT_NTA);
	inoutbuf[i]*=inbuf[i];
	inoutbuf[i+1]*=inbuf[i+1];
	inoutbuf[i+2]*=inbuf[i+2];
	inoutbuf[i+3]*=inbuf[i+3];
	inoutbuf[i+4]*=inbuf[i+4];
	inoutbuf[i+5]*=inbuf[i+5];
	inoutbuf[i+6]*=inbuf[i+6];
	inoutbuf[i+7]*=inbuf[i+7];
	inoutbuf[i+8]*=inbuf[i+8];
	inoutbuf[i+9]*=inbuf[i+9];
	inoutbuf[i+10]*=inbuf[i+10];
	inoutbuf[i+11]*=inbuf[i+11];
	inoutbuf[i+12]*=inbuf[i+12];
	inoutbuf[i+13]*=inbuf[i+13];
	inoutbuf[i+14]*=inbuf[i+14];
	inoutbuf[i+15]*=inbuf[i+15];
	inoutbuf[i+16]*=inbuf[i+16];
	inoutbuf[i+17]*=inbuf[i+17];
	inoutbuf[i+18]*=inbuf[i+18];
	inoutbuf[i+19]*=inbuf[i+19];
	inoutbuf[i+20]*=inbuf[i+20];
	inoutbuf[i+21]*=inbuf[i+21];
	inoutbuf[i+22]*=inbuf[i+22];
	inoutbuf[i+23]*=inbuf[i+23];
	inoutbuf[i+24]*=inbuf[i+24];
	inoutbuf[i+25]*=inbuf[i+25];
	inoutbuf[i+26]*=inbuf[i+26];
	inoutbuf[i+27]*=inbuf[i+27];
	inoutbuf[i+28]*=inbuf[i+28];
	inoutbuf[i+29]*=inbuf[i+29];
	inoutbuf[i+30]*=inbuf[i+30];
	inoutbuf[i+31]*=inbuf[i+31];
    }
    for(;i<len;++i)
	inoutbuf[i]*=inbuf[i];
}


static void prod_float_sse(float *inbuf,float* inoutbuf,unsigned int len) ;
/*{
    unsigned int i,u;
    __m128 op1,op2;
    if(!len) return;
    _mm_prefetch(inbuf+8,_MM_HINT_NTA);
    _mm_prefetch(inoutbuf+8,_MM_HINT_NTA);
    u=len&(~7);   
    for(i=0;i<u;i+=8) {
	_mm_prefetch(inbuf+i+16,_MM_HINT_NTA);
	_mm_prefetch(inoutbuf+i+16,_MM_HINT_NTA);
	
	op1=_mm_loadu_ps(inbuf+i);
	op2=_mm_loadu_ps(inoutbuf+i);
	op1=_mm_mul_ps(op1,op2);
	_mm_storeu_ps(inoutbuf+i,op1);

	op1=_mm_loadu_ps(inbuf+i+4);
	op2=_mm_loadu_ps(inoutbuf+i+4);
	op1=_mm_mul_ps(op1,op2);
	_mm_storeu_ps(inoutbuf+i+4,op1);
    }
    for(;i<len;++i) {
	op1=_mm_load_ss(inbuf+i);
	op2=_mm_load_ss(inoutbuf+i);
	op1=_mm_mul_ss(op1,op2);
	_mm_store_ss(inoutbuf+i,op1);
    }
    
}
*/
static void prod_double_sse(double *inbuf,double* inoutbuf,unsigned int len) {
    unsigned int i,u;

    if(!len) return;
    _mm_prefetch((char*)(inbuf+4),_MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+4),_MM_HINT_NTA);
    u=len&(~3);   
    for(i=0;i<u;i+=4) {
	_mm_prefetch((char*)(inbuf+i+8),_MM_HINT_NTA);
	_mm_prefetch((char*)(inoutbuf+i+8),_MM_HINT_NTA);
	inoutbuf[i]*=inbuf[i];
	inoutbuf[i+1]*=inbuf[i+1];
	inoutbuf[i+2]*=inbuf[i+2];
	inoutbuf[i+3]*=inbuf[i+3];
    }
    for(;i<len;++i)
	inoutbuf[i]*=inbuf[i];
}

static void prod_s_complex_sse(s_complex *inbuf,s_complex *inoutbuf,unsigned int len) {
    unsigned int i,u;
    float c;

    if(!len) return;
    _mm_prefetch((char*)(inbuf+4),_MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+4),_MM_HINT_NTA);
    u=len&(~3);

    for(i=0;i<u;i+=4) {
	_mm_prefetch((char*)(inbuf+i+8),_MM_HINT_NTA);
	_mm_prefetch((char*)(inoutbuf+i+8),_MM_HINT_NTA);
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

static void prod_d_complex_sse(d_complex *inbuf,d_complex *inoutbuf,unsigned int len) {
    unsigned int i,u;
    double c;
    if(!len) return;
    _mm_prefetch((char*)(inbuf+2),_MM_HINT_NTA);
    _mm_prefetch((char*)(inoutbuf+2),_MM_HINT_NTA);
    u=len&(~2);
    for(i=0;i<u;i+=2) {
	_mm_prefetch((char*)(inbuf+i+4),_MM_HINT_NTA);
	_mm_prefetch((char*)(inoutbuf+i+4),_MM_HINT_NTA);
	c = inoutbuf[i].re;
	inoutbuf[i].re = inoutbuf[i].re*inbuf[i].re - inoutbuf[i].im*inbuf[i].im;
	inoutbuf[i].im = c*inbuf[i].im+inoutbuf[i].im*inbuf[i].re ;
	
	c = inoutbuf[i+1].re;
	inoutbuf[i+1].re = inoutbuf[i+1].re*inbuf[i+1].re - inoutbuf[i+1].im*inbuf[i+1].im;
	inoutbuf[i+1].im = c*inbuf[i+1].im+inoutbuf[i+1].im*inbuf[i+1].re ;
    }
    for(;i<len;++i) {
	c = inoutbuf[i].re;
	inoutbuf[i].re = inoutbuf[i].re*inbuf[i].re - inoutbuf[i].im*inbuf[i].im;
	inoutbuf[i].im = c*inbuf[i].im+inoutbuf[i].im*inbuf[i].re ;
    }
}

 const proto_func jmp_prod_sse[] = {
    prod_int_sse,	    /*int*/
    prod_float_sse,	    /*float*/
    prod_double_sse,    /*double*/
    prod_s_complex_sse, /*complex*/
    prod_int_sse,	    /*long*/
    prod_short_sse, /*short*/
    prod_byte_sse,	    /*char*/
    prod_byte_sse,	    /*byte*/
    prod_byte_sse,	    /*uchar*/
    prod_short_sse, /*ushort*/
    prod_int_sse,	    /*ulong*/
    prod_int_sse,	    /*uint*/
    prod_error_func,/*contig*/
    prod_error_func,/*vector*/
    prod_error_func,/*hvector*/
    prod_error_func,/*indexed*/
    prod_error_func,/*hindexed*/
    prod_error_func,/*struct*/
    prod_d_complex_sse, /*double_complex*/
    prod_error_func,/*packed*/
    prod_error_func,/*ub*/
    prod_error_func,/*lb*/
    prod_double_sse,    /*longdouble*/
    prod_error_func,/*longlongint*/
    prod_error_func,/*logical*/
    prod_int_sse    /*fort_int*/
};

void MPIR_prod_sse(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_prod_sse[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}




/* ---------------------MPI_MAX----------------------------------*/
extern void max_error_func(void* d1,void*d2,unsigned int l);

static void max_sint_sse(int *inbuf,int*inoutbuf,unsigned int len);
static void max_sshort_sse(short *inbuf,short *inoutbuf,unsigned int len);
static void max_sbyte_sse(signed char *inbuf,signed char *inoutbuf,unsigned int len);

static void max_float_sse(float *inbuf,float *inoutbuf,unsigned int len);
static void max_double_cmov(double *inbuf,double *inoutbuf,unsigned int len);
static void max_uint_cmov(unsigned int *inbuf,unsigned int*inoutbuf,unsigned int len);
static void max_ushort_cmov(unsigned short *inbuf,unsigned short *inoutbuf,unsigned int len);
static void max_ubyte_sse(unsigned char *inbuf,unsigned char *inoutbuf,unsigned int len);


static  const proto_func jmp_max_sse_cmov[] = {
    max_sint_sse,   /*int*/
    max_float_sse, /*float*/
    max_double_cmov,/*double*/
    max_error_func, /*complex*/
    max_sint_sse,   /*long*/
    max_sshort_sse, /*short*/
    max_sbyte_sse,  /*char*/
    max_ubyte_sse, /*byte*/
    max_ubyte_sse, /*uchar*/
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
    max_double_cmov,/*longdouble*/
    max_error_func, /*longlongint*/
    max_error_func, /*logical*/
    max_sint_sse    /*fort_int*/
};

void MPIR_max_sse_cmov(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_max_sse_cmov[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}




/* ---------------------MPI_MIN----------------------------------*/
extern void min_error_func(void* d1,void*d2,unsigned int l);
static void min_sint_sse(int *inbuf,int*inoutbuf,unsigned int len);
static void min_sshort_sse(short *inbuf,short *inoutbuf,unsigned int len);
static void min_sbyte_sse(signed char *inbuf,signed char *inoutbuf,unsigned int len);

static void min_float_sse(float *inbuf,float *inoutbuf,unsigned int len);
static void min_double_cmov(double *inbuf,double *inoutbuf,unsigned int len);
static void min_uint_cmov(unsigned int *inbuf,unsigned int*inoutbuf,unsigned int len);
static void min_ushort_cmov(unsigned short *inbuf,unsigned short *inoutbuf,unsigned int len);
static void min_ubyte_sse(unsigned char *inbuf,unsigned char *inoutbuf,unsigned int len);


static  const proto_func jmp_min_sse_cmov[] = {
    min_sint_sse,   /*int*/
    min_float_sse, /*float*/
    min_double_cmov,/*double*/
    min_error_func, /*complex*/
    min_sint_sse,   /*long*/
    min_sshort_sse, /*short*/
    min_sbyte_sse,  /*char*/
    min_ubyte_sse, /*byte*/
    min_ubyte_sse, /*uchar*/
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
    min_sint_sse    /*fort_int*/
};

void MPIR_min_sse_cmov(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_min_sse_cmov[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}






/* ---------------------MPI_band----------------------------------*/
extern void band_error_func(void* d1,void*d2,unsigned int l);

static void band_int_sse(int *inbuf,int*inoutbuf,unsigned int len);
static void band_short_sse(short *inbuf,short *inoutbuf,unsigned int len);
static void band_byte_sse(signed char *inbuf,signed char *inoutbuf,unsigned int len);


static  const proto_func jmp_band_sse[] = {
    band_int_sse,   /*int*/
    band_error_func, /*float*/
    band_error_func,/*double*/
    band_error_func, /*complex*/
    band_int_sse,   /*long*/
    band_short_sse, /*short*/
    band_byte_sse,  /*char*/
    band_byte_sse, /*byte*/
    band_byte_sse, /*uchar*/
    band_short_sse,/*ushort*/
    band_int_sse,  /*ulong*/
    band_int_sse,  /*uint*/
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
    band_int_sse,   /*logical*/
    band_int_sse    /*fort_int*/
};

void MPIR_band_sse(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_band_sse[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}





/* ---------------------MPI_BOR----------------------------------*/
extern void bor_error_func(void* d1,void*d2,unsigned int l);

static void bor_int_sse(int *inbuf,int*inoutbuf,unsigned int len);
static void bor_short_sse(short *inbuf,short *inoutbuf,unsigned int len);
static void bor_byte_sse(signed char *inbuf,signed char *inoutbuf,unsigned int len);


static  const proto_func jmp_bor_sse[] = {
    bor_int_sse,   /*int*/
    bor_error_func, /*float*/
    bor_error_func,/*double*/
    bor_error_func, /*complex*/
    bor_int_sse,   /*long*/
    bor_short_sse, /*short*/
    bor_byte_sse,  /*char*/
    bor_byte_sse, /*byte*/
    bor_byte_sse, /*uchar*/
    bor_short_sse,/*ushort*/
    bor_int_sse,  /*ulong*/
    bor_int_sse,  /*uint*/
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
    bor_int_sse,   /*logical*/
    bor_int_sse    /*fort_int*/
};

void MPIR_bor_sse(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_bor_sse[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}





/* ---------------------MPI_BXOR----------------------------------*/
extern void bxor_error_func(void* d1,void*d2,unsigned int l);

static void bxor_int_sse(int *inbuf,int*inoutbuf,unsigned int len);
static void bxor_short_sse(short *inbuf,short *inoutbuf,unsigned int len);
static void bxor_byte_sse(signed char *inbuf,signed char *inoutbuf,unsigned int len);


static  const proto_func jmp_bxor_sse[] = {
    bxor_int_sse,   /*int*/
    bxor_error_func, /*float*/
    bxor_error_func,/*double*/
    bxor_error_func, /*complex*/
    bxor_int_sse,   /*long*/
    bxor_short_sse, /*short*/
    bxor_byte_sse,  /*char*/
    bxor_byte_sse, /*byte*/
    bxor_byte_sse, /*uchar*/
    bxor_short_sse,/*ushort*/
    bxor_int_sse,  /*ulong*/
    bxor_int_sse,  /*uint*/
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
    bxor_int_sse,   /*logical*/
    bxor_int_sse    /*fort_int*/
};

void MPIR_bxor_sse(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_bxor_sse[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}





/* ---------------------MPI_LXOR----------------------------------*/
extern void lxor_error_func(void* d1,void*d2,unsigned int l);

static void lxor_int_sse(int *inbuf,int*inoutbuf,unsigned int len);
static void lxor_short_sse(short *inbuf,short *inoutbuf,unsigned int len);
static void lxor_byte_sse(signed char *inbuf,signed char *inoutbuf,unsigned int len);


static  const proto_func jmp_lxor_sse[] = {
    lxor_int_sse,   /*int*/
    lxor_error_func, /*float*/
    lxor_error_func,/*double*/
    lxor_error_func, /*complex*/
    lxor_int_sse,   /*long*/
    lxor_short_sse, /*short*/
    lxor_byte_sse,  /*char*/
    lxor_byte_sse, /*byte*/
    lxor_byte_sse, /*uchar*/
    lxor_short_sse,/*ushort*/
    lxor_int_sse,  /*ulong*/
    lxor_int_sse,  /*uint*/
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
    lxor_int_sse,   /*logical*/
    lxor_int_sse    /*fort_int*/
};

void MPIR_lxor_sse(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_lxor_sse[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}





/* ---------------------MPI_LAND----------------------------------*/
extern void land_error_func(void* d1,void*d2,unsigned int l);

static void land_int_sse(int *inbuf,int*inoutbuf,unsigned int len);
static void land_short_sse(short *inbuf,short *inoutbuf,unsigned int len);
static void land_byte_sse(signed char *inbuf,signed char *inoutbuf,unsigned int len);


static  const proto_func jmp_land_sse[] = {
    land_int_sse,   /*int*/
    land_error_func, /*float*/
    land_error_func,/*double*/
    land_error_func, /*complex*/
    land_int_sse,   /*long*/
    land_short_sse, /*short*/
    land_byte_sse,  /*char*/
    land_byte_sse, /*byte*/
    land_byte_sse, /*uchar*/
    land_short_sse,/*ushort*/
    land_int_sse,  /*ulong*/
    land_int_sse,  /*uint*/
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
    land_int_sse,   /*logical*/
    land_int_sse    /*fort_int*/
};

void MPIR_land_sse(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_land_sse[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}






/* ---------------------MPI_LOR----------------------------------*/
extern void lor_error_func(void* d1,void*d2,unsigned int l);

static void lor_int_sse(int *inbuf,int*inoutbuf,unsigned int len);
static void lor_short_sse(short *inbuf,short *inoutbuf,unsigned int len);
static void lor_byte_sse(signed char *inbuf,signed char *inoutbuf,unsigned int len);


static  const proto_func jmp_lor_sse[] = {
    lor_int_sse,   /*int*/
    lor_error_func, /*float*/
    lor_error_func,/*double*/
    lor_error_func, /*complex*/
    lor_int_sse,   /*long*/
    lor_short_sse, /*short*/
    lor_byte_sse,  /*char*/
    lor_byte_sse, /*byte*/
    lor_byte_sse, /*uchar*/
    lor_short_sse,/*ushort*/
    lor_int_sse,  /*ulong*/
    lor_int_sse,  /*uint*/
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
    lor_int_sse,   /*logical*/
    lor_int_sse    /*fort_int*/
};

void MPIR_lor_sse(void* a,void* b,int *l,MPI_Datatype *type) {    
    jmp_lor_sse[(MPIR_GET_DTYPE_PTR(*type))->dte_type](a,b,*l);
}


#endif
