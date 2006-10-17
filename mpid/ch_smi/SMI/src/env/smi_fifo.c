/*$Id$*/

#include "smidebug.h"
#include "smi_fifo.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

int _smi_FIFO_init (_smi_FIFO_t *fifo, int flags)
{    
    ALLOCATE( *fifo, _smi_FIFO_t, sizeof(_smi_FIFO));

    (*fifo)->attic = NULL;   
    (*fifo)->tail  = NULL;
    (*fifo)->head  = NULL;

    if (flags & SMI_FIFO_THREADSAFE) {
	ALLOCATE ((*fifo)->lock, pthread_mutex_t *, sizeof (pthread_mutex_t));
	SMI_INIT_LOCK((*fifo)->lock);
    } else {
	(*fifo)->lock = NULL;
    }

    (*fifo)->data_is_malloced = (flags|SMI_FIFO_MALLOCED_DATA);

    return SMI_SUCCESS;
}

int _smi_FIFO_push ( _smi_FIFO_t fifo, void *data )
{
    _smi_FIFO_entry_t *new;
    
    if (fifo->lock != NULL) {
      SMI_LOCK(fifo->lock);
    }
    /* is there a re-usable entry in the attic ? */
    if ((new = fifo->attic) != NULL) {
	fifo->attic = new->prev;
    } else {
	ALLOCATE( new, _smi_FIFO_entry_t *, sizeof(_smi_FIFO_entry_t));
    }

    new->content = data;
    new->prev    = NULL;
    /* insert new entry */
    if (fifo->head != NULL)
	fifo->head->prev = new;
    fifo->head = new;
    if (fifo->tail == NULL)
	fifo->tail = new;

    if (fifo->lock != NULL) {
	SMI_UNLOCK(fifo->lock);
    }

    return SMI_SUCCESS;
}

void *_smi_FIFO_pop ( _smi_FIFO_t fifo )
{
    void *data;
    _smi_FIFO_entry_t *last;

    if (fifo->lock != NULL) {
	SMI_LOCK(fifo->lock);
    }

    last = fifo->tail;
    if (last != NULL) {
	/* get the data */
	data = last->content;
	last->content = NULL;

	/* remove entry & store in the attic */
	fifo->tail = last->prev;
	last->prev = fifo->attic;
	fifo->attic = last;
    } else
	data = NULL;

    if (fifo->lock != NULL) {
	SMI_UNLOCK(fifo->lock);
    }

    return data;
}

void _smi_FIFO_destroy ( _smi_FIFO_t fifo )
{
    _smi_FIFO_entry_t *entry, *prev_entry;
    
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
	SMI_DESTROY_LOCK (fifo->lock);
	free(fifo->lock);
    }
    free(fifo);

    return;
}
