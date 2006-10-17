/*$Id: stack.h,v 1.8 2002/01/24 14:37:42 joachim Exp $*/
#ifndef _MPID_STACK_H
#define _MPID_STACK_H

#ifndef WIN32
#include <pthread.h>
#endif
#include "utildefs.h"

typedef struct _MPID_stack_t {
    /* this is the attic for listentries to be re-used */
    struct _MPID_stack_entry_t *attic;
    /* this is the real stack */
    struct _MPID_stack_entry_t *stack;

    int data_is_malloced;
    int nbr_entries;
    MPID_LOCK_T *lock; 
} MPID_stack;
typedef MPID_stack* MPID_stack_t;

typedef struct _MPID_stack_entry_t {
    void *content;
    struct _MPID_stack_entry_t *next;
} MPID_stack_entry_t;


MPID_stack_t MPID_stack_init ( int flag );
void MPID_stack_push ( MPID_stack_t stack, void *data );
void *MPID_stack_pop ( MPID_stack_t stack );
void MPID_stack_destroy ( MPID_stack_t stack );

#endif

