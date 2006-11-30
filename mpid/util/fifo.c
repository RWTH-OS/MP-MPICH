/*$Id$*/

#include "fifo.h"


MPID_FIFO_t MPID_FIFO_init (int flags)
{    
    MPID_FIFO_t fifo;
    MPID_ALLOCATE(fifo, MPID_FIFO_t, sizeof(MPID_FIFO));

    fifo->attic = NULL;   
    fifo->tail  = NULL;
    fifo->head  = NULL;

    if (flags & MPID_UTIL_THREADSAFE) {
	MPID_ALLOCATE (fifo->lock, MPID_LOCK_T *, sizeof (MPID_LOCK_T));
	MPID_INIT_LOCK (fifo->lock);
    } else {
	fifo->lock = NULL;
    }
    fifo->nbr_entries = 0;
    fifo->data_is_malloced = (flags|MPID_UTIL_MALLOCED_DATA);

    return fifo;
}

void MPID_FIFO_push ( MPID_FIFO_t fifo, void *data )
{
    MPID_FIFO_entry_t *new;
    
    if (fifo->lock != NULL)
	MPID_LOCK(fifo->lock);

    /* is there a re-usable entry in the attic ? */
    if ((new = fifo->attic) != NULL) {
	fifo->attic = new->prev;
    } else {
	MPID_ALLOCATE( new, MPID_FIFO_entry_t *, sizeof(MPID_FIFO_entry_t));
    }

    new->content = data;
    new->prev    = NULL;
    /* insert new entry */
    if (fifo->head != NULL)
	fifo->head->prev = new;
    fifo->head = new;
    if (fifo->tail == NULL)
	fifo->tail = new;
    fifo->nbr_entries++;

    if (fifo->lock != NULL)
	MPID_UNLOCK(fifo->lock);

    return;
}

void *MPID_FIFO_pop ( MPID_FIFO_t fifo )
{
    void *data;
    MPID_FIFO_entry_t *last;

    if (fifo->lock != NULL)
	MPID_LOCK(fifo->lock);
    
    last = fifo->tail;
    if (last != NULL) {
	/* get the data */
	data = last->content;
	last->content = NULL;
	
	/* remove entry & store in the attic */
	if (fifo->tail == fifo->head)
	    /* remove only entry in FIFO which is then empty. */
	    fifo->head = NULL;
	fifo->tail = last->prev;
	last->prev = fifo->attic;
	fifo->attic = last;

	fifo->nbr_entries--;
    } else
	data = NULL;

    if (fifo->lock != NULL)
	MPID_UNLOCK(fifo->lock);

    return data;
}

void MPID_FIFO_destroy ( MPID_FIFO_t fifo )
{
    MPID_FIFO_entry_t *entry, *prev_entry;
    
    /* delete fifo */
    entry = fifo->tail;
    while (entry != NULL) {
	prev_entry = entry->prev;
	if (fifo->data_is_malloced)
	    free(entry->content);
	free(entry);
	entry = prev_entry;
    }
    /* delete attic */
    entry = fifo->attic;
    while (entry != NULL) {
	prev_entry = entry->prev;
	free(entry);
	entry = prev_entry;
    }

    /* delete fifo */
    if (fifo->lock) {
	MPID_DESTROY_LOCK (fifo->lock);
	free(fifo->lock);
    }
    free(fifo);

    return;
}
