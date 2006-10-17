/* $Id: intqueue.c,v 1.5 2004/03/26 09:41:42 boris Exp $ 
 * (c) 2002 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de
 */

#include <stdlib.h>
#include "intqueue.h"

/*
 * queue handling routines
 */

int Qinit (IntQueue *Q, int n, int full)
{
  int i;

#ifdef ROUTER_THREADS
  pthread_mutex_init (&(Q->q_lock), NULL);
#endif  
  Q->entries = (int *)calloc (n, sizeof(int));
  if (Q->entries == NULL)
    return (0);
  
  Q->max_entries = n;
  Q->current = 0;
  if (full) {
    Q->n_entries = n;
    for (i = 0; i < n; i++)
      Q->entries[i] = i;
  } else {
    Q->n_entries = 0;
  }
  return (n);
}

void Qdestroy (IntQueue *Q)
{
  free (Q->entries);
#ifdef ROUTER_THREADS
  pthread_mutex_destroy(&(Q->q_lock));
#endif

  return;
}

int Qfirst (IntQueue *Q)
{
  Q->current = 0;
  if (Q->n_entries > Q->current)
    return (Q->entries[Q->current]);
  else 
    return (-1);
}

int Qnext (IntQueue *Q)
{
  if (Q->n_entries > ++(Q->current))
    return (Q->entries[Q->current]);
  else 
    return (-1);
}

int Qput (IntQueue *Q, int value)
{
  if (++(Q->n_entries) > Q->max_entries) /* XXXX n_entries is incremented in any case -> is this correct? */
    return (-1);
  Q->entries[Q->n_entries - 1] = value;
  return (Q->n_entries);
}

int Qremove (IntQueue *Q, int value)
{
  int i, j;
  int found = 0;
  
  for (i = 0; i < Q->n_entries; i++) {
    if (value != Q->entries[i])
      continue;
    /* found the entry to be removed */
    for (j = i; j < Q->n_entries; j++)
      Q->entries[j] = Q->entries[j+1];
    Q->n_entries--;
    found = 1;
    break;
  }
  return (found);
}

int Qget (IntQueue *Q)
{
    if (Q->n_entries == 0)
	return (-1);
    else {
	Q->n_entries--;
	return (Q->entries[Q->n_entries]);
    }
}

int Qfull (IntQueue *Q)
{
    return (Q->n_entries >= Q->max_entries);
}

int Qempty (IntQueue *Q)
{
    return (Q->n_entries == 0);
}
