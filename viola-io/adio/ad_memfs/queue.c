/*==============================================================================
  
   Purpose          : int queue implementation
   Author           : Rudolf Berrendorf
                      Department of Computer Science
                      University of Applied Sciences Bonn-Rhein-Sieg
	              53754 Sankt Augustin, Germany
                      rudolf.berrendorf@fh-brs.de
   Version          : 1.0
  
==============================================================================*/

/* system includes */
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

/* common data structures */
#include "memory.h"

/* me */
#include "queue.h"


/*============================================================================*/
/* queue invariant */

#define queueInvariant(q) \
  assert (q != NULL); \
  assert (q->read_pos >= 0); \
  assert (q->write_pos >= 0); \
  assert(q->read_pos <= q->allocated); \
  assert(q->write_pos <= q->allocated); \
  assert (q->allocated >= 0); \
  assert (q->read_pos <= q->write_pos+1)


/*============================================================================*/
/* statistics */

static int64_t enqueued;	       /* enqueue operations */
static int64_t dequeued;	       /* dequeue operations */
static int64_t queues;		       /* active queues */
static int64_t current_size;	       /* actual memory size for queues */
static int64_t max_size;	       /* maximum memory size for queues */

static int alloc_inc = 1;	       /* increment for memory allocation */


/*============================================================================*/
/* update statistics */

static void
update_memstat (int size)
{
  current_size += size;
  if (current_size > max_size)
    max_size = current_size;
}

/*============================================================================*/
/* create an empty queue */

queue_t
queueCreate (void)
{
  queue_t q;

#if defined(QUEUE_DEBUG)
  printf("[queue] create\n");
#endif

  /* allocate memory for queue */
  q = memoryAlloc (sizeof (*q));
  if (q == NULL)
    return q;
  update_memstat (memoryGetSize (q));

  q->read_pos = 0;
  q->write_pos = 0;
  q->allocated = 0;
  q->items = NULL;

  /* check invariant */
  queueInvariant(q);
  assert (queueIsEmpty (q));

  /* statistics */
  ++queues;

  return q;
}

/*============================================================================*/
/* delete a queue */

void
queueDelete (queue_t q)
{
  /* check invariant */
  queueInvariant(q);

#if defined(QUEUE_DEBUG)
  printf("[queue] delete\n");
#endif

  /* free items */
  if (q->items != NULL)
    {
      update_memstat (-memoryGetSize (q->items));
      memoryFree (q->items);
    }

  /* free queue */
  update_memstat (-memoryGetSize (q));
  memoryFree (q);

  /* statistics */
  --queues;
}

/*============================================================================*/
/* test for emptyness */

int
queueIsEmpty (const queue_t q)
{
  /* check invariant */
  queueInvariant(q);

#if defined(QUEUE_DEBUG)
  printf ("[queue] isEmpty: %s\n",
	  (q->read_pos >= q->write_pos) ? "true" : "false");
#endif

  return q->read_pos >= q->write_pos;
}

/*============================================================================*/
/* enqueue an item */

queue_t
queueEnqueue (queue_t q, values_t item)
{
  int i, n;

  /* check invariant */
  queueInvariant(q);

#if defined(QUEUE_DEBUG)
  printf ("[queue] enqueue item %d start: ", item.fh);
  queuePrint (q);
#endif

  if (q->write_pos == q->allocated)
    {
      /* not enough place */
      if (q->read_pos > 0)
	{
	  /* compact queue */
	  n = q->write_pos - q->read_pos;
	  assert (n < q->allocated);
	  for (i = 0; i < n; i++)
	    {
	      assert (q->read_pos + i >= 0);
	      assert (q->read_pos + i < q->allocated);
	      q->items[i] = q->items[q->read_pos + i];
	    }
	  q->read_pos = 0;
	  q->write_pos = n;
	}
      else
	{
	  /* enlarge item buffer */
	  q->allocated += alloc_inc;
	  update_memstat (-memoryGetSize (q->items));
	  q->items = memoryRealloc (q->items,
				    q->allocated * sizeof (*(q->items)));
	  update_memstat (memoryGetSize (q->items));
	}
    }

  assert (q->write_pos >= 0);
  assert (q->write_pos <= q->allocated);
  q->items[q->write_pos] = item;
  ++q->write_pos;

  /* statistics */
  ++enqueued;

  /* check invariant */
  queueInvariant(q);
  assert (!queueIsEmpty (q));

#if defined(QUEUE_DEBUG)
  printf ("[queue] enqueue %d end  : ", item);
  queuePrint (q);
#endif

  return q;
}

/*============================================================================*/
/* dequeue first item */

values_t
queueDequeue (queue_t q)
{
  values_t retvalue;

  /* check invariant */
  queueInvariant(q);

#if defined(QUEUE_DEBUG)
  printf ("[queue] dequeue %d start: ", q->items[q->read_pos]);
  queuePrint (q);
#endif

  assert (q->read_pos >= 0);
  assert (q->read_pos < q->allocated);
  assert (q->read_pos <= q->write_pos);

  /* statistics */
  ++dequeued;

#if defined(QUEUE_DEBUG)
  printf ("[queue] dequeue %d end  : ", q->items[q->read_pos]);
  queuePrint (q);
#endif

  retvalue = q->items[q->read_pos++];

  /* check invariant */
  queueInvariant(q);

  return retvalue;
}

/*============================================================================*/
/* print queue */

void
queuePrint (const queue_t q)
{
  int i,j;
  
  /* check invariant */
  queueInvariant(q);

  fprintf(stderr,"  read_pos=%d, write_pos=%d, allocated=%d \n",
	  q->read_pos, q->write_pos, q->allocated);
  for (i = q->read_pos; i < q->write_pos; i++){
    fprintf(stderr,"  fh %d, size %d, server %d ", q->items[i].fh, q->items[i].size, q->items[i].server);
    for (j = 0; j < q->items[i].size; j++){
      fprintf(stderr, "[%d]", q->items[i].blocks[j] );
    }
  }
  printf ("\n");
}

/*============================================================================*/
/* print statistics on queues */

void
queueStatistics (void)
{
  fprintf (stderr,"[queue] active queues=%lld, enqueue ops=%lld, dequeue ops=%lld\n",
	  queues, enqueued, dequeued);
  fprintf (stderr,"[queue] current size=%lld, max size=%lld\n", current_size, max_size);
}

/*============================================================================*/
/* test program */

#if defined(QUEUE_TEST)

#define N 3
#define N2 100

int
main (int argc, char **argv)
{
  queue_t q;
  values_t v,v1,v2; 
  int i,j,sum1, sum2;

  // Test variables
  int fh=0;
  int size = 5;
  int server = 1;
  int *blocks = malloc(size * sizeof(int));
  int *blocks2 = malloc(size * sizeof(int));


  fprintf(stderr,"Print Variables: %d %d %d \n", fh,size,server);
  
  for(j=0;j<size;j++){
    blocks[j] = j;
    blocks2[j]= j+10;
    fprintf(stderr,"[%d]",blocks[j]);
  }
  fprintf(stderr," finished \n");

  v.fh = fh;
  v.size = size;
  v.server = server;
  v.blocks = blocks;

  v2.fh = fh+10;
  v2.size = size;
  v2.server = server+10;
  v2.blocks = blocks2;


  fprintf(stderr,"Print v: %d %d %d \n", v.fh,v.size,v.server);
  for(j=0;j<size;j++){
    fprintf(stderr,"[%d]",v.blocks[j]);
  }
  fprintf(stderr," finished \n");


  q = queueCreate ();
  // empty queue
  fprintf(stderr, "Empty Queue \n");
  queuePrint(q);

  // one element in queue
  fprintf(stderr, "One element in queue \n");
  queueEnqueue(q,v);
  queuePrint(q);

  // two elements in queue
  fprintf(stderr, "Two elements in queue \n");
  queueEnqueue(q,v2);
  queuePrint(q);
  
  // Get first element out of queue
  v1 = queueDequeue(q);
  fprintf(stderr,"First element in queue fh %d, size %d, server %d \n", v1.fh, v1.size, v1.server );
  for(j=0;j<size;j++){
    fprintf(stderr,"[%d]",v1.blocks[j]);
  }
  fprintf(stderr," finished \n");

  //isEmpty == false
  
  fprintf(stderr," isEmpty %d \n", queueIsEmpty(q) ); 

  // Get first element out of queue
  v1 = queueDequeue(q);
  fprintf(stderr,"First element in queue fh %d, size %d, server %d \n", v1.fh, v1.size, v1.server );
  for(j=0;j<size;j++){
    fprintf(stderr,"[%d]",v1.blocks[j]);
  }
  fprintf(stderr," finished \n");

  // Print last element
  queuePrint(q);

  //isEmpty == true
  fprintf(stderr," isEmpty %d \n", queueIsEmpty(q) );

  // Statistics
  queueStatistics();

}

#endif

/*============================================================================*
 *                             that's all folks                               *
 *============================================================================*/
