/* $Id: dataqueue.h,v 1.4 2002/01/24 14:37:42 joachim Exp $ */
#ifndef _MPID_DATAQUEUE_H
#define _MPID_DATAQUEUE_H

#ifndef WIN32
#include <pthread.h>
#endif

#include "utildefs.h"

 typedef struct _MPID_dataqueue_entry {
    void *content;
    struct _MPID_dataqueue_entry *next;
    struct _MPID_dataqueue_entry *last;
} MPID_dataqueue_entry;

typedef struct _MPID_dataqueue {
    /* this is the attic for listentries to be re-used */
    struct _MPID_dataqueue_entry *attic;
    /* this is the real stack */
    struct _MPID_dataqueue_entry *head;
    struct _MPID_dataqueue_entry *tail;
    
    int data_is_malloced;
    int nbr_entries;
    MPID_LOCK_T *lock; 
} MPID_dataqueue;
typedef MPID_dataqueue* MPID_dataqueue_t;


MPID_dataqueue_t MPID_dataqueue_init (int flags);

void MPID_dataqueue_enqueue (MPID_dataqueue_t queue, void *data);

/* removes first element in queue and returns stored address, NULL if queue is empty */
void *MPID_dataqueue_dequeue (MPID_dataqueue_t queue);

/* removes element with stored address data from queue and returns stored address, NULL
   if element could not be found; if there are more elements with the same data in queue,
   the least inserted is removed */
void *MPID_dataqueue_remove(MPID_dataqueue_t queue, void *data);

void MPID_dataqueue_destroy (MPID_dataqueue_t queue);

#endif
