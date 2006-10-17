
/*$Id: stack.c,v 1.7 2002/01/24 14:37:42 joachim Exp $*/

#include "stack.h"


MPID_stack_t MPID_stack_init (int flags)
{    
    MPID_stack_t stack;
    MPID_ALLOCATE(stack, MPID_stack_t , sizeof(MPID_stack));

    stack->attic = NULL;   
    stack->stack = NULL;

    if (flags & MPID_UTIL_THREADSAFE) {
	MPID_ALLOCATE (stack->lock, MPID_LOCK_T *, sizeof (MPID_LOCK_T));
	MPID_INIT_LOCK (stack->lock);
    } else {
	stack->lock = NULL;
    }
    stack->nbr_entries = 0;
    stack->data_is_malloced = (flags|MPID_UTIL_MALLOCED_DATA);

    return stack;
}

void MPID_stack_push ( MPID_stack_t stack, void *data )
{
    MPID_stack_entry_t *new;
    
    if (stack->lock != NULL)
	MPID_LOCK(stack->lock);

    /* is there a re-usable entry in the attic ? */
    if ((new = stack->attic) != NULL) {
	stack->attic = new->next;
    } else {
	MPID_ALLOCATE( new, MPID_stack_entry_t *, sizeof(MPID_stack_entry_t));
    }

    new->content = data;
    new->next    = stack->stack;
    stack->stack  = new;
    stack->nbr_entries++;

    if (stack->lock != NULL)
	MPID_UNLOCK(stack->lock);

    return;
}

void *MPID_stack_pop ( MPID_stack_t stack )
{
    void *data;
    MPID_stack_entry_t *first;

    if (stack->lock != NULL)
	MPID_LOCK(stack->lock);

    first = stack->stack;
    if( first != NULL) {
	/* get the data */
	data = first->content;
	/* close the gap */
	stack->stack = first->next;
	/* store in the attic */
	first->next = stack->attic;
	stack->attic = first;

	stack->nbr_entries--;
    }
    else
	data = NULL;

    if (stack->lock != NULL)
	MPID_UNLOCK(stack->lock);

    return data;
}

void MPID_stack_destroy ( MPID_stack_t stack )
{
    MPID_stack_entry_t *entry, *next_entry;
    
    /* delete stack */
    entry = stack->stack;
    while (entry != NULL) {
	next_entry = entry->next;
	if (stack->data_is_malloced)
	    free(entry->content);
	free(entry);
	entry = next_entry;
    }
    /* delete attic */
    entry = stack->attic;
    while (entry != NULL) {
	next_entry = entry->next;
	free(entry);
	entry = next_entry;
    }

    /* delete stack */
    if (stack->lock) {
	MPID_DESTROY_LOCK(stack->lock);
	free(stack->lock);
    }
    free(stack);

    return;
}
