/*
 *  $Id$
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 *
 * Default MPICH Compute Operations for Reductions
 *
 * These are used to be compared with optimized MMX/SSE/whatever reduce operations.
 * With certain compilers (Intel or PGI), it would also be possbile to add 
 * pragmas/directives to these C functions to have them optimized by the compiler.
 *
 * To be evaluated!
 */

#include <stdio.h>

#include "mpi.h"

int MPIR_Op_errno;

typedef struct {
	float re;
	float im;
} s_complex;

typedef struct {
	double re;
	double im;
} d_complex;


#define MPIR_ERR_OP_NOT_DEFINED MPI_ERR_OP
#define MPIR_ERROR(comm,code,string) \
    fprintf(stderr,"%d: %s in %s (%d)\n",code, string, __FILE__, __LINE__ );fflush(stderr);

#define MPIR_MAX(a,b) ((a)>(b)?(a):(b))
#define MPIR_MIN(a,b) ((a)<(b)?(a):(b))

#define MPIR_TO_FLOG(a) ((a) ? 1:0)
#define MPIR_FROM_FLOG(a) ((a)&1)


#define MPIR_INT MPI_INT
#define MPIR_UINT MPI_UNSIGNED
#define MPIR_LONG MPI_LONG
#define MPIR_ULONG MPI_UNSIGNED_LONG
#define MPIR_LONGLONGINT MPI_LONG_LONG_INT
#define MPIR_SHORT MPI_SHORT
#define MPIR_USHORT MPI_UNSIGNED_SHORT
#define MPIR_CHAR MPI_CHAR
#define MPIR_UCHAR MPI_UNSIGNED_CHAR
#define MPIR_BYTE MPI_BYTE
#define MPIR_FLOAT MPI_FLOAT
#define MPIR_DOUBLE MPI_DOUBLE
#define MPIR_COMPLEX MPI_COMPLEX
#define MPIR_DOUBLE_COMPLEX MPI_DOUBLE_COMPLEX
#define MPIR_LONGDOUBLE MPI_LONG_DOUBLE
#define MPIR_LOGICAL MPI_LOGICAL


void MPICH_DEFAULT_MAXF(void *invec,
		void *inoutvec, int *Len, MPI_Datatype *type)
{
	int i, len = *Len;

	switch (*type) {
	case MPIR_INT:{
			int *a = (int *) inoutvec;
			int *b = (int *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MAX(a[i], b[i]);
			break;
		}
	case MPIR_UINT:{
			unsigned int *a = (unsigned int *) inoutvec;
			unsigned int *b = (unsigned int *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MAX(a[i], b[i]);
			break;
		}
	case MPIR_LONG:{
			long *a = (long *) inoutvec;
			long *b = (long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MAX(a[i], b[i]);
			break;
		}
#if defined(HAVE_LONG_LONG_INT)
	case MPIR_LONGLONGINT:{
			long long *a = (long long *) inoutvec;
			long long *b = (long long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MAX(a[i], b[i]);
			break;
		}
#endif
	case MPIR_ULONG:{
			unsigned long *a = (unsigned long *) inoutvec;
			unsigned long *b = (unsigned long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MAX(a[i], b[i]);
			break;
		}
	case MPIR_SHORT:{
			short *a = (short *) inoutvec;
			short *b = (short *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MAX(a[i], b[i]);
			break;
		}
	case MPIR_USHORT:{
			unsigned short *a = (unsigned short *) inoutvec;
			unsigned short *b = (unsigned short *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MAX(a[i], b[i]);
			break;
		}
	case MPIR_CHAR:{
			char *a = (char *) inoutvec;
			char *b = (char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MAX(a[i], b[i]);
			break;
		}
	case MPIR_UCHAR:
	case MPIR_BYTE:{
			unsigned char *a = (unsigned char *) inoutvec;
			unsigned char *b = (unsigned char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MAX(a[i], b[i]);
			break;
		}
	case MPIR_FLOAT:{
			float *a = (float *) inoutvec;
			float *b = (float *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MAX(a[i], b[i]);
			break;
		}
	case MPIR_DOUBLE:{
			double *a = (double *) inoutvec;
			double *b = (double *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MAX(a[i], b[i]);
			break;
		}
#if defined(HAVE_LONG_DOUBLE)
	case MPIR_LONGDOUBLE:{
			long double *a = (long double *) inoutvec;
			long double *b = (long double *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MAX(a[i], b[i]);
			break;
		}
#endif
	default:
		MPIR_Op_errno = MPIR_ERR_OP_NOT_DEFINED;
		MPIR_ERROR(MPIR_COMM_WORLD, MPIR_ERR_OP_NOT_DEFINED, "MPI_MAX");
		break;
	}
}


void MPICH_DEFAULT_MINF(void *invec,
		void *inoutvec, int *Len, MPI_Datatype *type)
{
	int i, len = *Len;

	switch (*type) {
	case MPIR_INT:{
			int *a = (int *) inoutvec;
			int *b = (int *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MIN(a[i], b[i]);
			break;
		}
	case MPIR_UINT:{
			unsigned *a = (unsigned *) inoutvec;
			unsigned *b = (unsigned *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MIN(a[i], b[i]);
			break;
		}
	case MPIR_LONG:{
			long *a = (long *) inoutvec;
			long *b = (long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MIN(a[i], b[i]);
			break;
		}
#if defined(HAVE_LONG_LONG_INT)
	case MPIR_LONGLONGINT:{
			long long *a = (long long *) inoutvec;
			long long *b = (long long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MIN(a[i], b[i]);
			break;
		}
#endif
	case MPIR_ULONG:{
			unsigned long *a = (unsigned long *) inoutvec;
			unsigned long *b = (unsigned long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MIN(a[i], b[i]);
			break;
		}
	case MPIR_SHORT:{
			short *a = (short *) inoutvec;
			short *b = (short *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MIN(a[i], b[i]);
			break;
		}
	case MPIR_USHORT:{
			unsigned short *a = (unsigned short *) inoutvec;
			unsigned short *b = (unsigned short *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MIN(a[i], b[i]);
			break;
		}
	case MPIR_CHAR:{
			char *a = (char *) inoutvec;
			char *b = (char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MIN(a[i], b[i]);
			break;
		}
	case MPIR_UCHAR:
	case MPI_BYTE:{
			unsigned char *a = (unsigned char *) inoutvec;
			unsigned char *b = (unsigned char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MIN(a[i], b[i]);
			break;
		}
	case MPIR_FLOAT:{
			float *a = (float *) inoutvec;
			float *b = (float *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MIN(a[i], b[i]);
			break;
		}
	case MPIR_DOUBLE:{
			double *a = (double *) inoutvec;
			double *b = (double *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MIN(a[i], b[i]);
			break;
		}
#if defined(HAVE_LONG_DOUBLE)
	case MPIR_LONGDOUBLE:{
			long double *a = (long double *) inoutvec;
			long double *b = (long double *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_MIN(a[i], b[i]);
			break;
		}
#endif
	default:
		MPIR_Op_errno = MPIR_ERR_OP_NOT_DEFINED;
		MPIR_ERROR(MPIR_COMM_WORLD, MPIR_ERR_OP_NOT_DEFINED, "MPI_MIN");
		break;
	}
}

#ifndef MPIR_SUM
#define MPIR_LSUM(a,b) ((a)+(b))
#endif

void MPICH_DEFAULT_SUM(void *invec,
		void *inoutvec, int *Len, MPI_Datatype *type)
{
	int i, len = *Len;

	switch (*type) {
	case MPIR_INT:{
			int *a = (int *) inoutvec;
			int *b = (int *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LSUM(a[i], b[i]);
			break;
		}
	case MPIR_UINT:{
			unsigned *a = (unsigned *) inoutvec;
			unsigned *b = (unsigned *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LSUM(a[i], b[i]);
			break;
		}
	case MPIR_LONG:{
			long *a = (long *) inoutvec;
			long *b = (long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LSUM(a[i], b[i]);
			break;
		}
#if defined(HAVE_LONG_LONG_INT)
	case MPIR_LONGLONGINT:{
			long long *a = (long long *) inoutvec;
			long long *b = (long long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LSUM(a[i], b[i]);
			break;
		}
#endif

	case MPIR_ULONG:{
			unsigned long *a = (unsigned long *) inoutvec;
			unsigned long *b = (unsigned long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LSUM(a[i], b[i]);
			break;
		}
	case MPIR_SHORT:{
			short *a = (short *) inoutvec;
			short *b = (short *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LSUM(a[i], b[i]);
			break;
		}
	case MPIR_USHORT:{
			unsigned short *a = (unsigned short *) inoutvec;
			unsigned short *b = (unsigned short *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LSUM(a[i], b[i]);
			break;
		}
	case MPIR_CHAR:{
			char *a = (char *) inoutvec;
			char *b = (char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LSUM(a[i], b[i]);
			break;
		}
	case MPIR_UCHAR:
	case MPI_BYTE:{
			unsigned char *a = (unsigned char *) inoutvec;
			unsigned char *b = (unsigned char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LSUM(a[i], b[i]);
			break;
		}
	case MPIR_FLOAT:{
			float *a = (float *) inoutvec;
			float *b = (float *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LSUM(a[i], b[i]);
			break;
		}
	case MPIR_DOUBLE:{
			double *a = (double *) inoutvec;
			double *b = (double *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LSUM(a[i], b[i]);
			break;
		}
#if defined(HAVE_LONG_DOUBLE)
	case MPIR_LONGDOUBLE:{
			long double *a = (long double *) inoutvec;
			long double *b = (long double *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LSUM(a[i], b[i]);
			break;
		}
#endif
	case MPIR_COMPLEX:{
			s_complex *a = (s_complex *) inoutvec;
			s_complex *b = (s_complex *) invec;
			for (i = 0; i < len; i++) {
				a[i].re = MPIR_LSUM(a[i].re, b[i].re);
				a[i].im = MPIR_LSUM(a[i].im, b[i].im);
			}
			break;
		}
	case MPIR_DOUBLE_COMPLEX:{
			d_complex *a = (d_complex *) inoutvec;
			d_complex *b = (d_complex *) invec;
			for (i = 0; i < len; i++) {
				a[i].re = MPIR_LSUM(a[i].re, b[i].re);
				a[i].im = MPIR_LSUM(a[i].im, b[i].im);
			}
			break;
		}
	default:
		MPIR_Op_errno = MPIR_ERR_OP_NOT_DEFINED;
		MPIR_ERROR(MPIR_COMM_WORLD, MPIR_ERR_OP_NOT_DEFINED, "MPI_SUM");
		break;
	}
}



#define MPIR_LPROD(a,b) ((a)*(b))

void MPICH_DEFAULT_PROD(void *invec,
		void *inoutvec, int *Len, MPI_Datatype *type)
{
	int i, len = *Len;


	switch (*type) {
	case MPIR_INT:{
			int *a = (int *) inoutvec;
			int *b = (int *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LPROD(a[i], b[i]);
			break;
		}
	case MPIR_UINT:{
			unsigned *a = (unsigned *) inoutvec;
			unsigned *b = (unsigned *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LPROD(a[i], b[i]);
			break;
		}
	case MPIR_LONG:{
			long *a = (long *) inoutvec;
			long *b = (long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LPROD(a[i], b[i]);
			break;
		}
#if defined(HAVE_LONG_LONG_INT)
	case MPIR_LONGLONGINT:{
			long long *a = (long long *) inoutvec;
			long long *b = (long long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LPROD(a[i], b[i]);
			break;
		}
#endif
	case MPIR_ULONG:{
			unsigned long *a = (unsigned long *) inoutvec;
			unsigned long *b = (unsigned long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LPROD(a[i], b[i]);
			break;
		}
	case MPIR_SHORT:{
			short *a = (short *) inoutvec;
			short *b = (short *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LPROD(a[i], b[i]);
			break;
		}
	case MPIR_USHORT:{
			unsigned short *a = (unsigned short *) inoutvec;
			unsigned short *b = (unsigned short *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LPROD(a[i], b[i]);
			break;
		}
	case MPIR_CHAR:{
			char *a = (char *) inoutvec;
			char *b = (char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LPROD(a[i], b[i]);
			break;
		}
	case MPIR_UCHAR:
	case MPI_BYTE:{
			unsigned char *a = (unsigned char *) inoutvec;
			unsigned char *b = (unsigned char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LPROD(a[i], b[i]);
			break;
		}
	case MPIR_FLOAT:{
			float *a = (float *) inoutvec;
			float *b = (float *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LPROD(a[i], b[i]);
			break;
		}
	case MPIR_DOUBLE:{
			double *a = (double *) inoutvec;
			double *b = (double *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LPROD(a[i], b[i]);
			break;
		}
#if defined(HAVE_LONG_DOUBLE)
	case MPIR_LONGDOUBLE:{
			long double *a = (long double *) inoutvec;
			long double *b = (long double *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LPROD(a[i], b[i]);
			break;
		}
#endif
	case MPIR_COMPLEX:{
			s_complex *a = (s_complex *) inoutvec;
			s_complex *b = (s_complex *) invec;
			for (i = 0; i < len; i++) {
				s_complex c;
				c.re = a[i].re;
				c.im = a[i].im;
				a[i].re = c.re * b[i].re - c.im * b[i].im;
				a[i].im = c.im * b[i].re + c.re * b[i].im;
			}
			break;
		}
	case MPIR_DOUBLE_COMPLEX:{
			d_complex *a = (d_complex *) inoutvec;
			d_complex *b = (d_complex *) invec;
			for (i = 0; i < len; i++) {
				d_complex c;
				c.re = a[i].re;
				c.im = a[i].im;
				a[i].re = c.re * b[i].re - c.im * b[i].im;
				a[i].im = c.im * b[i].re + c.re * b[i].im;
			}
			break;
		}
	default:
		MPIR_Op_errno = MPIR_ERR_OP_NOT_DEFINED;
		MPIR_ERROR(MPIR_COMM_WORLD, MPIR_ERR_OP_NOT_DEFINED,
				"MPI_PROD");
		break;
	}
}



#ifndef MPIR_LLAND
#define MPIR_LLAND(a,b) ((a)&&(b))
#endif
void MPICH_DEFAULT_LAND(void *invec,
		void *inoutvec, int *Len, MPI_Datatype *type)
{
	int i, len = *Len;


	switch (*type) {
	case MPIR_INT:{
			int *a = (int *) inoutvec;
			int *b = (int *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLAND(a[i], b[i]);
			break;
		}
	case MPIR_UINT:{
			unsigned *a = (unsigned *) inoutvec;
			unsigned *b = (unsigned *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLAND(a[i], b[i]);
			break;
		}
	case MPIR_LONG:{
			long *a = (long *) inoutvec;
			long *b = (long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLAND(a[i], b[i]);
			break;
		}
#if defined(HAVE_LONG_LONG_INT)
	case MPIR_LONGLONGINT:{
			long long *a = (long long *) inoutvec;
			long long *b = (long long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLAND(a[i], b[i]);
			break;
		}
#endif
	case MPIR_ULONG:{
			unsigned long *a = (unsigned long *) inoutvec;
			unsigned long *b = (unsigned long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLAND(a[i], b[i]);
			break;
		}
	case MPIR_SHORT:{
			short *a = (short *) inoutvec;
			short *b = (short *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLAND(a[i], b[i]);
			break;
		}
	case MPIR_USHORT:{
			unsigned short *a = (unsigned short *) inoutvec;
			unsigned short *b = (unsigned short *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLAND(a[i], b[i]);
			break;
		}
	case MPIR_CHAR:{
			char *a = (char *) inoutvec;
			char *b = (char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLAND(a[i], b[i]);
			break;
		}
	case MPIR_UCHAR:
	case MPIR_BYTE:{
			unsigned char *a = (unsigned char *) inoutvec;
			unsigned char *b = (unsigned char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLAND(a[i], b[i]);
			break;
		}
	case MPIR_FLOAT:{
			float *a = (float *) inoutvec;
			float *b = (float *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLAND(a[i], b[i]);
			break;
		}
	case MPIR_DOUBLE:{
			double *a = (double *) inoutvec;
			double *b = (double *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLAND(a[i], b[i]);
			break;
		}
#if defined(HAVE_LONG_DOUBLE)
	case MPIR_LONGDOUBLE:{
			long double *a = (long double *) inoutvec;
			long double *b = (long double *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLAND(a[i], b[i]);
			break;
		}
#endif
	case MPIR_LOGICAL:{
			MPI_Fint *a = (MPI_Fint *) inoutvec;
			MPI_Fint *b = (MPI_Fint *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_TO_FLOG(MPIR_LLAND(MPIR_FROM_FLOG(a[i]), MPIR_FROM_FLOG(b[i])));
			break;
		}
	default:
		MPIR_Op_errno = MPIR_ERR_OP_NOT_DEFINED;
		MPIR_ERROR(MPIR_COMM_WORLD, MPIR_ERR_OP_NOT_DEFINED,
				"MPI_LAND");
		break;
	}
}



#ifndef MPIR_LBAND
#define MPIR_LBAND(a,b) ((a)&(b))
#endif
void MPICH_DEFAULT_BAND(void *invec,
		void *inoutvec, int *Len, MPI_Datatype *type)
{
	int i, len = *Len;


	switch (*type) {
	case MPIR_LOGICAL:{
			MPI_Fint *a = (MPI_Fint *) inoutvec;
			MPI_Fint *b = (MPI_Fint *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBAND(a[i], b[i]);
			break;
		}
	case MPIR_INT:{
			int *a = (int *) inoutvec;
			int *b = (int *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBAND(a[i], b[i]);
			break;
		}
	case MPIR_UINT:{
			unsigned *a = (unsigned *) inoutvec;
			unsigned *b = (unsigned *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBAND(a[i], b[i]);
			break;
		}
	case MPIR_LONG:{
			long *a = (long *) inoutvec;
			long *b = (long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBAND(a[i], b[i]);
			break;
		}
#if defined(HAVE_LONG_LONG_INT)
	case MPIR_LONGLONGINT:{
			long long *a = (long long *) inoutvec;
			long long *b = (long long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBAND(a[i], b[i]);
			break;
		}
#endif
	case MPIR_ULONG:{
			unsigned long *a = (unsigned long *) inoutvec;
			unsigned long *b = (unsigned long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBAND(a[i], b[i]);
			break;
		}
	case MPIR_SHORT:{
			short *a = (short *) inoutvec;
			short *b = (short *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBAND(a[i], b[i]);
			break;
		}
	case MPIR_USHORT:{
			unsigned short *a = (unsigned short *) inoutvec;
			unsigned short *b = (unsigned short *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBAND(a[i], b[i]);
			break;
		}
	case MPIR_CHAR:{
			char *a = (char *) inoutvec;
			char *b = (char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBAND(a[i], b[i]);
			break;
		}
	case MPIR_UCHAR:{
			unsigned char *a = (unsigned char *) inoutvec;
			unsigned char *b = (unsigned char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBAND(a[i], b[i]);
			break;
		}
	case MPIR_BYTE:{
			unsigned char *a = (unsigned char *) inoutvec;
			unsigned char *b = (unsigned char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBAND(a[i], b[i]);
			break;
		}
	default:
		MPIR_Op_errno = MPIR_ERR_OP_NOT_DEFINED;
		MPIR_ERROR(MPIR_COMM_WORLD, MPIR_ERR_OP_NOT_DEFINED,
				"MPI_BAND");
		break;
	}
}



#ifndef MPIR_LLOR
#define MPIR_LLOR(a,b) ((a)||(b))
#endif
void MPICH_DEFAULT_LOR(void *invec,
		void *inoutvec, int *Len, MPI_Datatype *type)
{
	int i, len = *Len;


	switch (*type) {
	case MPIR_INT:{
			int *a = (int *) inoutvec;
			int *b = (int *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLOR(a[i], b[i]);
			break;
		}
	case MPIR_UINT:{
			unsigned *a = (unsigned *) inoutvec;
			unsigned *b = (unsigned *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLOR(a[i], b[i]);
			break;
		}
	case MPIR_LONG:{
			long *a = (long *) inoutvec;
			long *b = (long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLOR(a[i], b[i]);
			break;
		}
#if defined(HAVE_LONG_LONG_INT)
	case MPIR_LONGLONGINT:{
			long long *a = (long long *) inoutvec;
			long long *b = (long long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLOR(a[i], b[i]);
			break;
		}
#endif
	case MPIR_ULONG:{
			unsigned long *a = (unsigned long *) inoutvec;
			unsigned long *b = (unsigned long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLOR(a[i], b[i]);
			break;
		}
	case MPIR_SHORT:{
			short *a = (short *) inoutvec;
			short *b = (short *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLOR(a[i], b[i]);
			break;
		}
	case MPIR_USHORT:{
			unsigned short *a = (unsigned short *) inoutvec;
			unsigned short *b = (unsigned short *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLOR(a[i], b[i]);
			break;
		}
	case MPIR_CHAR:{
			char *a = (char *) inoutvec;
			char *b = (char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLOR(a[i], b[i]);
			break;
		}
	case MPIR_UCHAR:
	case MPI_BYTE:{
			unsigned char *a = (unsigned char *) inoutvec;
			unsigned char *b = (unsigned char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLOR(a[i], b[i]);
			break;
		}
	case MPIR_FLOAT:{
			float *a = (float *) inoutvec;
			float *b = (float *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLOR(a[i], b[i]);
			break;
		}
	case MPIR_DOUBLE:{
			double *a = (double *) inoutvec;
			double *b = (double *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLOR(a[i], b[i]);
			break;
		}
#if defined(HAVE_LONG_DOUBLE)
	case MPIR_LONGDOUBLE:{
			long double *a = (long double *) inoutvec;
			long double *b = (long double *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLOR(a[i], b[i]);
			break;
		}
#endif
	case MPIR_LOGICAL:{
			MPI_Fint *a = (MPI_Fint *) inoutvec;
			MPI_Fint *b = (MPI_Fint *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_TO_FLOG(MPIR_LLOR(MPIR_FROM_FLOG(a[i]), MPIR_FROM_FLOG(b[i])));
			break;
		}

	default:
		MPIR_Op_errno = MPIR_ERR_OP_NOT_DEFINED;
		MPIR_ERROR(MPIR_COMM_WORLD, MPIR_ERR_OP_NOT_DEFINED, "MPI_LOR");
		break;
	}
}


#ifndef MPIR_LBOR
#define MPIR_LBOR(a,b) ((a)|(b))
#endif
void MPICH_DEFAULT_BOR(void *invec,
		void *inoutvec, int *Len, MPI_Datatype *type)
{
	int i, len = *Len;


	switch (*type) {
	case MPIR_LOGICAL:{
			MPI_Fint *a = (MPI_Fint *) inoutvec;
			MPI_Fint *b = (MPI_Fint *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBOR(a[i], b[i]);
			break;
		}
	case MPIR_INT:{
			int *a = (int *) inoutvec;
			int *b = (int *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBOR(a[i], b[i]);
			break;
		}
	case MPIR_UINT:{
			unsigned *a = (unsigned *) inoutvec;
			unsigned *b = (unsigned *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBOR(a[i], b[i]);
			break;
		}
	case MPIR_LONG:{
			long *a = (long *) inoutvec;
			long *b = (long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBOR(a[i], b[i]);
			break;
		}
#if defined(HAVE_LONG_LONG_INT)
	case MPIR_LONGLONGINT:{
			long long *a = (long long *) inoutvec;
			long long *b = (long long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBOR(a[i], b[i]);
			break;
		}
#endif
	case MPIR_ULONG:{
			unsigned long *a = (unsigned long *) inoutvec;
			unsigned long *b = (unsigned long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBOR(a[i], b[i]);
			break;
		}
	case MPIR_SHORT:{
			short *a = (short *) inoutvec;
			short *b = (short *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBOR(a[i], b[i]);
			break;
		}
	case MPIR_USHORT:{
			unsigned short *a = (unsigned short *) inoutvec;
			unsigned short *b = (unsigned short *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBOR(a[i], b[i]);
			break;
		}
	case MPIR_CHAR:{
			char *a = (char *) inoutvec;
			char *b = (char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBOR(a[i], b[i]);
			break;
		}
	case MPIR_UCHAR:{
			unsigned char *a = (unsigned char *) inoutvec;
			unsigned char *b = (unsigned char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBOR(a[i], b[i]);
			break;
		}
	case MPIR_BYTE:{
			unsigned char *a = (unsigned char *) inoutvec;
			unsigned char *b = (unsigned char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBOR(a[i], b[i]);
			break;
		}
	default:
		MPIR_Op_errno = MPIR_ERR_OP_NOT_DEFINED;
		MPIR_ERROR(MPIR_COMM_WORLD, MPIR_ERR_OP_NOT_DEFINED, "MPI_BOR");
		break;
	}
}



#ifndef MPIR_LLXOR
#define MPIR_LLXOR(a,b) (((a)&&(!b))||((!a)&&(b)))
#endif
void MPICH_DEFAULT_LXOR(void *invec,
		void *inoutvec, int *Len, MPI_Datatype *type)
{
	int i, len = *Len;


	switch (*type) {
	case MPIR_INT:{
			int *a = (int *) inoutvec;
			int *b = (int *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLXOR(a[i], b[i]);
			break;
		}
	case MPIR_UINT:{
			unsigned *a = (unsigned *) inoutvec;
			unsigned *b = (unsigned *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLXOR(a[i], b[i]);
			break;
		}
	case MPIR_LONG:{
			long *a = (long *) inoutvec;
			long *b = (long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLXOR(a[i], b[i]);
			break;
		}
#if defined(HAVE_LONG_LONG_INT)
	case MPIR_LONGLONGINT:{
			long long *a = (long long *) inoutvec;
			long long *b = (long long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLXOR(a[i], b[i]);
			break;
		}
#endif
	case MPIR_ULONG:{
			unsigned long *a = (unsigned long *) inoutvec;
			unsigned long *b = (unsigned long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLXOR(a[i], b[i]);
			break;
		}
	case MPIR_SHORT:{
			short *a = (short *) inoutvec;
			short *b = (short *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLXOR(a[i], b[i]);
			break;
		}
	case MPIR_USHORT:{
			unsigned short *a = (unsigned short *) inoutvec;
			unsigned short *b = (unsigned short *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLXOR(a[i], b[i]);
			break;
		}
	case MPIR_CHAR:{
			char *a = (char *) inoutvec;
			char *b = (char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLXOR(a[i], b[i]);
			break;
		}
	case MPIR_UCHAR:
	case MPI_BYTE:{
			unsigned char *a = (unsigned char *) inoutvec;
			unsigned char *b = (unsigned char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLXOR(a[i], b[i]);
			break;
		}
	case MPIR_FLOAT:{
			float *a = (float *) inoutvec;
			float *b = (float *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLXOR(a[i], b[i]);
			break;
		}
	case MPIR_DOUBLE:{
			double *a = (double *) inoutvec;
			double *b = (double *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLXOR(a[i], b[i]);
			break;
		}
#if defined(HAVE_LONG_DOUBLE)
	case MPIR_LONGDOUBLE:{
			long double *a = (long double *) inoutvec;
			long double *b = (long double *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LLXOR(a[i], b[i]);
			break;
		}
#endif
	case MPIR_LOGICAL:{
			MPI_Fint *a = (MPI_Fint *) inoutvec;
			MPI_Fint *b = (MPI_Fint *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_TO_FLOG(MPIR_LLXOR(MPIR_FROM_FLOG(a[i]), MPIR_FROM_FLOG(b[i])));
			break;
		}
	default:
		MPIR_Op_errno = MPIR_ERR_OP_NOT_DEFINED;
		MPIR_ERROR(MPIR_COMM_WORLD, MPIR_ERR_OP_NOT_DEFINED,
				"MPI_LXOR");
		break;
	}
}



#ifndef MPIR_LBXOR
#define MPIR_LBXOR(a,b) ((a)^(b))
#endif
void MPICH_DEFAULT_BXOR(void *invec,
		void *inoutvec, int *Len, MPI_Datatype *type)
{
	int i, len = *Len;


	switch (*type) {
	case MPIR_LOGICAL:{
			MPI_Fint *a = (MPI_Fint *) inoutvec;
			MPI_Fint *b = (MPI_Fint *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBXOR(a[i], b[i]);
			break;
		}
	case MPIR_INT:{
			int *a = (int *) inoutvec;
			int *b = (int *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBXOR(a[i], b[i]);
			break;
		}
	case MPIR_UINT:{
			unsigned *a = (unsigned *) inoutvec;
			unsigned *b = (unsigned *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBXOR(a[i], b[i]);
			break;
		}
	case MPIR_LONG:{
			long *a = (long *) inoutvec;
			long *b = (long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBXOR(a[i], b[i]);
			break;
		}
#if defined(HAVE_LONG_LONG_INT)
	case MPIR_LONGLONGINT:{
			long long *a = (long long *) inoutvec;
			long long *b = (long long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBXOR(a[i], b[i]);
			break;
		}
#endif
	case MPIR_ULONG:{
			unsigned long *a = (unsigned long *) inoutvec;
			unsigned long *b = (unsigned long *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBXOR(a[i], b[i]);
			break;
		}
	case MPIR_SHORT:{
			short *a = (short *) inoutvec;
			short *b = (short *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBXOR(a[i], b[i]);
			break;
		}
	case MPIR_USHORT:{
			unsigned short *a = (unsigned short *) inoutvec;
			unsigned short *b = (unsigned short *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBXOR(a[i], b[i]);
			break;
		}
	case MPIR_CHAR:{
			char *a = (char *) inoutvec;
			char *b = (char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBXOR(a[i], b[i]);
			break;
		}
	case MPIR_UCHAR:{
			unsigned char *a = (unsigned char *) inoutvec;
			unsigned char *b = (unsigned char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBXOR(a[i], b[i]);
			break;
		}
	case MPIR_BYTE:{
			unsigned char *a = (unsigned char *) inoutvec;
			unsigned char *b = (unsigned char *) invec;
			for (i = 0; i < len; i++)
				a[i] = MPIR_LBXOR(a[i], b[i]);
			break;
		}
	default:
		MPIR_Op_errno = MPIR_ERR_OP_NOT_DEFINED;
		MPIR_ERROR(MPIR_COMM_WORLD, MPIR_ERR_OP_NOT_DEFINED,
				"MPI_BXOR");
		break;
	}
}
