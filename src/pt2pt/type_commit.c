/*
 *  $Id: type_commit.c,v 1.9 2003/06/05 11:50:37 rainer Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpipt2pt.h"

#ifdef HAVE_WEAK_SYMBOLS

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Type_commit = PMPI_Type_commit
#elif defined(HAVE_ATTRIBUTE_WEAK)
EXPORT_MPI_API int MPI_Type_commit ( MPI_Datatype *datatype ) __attribute__ ((weak, alias ("PMPI_Type_commit")));
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Type_commit  MPI_Type_commit
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Type_commit as PMPI_Type_commit
/* end of weak pragmas */
#endif

/* Include mapping from MPI->PMPI */
#define MPI_BUILD_PROFILING
#include "mpiprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif

#include "type_ff.h"
#include "stdio.h"

/*@
    MPI_Type_commit - Commits the datatype

Input Parameter:
. datatype - datatype (handle) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_TYPE
@*/
EXPORT_MPI_API int MPI_Type_commit ( MPI_Datatype *datatype )
{
    struct MPIR_DATATYPE *dtype_ptr;
    static char myname[] = "MPI_TYPE_COMMIT";
    int mpi_errno = MPI_SUCCESS,dummy=0;
#if 0
    /* XXX debug */
	int i, max;
    MPIR_FF_LIST_ITEM *ltmp;
    MPIR_FF_STACK_ITEM *stmp;
#endif

    dtype_ptr   = MPIR_GET_DTYPE_PTR(*datatype);
    MPIR_TEST_DTYPE(*datatype,dtype_ptr,MPIR_COMM_WORLD,myname);

    /* We could also complain about committing twice, but we chose not to, 
       based on the view that it isn't obviously an error.
       */
   
    /* initialize field known for all datatypes */
    dtype_ptr->known = NULL;
	 
    /* Test for predefined datatypes */
    if (dtype_ptr->basic) {
	dtype_ptr->depth = 1; /* Depth for basic types is 1 */
	dtype_ptr->leaves = 1;
	return MPI_SUCCESS;
    }

    /* Just do the simplest conversion to contiguous where possible */
#if defined(MPID_HAS_HETERO)
    if (!MPID_IS_HETERO)
#endif
	{	
	    if (!(dtype_ptr)->is_contig) {
		/* I want to add a test for the struct { contig, UB } form of
		   variable count strided vectors; this will not have
		   size == extent.  Because of this, using the simple test of
		   size == extent as a filter is not useful.
		*/
		int          j, is_contig;
		MPI_Aint     offset;
		if ((MPI_Aint)dtype_ptr->size == dtype_ptr->extent) {
		    switch (dtype_ptr->dte_type) {
		    case MPIR_STRUCT:
			offset    = dtype_ptr->indices[0];
			/* If the initial offset is not 0, then mark as non-contiguous.
			   This is because many of the quick tests for valid buffers
			   depend on the initial address being valid if is_contig is
			   set */
			is_contig = (offset == 0);
			for (j=0;is_contig && j<dtype_ptr->count-1; j++) {
			    if (!dtype_ptr->old_types[j]->is_contig) { 
				is_contig = 0; break; }
			    if (offset + 
				dtype_ptr->old_types[j]->extent * 
				(MPI_Aint)dtype_ptr->blocklens[j] !=
				dtype_ptr->indices[j+1]) { is_contig = 0; break; }
			    offset += dtype_ptr->old_types[j]->extent * 
				(MPI_Aint)dtype_ptr->blocklens[j];
			}
			if (!dtype_ptr->old_types[dtype_ptr->count-1]->is_contig) 
			    is_contig = 0;
			if (is_contig) {
			    /* Note that since commit is passed the ADDRESS of the
			       datatype, we can replace it.
			       Unfortunately, the initialization code depends on 
			       commit NOT changing the datatype value (in the case that
			       it is a predefined datatype).  We could fix this, 
			       but it seems easier to just call a common "free
			       struct datatype fields" routine
			    */
			    /* MPI_Type_contiguous( ) */
			    /* MPIR_Free_struct_internals( dtype_ptr ); */
			    dtype_ptr->is_contig = 1;
			    dtype_ptr->old_type  = 0;
			    /* If we don't set to null, then the code in type_contig.c
			       will use the extent of type->old_types[0] */
			    /* dtype_ptr->old_type  = dtype_ptr->old_types[0]; */
			    /* PRINTF( "Making structure type contiguous..." ); */
			    /* Should free all old structure members ... */
			}
			break;
		    default:
			/* Just to indicate that we want all the other types to be 
			   ignored */
			break;
		    }
		}
	    }
	}

    /* Init for FF and alculate the depth of the datatype tree 
     do this only for the first commit!!! */
    if (!dtype_ptr->committed) {
	dtype_ptr->ff = NULL;
	dtype_ptr->leaves = 0;
	dtype_ptr->depth  = MPIR_get_dt_depth (dtype_ptr);

	/* We need the stack only for non-contig datatype */
	if (!dtype_ptr->is_contig) {
	    MPIR_build_ff (dtype_ptr,&(dtype_ptr->leaves)); 	    
#if 0
		/* XXX debug */
          printf ("Have build ff_stack and got %d leaves\n",dtype_ptr->leaves);
	      ltmp = dtype_ptr->ff;
	      while (ltmp) {
	      printf ("Stack for leave with size: %d Bytes, si=%d, pos=%d\n",ltmp->contig_size,ltmp->top,ltmp->pos);
	      stmp = ltmp->stack;
	      for (i=0;i<dtype_ptr->depth;i++) {
	      printf ("%d: (e = %d, c = %d)\n",i,stmp[i].extent,stmp[i].count);
	      }
	      ltmp=ltmp->next;
	      }
#endif
	}
    }

    /* Nothing else to do yet */
    dtype_ptr->flatcount = MPIR_Count_contiguous_blocks(dtype_ptr,&dummy);
    (dtype_ptr)->committed = 1;

    return MPI_SUCCESS;
}
