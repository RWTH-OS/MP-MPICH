/*
 *  $Id$
 *
 */


#ifndef _MPIR_X86_OPS_H
#define _MPIR_X86_OPS_H

#ifdef _DEBUG
#define DEBUG(a) a
#else
#define DEBUG(a)
#endif

/* check if we are runing on a x86 architecture */
#ifdef _WIN32
/* AMD64 is also an x86 and offer a SSE unit */
#if defined(_M_IX86) || defined(_M_AMD64)
#define CPU_ARCH_IS_X86 1
#else
#define CPU_ARCH_IS_X86 0
#endif
/* not WIN32, but Unix */
#elif  __GNUC__
/* Gnu C compiler */
#ifdef __i386__
#define CPU_ARCH_IS_X86 1
#else
#define CPU_ARCH_IS_X86 0
#endif
#elif __SUNPRO_C
/* Sun C compiler */
#ifdef __i386
#define CPU_ARCH_IS_X86 1
#else
#define CPU_ARCH_IS_X86 0
#endif
#else
/* unknown */
#define CPU_ARCH_IS_X86 0
#endif

#if CPU_ARCH_IS_X86

typedef struct {
    int cmov,
	mmx,
	sse,
	sse2,
	_3dnow;	
} X86_CPU_feature_t;

extern X86_CPU_feature_t MPIR_X86Features;


void MPIR_test_cpu (X86_CPU_feature_t *);
void MPIR_Setup_x86_reduce_ops(void);

typedef void (*proto_func)(void*,void*,unsigned int); 

extern int MPIR_Op_errno;

void MPIR_add_mmx(void* a,void* b,int *l,MPI_Datatype *type);
void MPIR_prod_mmx(void* a,void* b,int *l,MPI_Datatype *type);
void MPIR_max_mmx_cmov(void* a,void* b,int *l,MPI_Datatype *type);
void MPIR_min_mmx_cmov(void* a,void* b,int *l,MPI_Datatype *type);
void MPIR_band_mmx(void* a,void* b,int *l,MPI_Datatype *type);
void MPIR_bor_mmx(void* a,void* b,int *l,MPI_Datatype *type);
void MPIR_bxor_mmx(void* a,void* b,int *l,MPI_Datatype *type);
void MPIR_land_mmx(void* a,void* b,int *l,MPI_Datatype *type);
void MPIR_lor_mmx(void* a,void* b,int *l,MPI_Datatype *type);
void MPIR_lxor_mmx(void* a,void* b,int *l,MPI_Datatype *type);

void MPIR_add_sse(void* a,void* b,int *l,MPI_Datatype *type);
void MPIR_prod_sse(void* a,void* b,int *l,MPI_Datatype *type);
void MPIR_max_sse_cmov(void* a,void* b,int *l,MPI_Datatype *type);
void MPIR_min_sse_cmov(void* a,void* b,int *l,MPI_Datatype *type);
void MPIR_band_sse(void* a,void* b,int *l,MPI_Datatype *type);
void MPIR_bor_sse(void* a,void* b,int *l,MPI_Datatype *type);
void MPIR_bxor_sse(void* a,void* b,int *l,MPI_Datatype *type);
void MPIR_land_sse(void* a,void* b,int *l,MPI_Datatype *type);
void MPIR_lor_sse(void* a,void* b,int *l,MPI_Datatype *type);
void MPIR_lxor_sse(void* a,void* b,int *l,MPI_Datatype *type);

#endif
#endif
