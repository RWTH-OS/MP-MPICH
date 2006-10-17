/* $Id$ */

#ifndef MPID_SMI_SSIDE_MACROS_H
#define MPID_SMI_SSIDE_MACROS_H

#include <stdlib.h>
#include <stdio.h>



#define WANT_DO_LOCAL(n,dtype)	((float)(dtype)->extent /\
							(float)(dtype)->size >= \
							(float)MPID_SMI_cfg.SSIDED_FRAGMENT_MAX / 100.)


#define SSIDE_ERR_CALL(name) {\
		fprintf (stderr, "%d - internal fatal error in %s\n", \
					MPID_SMI_myid, (char *)name != NULL ? name : "<UNKNOWN>");\
		MPID_SMI_Abort (NULL, MPI_ERR_INTERN, NULL); }


/*
 * Accumulate macros for one-sided communication
 */
#define DO_ACCU(type,addr,data,op)	{\
			switch (op) {\
			case MPI_SUM:\
				*(type *)(addr) += *(type *)(data);\
				break;\
			case MPI_REPLACE:\
				*(type *)(addr) = *(type *)(data);\
				break;\
			case MPI_MAX:\
				if (*(type *)(data) > *(type *)(addr))\
					*(type *)(addr) = *(type *)(data);\
				break;\
			case MPI_MIN:\
				if (*(type *)(data) < *(type *)(addr))\
					*(type *)(addr) = *(type *)(data);\
				break;\
			case MPI_PROD:\
				*(type *)(addr) *= *(type *)(data);\
				break;\
			case MPI_DIV:\
				*(type *)(addr) /= *(type *)(data);\
				break;\
			case MPI_MOD:\
				*(type *)(addr) %= *(type *)(data);\
				break;\
			case MPI_LAND:\
				*(type *)(addr) = *(type *)(data) && *(type *)(addr);\
				break;\
			case MPI_BAND:\
				*(type *)(addr) &= *(type *)(data);\
				break;\
			case MPI_LOR:\
				*(type *)(addr) = *(type *)(data) || *(type *)(addr);\
				break;\
			case MPI_BOR:\
				*(type *)(addr) |= *(type *)(data);\
				break;\
			case MPI_LXOR:\
				*(type *)(addr) = (*(type *)(data) != 0) ^ (*(type *)(addr) != 0);\
				break;\
			case MPI_BXOR:\
				*(type *)(addr) ^= *(type *)(data);\
				break;\
			}}
#define DO_ACCU_FLOAT(type,addr,data,op)	{\
			switch (op) {\
			case MPI_SUM:\
				*(type *)(addr) += *(type *)(data);\
				break;\
			case MPI_REPLACE:\
				*(type *)(addr) = *(type *)(data);\
				break;\
			case MPI_MAX:\
				if (*(type *)(data) > *(type *)(addr))\
					*(type *)(addr) = *(type *)(data);\
				break;\
			case MPI_MIN:\
				if (*(type *)(data) < *(type *)(addr))\
					*(type *)(addr) = *(type *)(data);\
				break;\
			case MPI_PROD:\
				*(type *)(addr) *= *(type *)(data);\
				break;\
			case MPI_DIV:\
				*(type *)(addr) /= *(type *)(data);\
				break;\
			}}

#define SQUARE(a) ((a)*(a))

#define DO_ACCU_COMPLEX(type,addr,data,op)	{\
			switch (op) {\
			case MPI_SUM:\
				(*(type *)(addr)).re += (*(type *)(data)).re;\
				(*(type *)(addr)).im += (*(type *)(data)).im;\
				break;\
			case MPI_REPLACE:\
				(*(type *)(addr)).re = (*(type *)(data)).re;\
				(*(type *)(addr)).im = (*(type *)(data)).im;\
				break;\
			case MPI_MAX:\
				if (SQUARE((double)((*(type *)(data)).re)) + \
						SQUARE((double)((*(type *)(data)).im)) >\
						SQUARE((double)((*(type *)(addr)).re)) +\
						SQUARE((double)((*(type *)(addr)).im))) {\
					(*(type *)(addr)).re = (*(type *)(data)).re;\
					(*(type *)(addr)).im = (*(type *)(data)).im;\
				}\
				break;\
			case MPI_MIN:\
				if (SQUARE((double)((*(type *)(data)).re)) + \
						SQUARE((double)((*(type *)(data)).im)) <\
						SQUARE((double)((*(type *)(addr)).re)) +\
						SQUARE((double)((*(type *)(addr)).im))) {\
					(*(type *)(addr)).re = (*(type *)(data)).re;\
					(*(type *)(addr)).im = (*(type *)(data)).im;\
				}\
				break;\
			case MPI_PROD:\
				(*(type *)(addr)).re = (*(type *)(addr)).re *\
									(*(type *)(data)).re -\
									(*(type *)(addr)).im *\
									(*(type *)(data)).im;\
				(*(type *)(addr)).im = (*(type *)(addr)).re *\
									(*(type *)(data)).im +\
									(*(type *)(addr)).im *\
									(*(type *)(data)).re;\
				break;\
			case MPI_DIV:\
				{\
				double	are, aim, dre, dim;\
				are = (*(type *)(addr)).re;\
				aim = (*(type *)(addr)).im;\
				dre = (*(type *)(data)).re;\
				dim = (*(type *)(data)).im;\
				(*(type *)(addr)).re = (are * dre + aim * dim) /\
									(dre * dre + dim * dim);\
				(*(type *)(addr)).im = (aim * dre + are * dim) /\
									(dre * dre + dim * dim);\
				}\
				break;\
			}}




#endif	/* MPID_SMI_SSIDE_MACROS_H */


/*
 * Overrides for XEmacs and vim so that we get a uniform tabbing style.
 * XEmacs/vim will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:tw=0:ts=4:wm=0:
 */
