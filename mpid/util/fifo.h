/*$Id: fifo.h,v 1.6 2002/01/24 14:37:42 joachim Exp $*/
#ifndef _MPID_FIFO_H
#define _MPID_FIFO_H

#ifndef WIN32
#include <pthread.h>
#endif

#include "utildefs.h"

typedef struct _MPID_FIFO_t {
    /* this is the attic for listentries to be re-used */
    struct _MPID_FIFO_entry_t *attic;
    /* this is the real FIFO */
    struct _MPID_FIFO_entry_t *head;   /* insert here */
    struct _MPID_FIFO_entry_t *tail;   /* remove from here */

    int data_is_malloced;
    int nbr_entries;
    MPID_LOCK_T *lock; 
} MPID_FIFO;
typedef MPID_FIFO* MPID_FIFO_t;

typedef struct _MPID_FIFO_entry_t {
    void *content;
    struct _MPID_FIFO_entry_t *prev;
} MPID_FIFO_entry_t;


MPID_FIFO_t MPID_FIFO_init (int flags);
void MPID_FIFO_push ( MPID_FIFO_t fifo, void *data );
void *MPID_FIFO_pop ( MPID_FIFO_t fifo );
void MPID_FIFO_destroy ( MPID_FIFO_t fifo );

#endif

