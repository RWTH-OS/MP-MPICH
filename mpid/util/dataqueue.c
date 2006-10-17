/* $Id: dataqueue.c,v 1.3 2002/01/24 14:37:42 joachim Exp $ */

#include "dataqueue.h"

MPID_dataqueue_t MPID_dataqueue_init (int flags)
{
    MPID_dataqueue_t queue;
    MPID_ALLOCATE( queue, MPID_dataqueue_t , sizeof(MPID_dataqueue));

    queue->attic = NULL;
    MPID_ALLOCATE( queue->head, MPID_dataqueue_entry *, sizeof(MPID_dataqueue_entry));
    MPID_ALLOCATE( queue->tail, MPID_dataqueue_entry *, sizeof(MPID_dataqueue_entry));
    queue->head->last = queue->head;
    queue->head->next = queue->tail;
    queue->tail->last = queue->head;
    queue->tail->next = queue->tail;
    
    if (flags & MPID_UTIL_THREADSAFE) {
	MPID_ALLOCATE (queue->lock, MPID_LOCK_T *, sizeof(MPID_LOCK_T));
	MPID_INIT_LOCK(queue->lock);
    }
    else {
	queue->lock = NULL;
    }
    queue->data_is_malloced = (flags|MPID_UTIL_MALLOCED_DATA);
    queue->nbr_entries = 0;

    return queue;
}

void MPID_dataqueue_enqueue ( queue, data )
MPID_dataqueue_t queue;
void *data;
{
    MPID_dataqueue_entry *new;

    if (queue->lock != NULL)
	MPID_LOCK(queue->lock);

    /* is there a re-usable entry in the attic ? */
    if ((new = queue->attic) != NULL) {
	queue->attic = new->next;
    }
    else {
	MPID_ALLOCATE( new, MPID_dataqueue_entry *, sizeof(MPID_dataqueue_entry));
    }
    /* insert new after head element in queue */
    new->content = data;
    new->last = queue->head;
    new->next = (queue->head)->next;
    ((queue->head)->next)->last = new;
    (queue->head)->next = new;

    if (queue->lock != NULL)
	MPID_UNLOCK(queue->lock);
}

void *MPID_dataqueue_dequeue ( queue )
MPID_dataqueue_t queue;
{
    void *data = NULL;
    MPID_dataqueue_entry *first = (queue->tail)->last;

    /* if queue isn`t empty */
    if( first != queue->head ) {

	if (queue->lock != NULL)
	    MPID_LOCK(queue->lock);
	
	/* get data to be returned */
	data = first->content;
	/* remove element from queue */
	(first->last)->next = first->next;
	(queue->tail)->last = first->last;
	/* store element in attic */
	first->next = queue->attic;
	queue->attic = first;

	if (queue->lock != NULL)
	    MPID_UNLOCK(queue->lock);
    }

    return data;
}

void *MPID_dataqueue_remove( queue, data )
MPID_dataqueue_t queue;
void *data;
{
    void *found_data = NULL;
    MPID_dataqueue_entry *entry = (queue->head)->next;

    if( queue->lock != NULL )
	MPID_LOCK(queue->lock);

    /* look for element */
    while( (entry != queue->tail) && (entry->content != data) )
	entry = entry->next;

    /* if element was found, remove it from queue and store memory in attic */
    if( entry != queue->tail ) {
	found_data = entry->content;
	(entry->last)->next = entry->next;
	(entry->next)->last = entry->last;
	entry->next = queue->attic;
	queue->attic = entry;
    }

    if( queue->lock != NULL )
	MPID_UNLOCK(queue->lock);

    return found_data;
}

void MPID_dataqueue_destroy( queue )
MPID_dataqueue_t queue;
{
    MPID_dataqueue_entry *entry, *next_entry;

    /* delete queue */
    entry = queue->head->next;
    while( entry != queue->tail ) {
	next_entry = entry->next;
	if (queue->data_is_malloced)
	    free(entry->content);
	free(entry);
	entry = next_entry;
    }
    /* delete attic */
    entry = queue->attic;
    while( entry != NULL ) {
	next_entry = entry->next;
	free(entry);
	entry = next_entry;
    }

    /* delete queue */
    if (queue->lock) {
	MPID_DESTROY_LOCK(queue->lock);
	free (queue->lock);
    }
    free(queue->head);
    free(queue->tail);
    free(queue);

    return;
}
