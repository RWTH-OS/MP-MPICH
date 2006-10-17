/*$Id: smi_fifo.h,v 1.1 2004/03/19 22:14:15 joachim Exp $*/
#ifndef _SMI_FIFO_H
#define _SMI_FIFO_H

#include <pthread.h>

#include "env/general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _smi_FIFO_ {
    /* this is the attic for listentries to be re-used */
    struct _smi_FIFO_entry_t *attic;
    /* this is the real FIFO */
    struct _smi_FIFO_entry_t *head;   /* insert here */
    struct _smi_FIFO_entry_t *tail;   /* remove from here */

    int data_is_malloced;
    pthread_mutex_t *lock; 
} _smi_FIFO;
typedef _smi_FIFO* _smi_FIFO_t;

typedef struct _smi_FIFO_entry_t {
    void *content;
    struct _smi_FIFO_entry_t *prev;
} _smi_FIFO_entry_t;

#define SMI_FIFO_THREADSAFE    1
#define SMI_FIFO_MALLOCED_DATA 2

int _smi_FIFO_init ( _smi_FIFO_t *fifo, int flag );
int _smi_FIFO_push ( _smi_FIFO_t fifo, void *data );
void *_smi_FIFO_pop ( _smi_FIFO_t fifo );
void _smi_FIFO_destroy ( _smi_FIFO_t fifo );

#ifdef __cplusplus
}
#endif

#endif
