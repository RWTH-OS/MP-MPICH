/*==============================================================================
  
   Purpose          : int queue implementation
   Author           : Rudolf Berrendorf
                      Department of Computer Science
                      University of Applied Sciences Bonn-Rhein-Sieg
	              53754 Sankt Augustin, Germany
                      rudolf.berrendorf@fh-brs.de
   Version          : 1.0
  
  
==============================================================================*/

#if !defined(QUEUE_H_INCLUDED)
#define QUEUE_H_INCLUDED

/*============================================================================*/
/* types */

typedef struct
{
  int fh;		// filehandle
  int *blocks;		// blocks to be locked
  int size;		// number of blocks
  int server;		// MPI_SOURCE
} values_t;


typedef struct
{
  int read_pos;			       /* read position */
  int write_pos;		       /* write position */
  int allocated;		       /* number of allocated items */
//  int *items;			       /* items */
  values_t *items;
} *queue_t;


/*============================================================================*/
/* exported functions */

/* create an empty queue */
extern queue_t queueCreate (void);

/* delete a queue */
extern void queueDelete (queue_t q);

/* enqueue an item */
extern queue_t queueEnqueue (queue_t q, values_t item);

/* test for emptyness */
extern int queueIsEmpty (const queue_t q);

/* dequeue first item */
extern values_t queueDequeue (queue_t q);

/* print queue */
extern void queuePrint (const queue_t q);

/* print statistics on queues */
extern void queueStatistics (void);

#endif

/*============================================================================*
 *                             that's all folks                               *
 *============================================================================*/
