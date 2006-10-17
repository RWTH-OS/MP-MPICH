/* $Id: intqueue.h,v 1.5 2003/02/17 11:42:05 stef Exp $ 
 * (c) 2002 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de
 */

#ifndef INTQUEUE_H
#define INTQUEUE_H

/*
 * a simple queue to manage integer values from 0 ... max-1
 * 
 * actually, it's a stack, you can put values at the end with QPut and remove it from there with Qget
 * you can step through it with Qfirst and Qnext and remove values with Qremove
 * for use with multiple threads a lock is provided but has to be used from the outside
 * beware of multiple entries with the same value in the integer queue
 */

#include <stdio.h>
#include <sys/types.h>
#ifdef ROUTER_THREADS
#include "pthread.h"
#endif
typedef struct _IntQueue {
#ifdef ROUTER_THREADS
  pthread_mutex_t q_lock;  /* provided to lock access to the integer queue */
#endif    
  int n_entries;   /* current number of entries in the integer queue */
  int max_entries; /* maximum number of entries in the integer queue */
  int current;     /* pointer used by Qfirst and Qnext */
  int *entries;    /* array holding the entries */
} IntQueue;


/*
 * initialization and destruction
 */

/* initializes the integer queue and the lock
   Parameters: Q    - Pointer to integer queue object to be initialized
               n    - maximum number of entries
               full - if 0, integer queue is empty after initialization
                      otherwise, it contains entries from 0 to n-1 and is full after initialization
   Return Values: 0 on failure, n in case of success */
int Qinit (IntQueue *Q, int n, int full);

/* destroys the queue and the lock */
void Qdestroy (IntQueue *Q);

/*
 * stepping through the values contained in the queue
 */

/* sets pointer to the first entry in integer queue
   Return Values: -1 if integer queue is empty, first entry otherwise */
int Qfirst (IntQueue *Q);

/* increments pointer in integer queue
   Return values: -1 if pointer does not point to a valid entry, entry pointed to otherwise */
int Qnext (IntQueue *Q);


/*
 * insertion, removal and aquisition of values
 */

/* inserts value at end of integer queue
   Return values: -1 if queue was already full, new number of entries in queue otherwise */
int Qput (IntQueue *Q, int value);

/* removes entry value (first one if there are multiples)
   Return: 0 if not found, 1 if found and removed */
int Qremove (IntQueue *Q, int value);

/* get last entry in integer queue
   return values: -1 if integer queue is empty, last value otherwise */
int Qget (IntQueue *Q);

/*
 * status indication
 */

int Qfull (IntQueue *Q);
int Qempty (IntQueue *Q);

#endif
