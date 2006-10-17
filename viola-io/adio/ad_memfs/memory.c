/*==============================================================================
  
   Purpose          : memory allocation
   Author           : Rudolf Berrendorf
                      Department of Computer Science
                      University of Applied Sciences Bonn-Rhein-Sieg
	              53754 Sankt Augustin, Germany
                      rudolf.berrendorf@fh-brs.de
   Version          : 1.0
  
==============================================================================*/

/* system includes */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* me */
#include "memory.h"


/*============================================================================*/
/* invariant */

#define memoryInvariant() \
  assert (current_size >= 0); \
  assert (max_size >= 0); \
  assert (max_size >= current_size)

/*============================================================================*/
/* statistics */

static int64_t current_size;
static int64_t max_size;

static int memory_inc = sizeof (double);

/*============================================================================*/
/* allocates n bytes of memory */

void *
memoryAlloc (const int n)
{
  int *ptr;
  void *ret_ptr;

  /* check invariant */
  memoryInvariant ();
  assert (n > 0);

  /* allocate memory */
  ptr = malloc (n + memory_inc);
  if (ptr == NULL)
    return ptr;

  /* clear memory */
  memset (ptr, 0, n + memory_inc);

  /* statistics */
  current_size += n;
  if (current_size > max_size)
    max_size = current_size;

  /* store size of segment */
  *ptr = n;

  /* return memory excluding size information */
  /* we take memory_inc to handle word alignment */
  ret_ptr = ((char *) ptr) + memory_inc;
#if defined(MEMORY_DEBUG)
  printf ("memory alloc  : %p, size=%d\n", ret_ptr, n);
#endif

  return ret_ptr;
}

/*============================================================================*/
/* re-allocates n bytes of memory */

void *
memoryRealloc (void *ptr, const int n)
{
  int *old_ptr;
  void *ret_ptr;
  int old_size;

  /* check invariant */
  memoryInvariant ();
  assert (n > 0);

  /* allocate new memory */
  ret_ptr = memoryAlloc (n);
  if (ret_ptr == NULL)
    return ret_ptr;

  /* determine size of memory to be copied */
  if (ptr != NULL)
    {
      old_ptr = (int *) (((char *) ptr) - memory_inc);
      old_size = *old_ptr;
      if (n < old_size)
	old_size = n;
      /* copy memory */
      memcpy (ret_ptr, ptr, old_size);

      /* free old memory */
      memoryFree (ptr);
    }

#if defined(MEMORY_DEBUG)
  printf ("memory realloc: old=%p, bew=%p, size=%d\n", ptr, ret_ptr, n);
#endif

  /* return new memory */
  return ret_ptr;
}

/*============================================================================*/
/* frees memory */

void
memoryFree (void *ptr)
{
  int *iptr;
  int n;

  /* check invariant */
  memoryInvariant ();
  assert (ptr != NULL);

  /* get whole segment including size information */
  iptr = (int *) (((char *) ptr) - memory_inc);

  /* do statistics */
  n = *iptr;
  current_size -= n;

  /* free memory */
  free (iptr);

#if defined(MEMORY_DEBUG)
  printf ("memory free  : %p, size=%d\n", ptr, n);
#endif
}

/*============================================================================*/
/* get size of memory segment */

int
memoryGetSize (const void *ptr)
{
  if (ptr == NULL)
    return 0;
  else
    /* get ptr to whole segment including size information */
    return *(int *) (((char *) ptr) - memory_inc);
}

/*============================================================================*/
/* prints memory statistics */

void
memoryStatistics (void)
{
  /* check invariant */
  memoryInvariant ();

  printf ("[memory] current size = %lld, maximum size = %lld\n",
	  current_size, max_size);
}

/*============================================================================*/
/* memory test */

#if defined(MEMORY_TEST)

#define N 100

int
main (int argc, char **argv)
{
  int *ptr[N];
  int i, sum1, sum2;

  sum1 = 0;
  for (i = 0; i < N; i++)
    {
      ptr[i] = memoryAlloc ((i + 1) * sizeof (int));
      *(ptr[i]) = i;
      sum1 += i;
    }

  sum2 = 0;
  for (i = 0; i < N; i++)
    {
      ptr[i] = memoryRealloc (ptr[i], 2 * (i + 1) * sizeof (int));
      sum2 += *(ptr[i]);
    }

  assert (sum1 == sum2);

  for (i = 1; i < N; i++)
    memoryFree (ptr[i]);

  return 0;
}

#endif

/*============================================================================*
 *                             that's all folks                               *
 *============================================================================*/
