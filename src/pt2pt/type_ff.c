/* $Id$ 

   MPIR_build_ff - function to build the stack for the "flattening on the fly"-
   algorithm.

   Parameter IN:

   - *type:       Pointer to the toplevel MPIR_DATATYPE structure
   
   This routine uses a iterativ implementation of the "flattening on the fly"-algorithm.
   It builds a stack where all information on the structure for a leave is stored.

*/

#include <stdio.h>
#include <stdlib.h>

#define MPID_INCLUDE_STDIO

#include "mpiimpl.h"
#ifdef malloc
#undef malloc
#undef free
#undef calloc
#endif

#include "type_ff.h"

/* Internal Datatype for the branches */   
struct ff_branch {
    struct MPIR_DATATYPE *subtree; /* Knot where we must continue with 
				      more branches (struct, indexed) */	
    int                     index; /* index to process after return to a knot */
    int                  position; /* Position in src buffer to restart after a branch */
    int                        sp; /* Index for the stack at the moment of branch */
    struct ff_branch        *prev; /* previous in branch list */
};

/* Writes a new stack item */
#define FF_STACK_ADD(STACK,IND,EXT,CNT) IND++; \
                                            STACK[IND].extent = EXT; \
                                            STACK[IND].count = CNT; \

/* Calculates the complete extent of this level */
#define FF_STACK_POS(STACK,IND) ((STACK[IND].count) * (STACK[IND].extent))

/* Writes a new branch list item */
#define FF_BRANCH_ADD(BL,SUB,I,P,SI) if (!BL) { \
                                             BL = (struct ff_branch *) malloc (sizeof(struct ff_branch)); \
                                             BL->prev = NULL; \
                                            } else { \
                                             struct ff_branch *btmp;\
                                             btmp = (struct ff_branch *) malloc (sizeof(struct ff_branch)); \
                                             btmp->prev = BL; \
                                             BL = btmp; \
                                            } \
                                            BL->subtree = SUB; \
                                            BL->index = I; \
                                            BL->position = P; \
                                            BL->sp = SI; 

#define FF_BRANCH_REMOVE(BL) if (BL) { \
                                 struct ff_branch *btmp;\
                                 btmp = BL; \
                                 BL = BL->prev;\
                                 free(btmp); \
                                }
          

int MPIR_build_ff (struct MPIR_DATATYPE *type, int *leaves)
{
    MPIR_FF_STACK_ITEM  *stack; /* Pointer to stack array */
    struct ff_branch    *blist; /* Pointer to branch list array */
    int                  si,bi; /* Stack and Branch Index */

    MPIR_FF_LIST_ITEM **ff_list; /* Pointer to the address of the next stack item */

    size_t               position; /* Start position in the source buffer */
    int                  next_index; /* Next Index for indexed or struct type after a branch */
    int                  depth; /* depth of dtype tree */
    int                  i; /* counter */
    int                  c_size; /* contig_size used in optimization */

    /* XXX Include some error handling, esp. for malloc() (use MPIR macro) ! */

    /* Initialize */
    depth = type->depth;
    stack = (MPIR_FF_STACK_ITEM *) malloc (sizeof(MPIR_FF_STACK_ITEM) * depth);
    blist = NULL;
    si = 0;
    bi = 0;
    ff_list = &type->ff;
    next_index = 0;
    position = 0;
    *leaves = 0;

    /* This is the break-condition for inner loop in the copy routine. */
    stack[0].count = -1;
    stack[0].extent = 0;
    stack[0].size   = 0;

    while (type) {
	switch (type->dte_type) {
	case MPIR_CONTIG:
	    FF_STACK_ADD(stack, si, type->old_type->extent, type->count);
	    position += type->lb;
	    
	    type = type->old_type;
	    break;
	    
	case MPIR_VECTOR:
	    FF_STACK_ADD(stack, si, (type->stride*type->old_type->extent), type->count);
	    position += FF_STACK_POS(stack, si);
	    
	    FF_STACK_ADD(stack, si, type->old_type->extent, type->blocklen);
	    position += type->lb;
	    
	    type = type->old_type;
	    break;
	    
	case MPIR_HVECTOR:
	    FF_STACK_ADD(stack, si, type->stride, type->count);
	    FF_STACK_ADD(stack, si, type->old_type->extent, type->blocklen);
	    position += type->lb;
	    
	    type = type->old_type;
	    break;

	case MPIR_INDEXED:
	    if (!next_index) {
		for (i = 1; i <= type->count - 1; i++) {
		    FF_BRANCH_ADD(blist,type,i,position,si);
		}
	    }

	    FF_STACK_ADD(stack, si, type->old_type->extent, type->blocklens[next_index]);
	    position += /* type->lb + */ (type->indices[next_index] * type->old_type->extent);
	    
	    type = type->old_type;
	    next_index = 0;
	    break;
	    
	case MPIR_HINDEXED:
	    if (!next_index) {
		for (i = 1; i <= type->count - 1; i++) {
		    FF_BRANCH_ADD(blist,type,i,position,si);
		}
	    }
	    
	    FF_STACK_ADD(stack, si, type->old_type->extent, type->blocklens[next_index]);
	    position += /* type->lb + */ type->indices[next_index];
	    
	    type = type->old_type;
	    next_index = 0;
	    break;
	    
	case MPIR_STRUCT:
	    if (!next_index) {
		for (i = 1; i <= type->count - 1; i++) {
		    FF_BRANCH_ADD(blist, type, i, position, si);
		}
	    }
	    
	    FF_STACK_ADD(stack, si, type->old_types[next_index]->extent, type->blocklens[next_index]);
	    position += /* type->lb + */ type->indices[next_index];
	    
	    type = type->old_types[next_index];
	    next_index = 0;
	    break;

	default: 
	    break;

	} /* switch (type->dte_type) */

	/* If we have a contiguous type next, we can start copying/saving the stack. */
	if (type != NULL && type->is_contig) {
	    MPIR_FF_LIST_ITEM *ftmp,*fnext;
	    
	    position += type->lb;
	    /* adjust leave counter */
	    (*leaves)++;
	    
	    /* calculate the size */
	    c_size = type->size;
	    stack[si].size = c_size;
	    for (i = si-1 ; i >= 1; i--) {
		stack[i].size = stack[i+1].count * stack[i+1].size;
	    }
	    /* copy the stack to the ff_list (sorted, biggest first) */
	    if (!(*ff_list) || ((*ff_list)->contig_size < c_size)) {
		/* first in list */
		fnext = *ff_list;
		*ff_list = (MPIR_FF_LIST_ITEM *) malloc (sizeof(MPIR_FF_LIST_ITEM));
		ftmp = *ff_list;
	    } else {
		/* start of list */
		ftmp = *ff_list;
		while (ftmp->next && ftmp->next->contig_size > c_size) {
		    /* advance till smaller */
		    ftmp = ftmp->next;
		}
		/* insert before next item */
		fnext = ftmp->next;
		ftmp->next = (MPIR_FF_LIST_ITEM *) malloc (sizeof(MPIR_FF_LIST_ITEM));
		ftmp = ftmp->next;
	    }
	    
	    ftmp->next = fnext;
	    ftmp->contig_size = c_size;
	    ftmp->top = si;
	    ftmp->pos = position;
	    ftmp->stack = (MPIR_FF_STACK_ITEM *) malloc (sizeof(MPIR_FF_STACK_ITEM) * depth);
	    memcpy (ftmp->stack, stack, sizeof(MPIR_FF_STACK_ITEM) * depth);
	    
	    /* Basic optimization for long blocklengths and for blen = 1 ("merging") */
	    for (i = ftmp->top; i > 0; i--) {
		if (ftmp->stack[ftmp->top].size == ftmp->stack[ftmp->top].extent) {
		    ftmp->contig_size = ftmp->contig_size*stack[ftmp->top].count;
		    if (ftmp->top > 1)
		      /* We can ommit this stack entry - it's information is in the list entry. */
		      ftmp->top--;
		    else {
		      /* This is the only stack entry; we need to keep it, but update it's contents. */
		      ftmp->stack[ftmp->top].size   = ftmp->contig_size;
		      ftmp->stack[ftmp->top].extent = ftmp->contig_size;
		      ftmp->stack[ftmp->top].count  = 1;
		    }
		} 
		if (ftmp->stack[ftmp->top].count == 1 && ftmp->top > 1) {
		    ftmp->top--;
		}
	    }

	    type = NULL;
	}
	
	if (!type && blist) {
	    /* No more levels, but we continue with the branch-list for remaining branches 
	     of the datatype. Process next subtree, remove items from stack until position 
	     of the branch and remove blist item */
	    type       = blist->subtree;
	    next_index = blist->index;
	    position   = blist->position;
	    si         = blist->sp;
	    FF_BRANCH_REMOVE(blist);
	} 
    } /* while (type) */
    
    free (stack);
    while (blist) {
	FF_BRANCH_REMOVE(blist);
    }

    return 1;
    /* printf ("FF_PACK copied: %d bytes\n",*outlen); */
} 
		

/* We have no need for the macros anymore */
#undef FF_STACK_ADD
#undef FF_STACK_REMOVE
#undef FF_SLIST_ADD
#undef FF_SLIST_REMOVE
#undef FF_SLIST_CLEAR


/* free all memory used for the ff_stack_list in the datatype */
int MPIR_free_ff (struct MPIR_DATATYPE *type)
{
    MPIR_FF_LIST_ITEM *tmp;
    
    if ((!type->ff) || (!type->committed)) 
	return 1;
    
    /* This is not very effective ;) 
       ...and it assumes that the stack is build correct!!! 
       Maybe use error handling before free? */
    
    tmp = type->ff;
    while (tmp->next) {
	while (tmp->next->next) tmp = tmp->next;
	free (tmp->next->stack);
	free (tmp->next);
	tmp->next = NULL;
	tmp = type->ff;
    }
    free (type->ff->stack);
    free (type->ff);
    type->ff = NULL;
    return 1;
}

int MPIR_get_dt_depth (struct MPIR_DATATYPE *dtype_ptr)
{
    int i;
    int max = 0;
    int tmp = 0;

    if (!dtype_ptr) 
	return 0;
    if (dtype_ptr->basic) 
	return 1;

    switch (dtype_ptr->dte_type) {
    case MPIR_STRUCT:
	for (i = 0; i < dtype_ptr->count; i++) {
	    tmp = MPIR_get_dt_depth(dtype_ptr->old_types[i]);
	    if (tmp > max) 
		max = tmp;
	}
	break;
    case MPIR_VECTOR:
    case MPIR_HVECTOR:
	/* we need one extra depth for vector type */
	max = 1 + MPIR_get_dt_depth (dtype_ptr->old_type);
	break;
    default:
	max = MPIR_get_dt_depth (dtype_ptr->old_type);
	break;
    } /* switch (dtype_ptr->dte_type) */
    
    return (max + 1);
}
