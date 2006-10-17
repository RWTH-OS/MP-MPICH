/*==============================================================================
  
   Purpose          : memory allocation
   Author           : Rudolf Berrendorf
                      Department of Computer Science
                      University of Applied Sciences Bonn-Rhein-Sieg
	              53754 Sankt Augustin, Germany
                      rudolf.berrendorf@fh-brs.de
   Version          : 1.0
  
==============================================================================*/

#if !defined(MEMORY_H_INCLUDED)
#define MEMORY_H_INCLUDED

/* allocates n bytes of memory */
extern void *memoryAlloc (const int n);

/* re-allocates n bytes of memory */
extern void *memoryRealloc (void *ptr, const int n);

/* frees memory */
extern void memoryFree (void *ptr);

/* get size of memory segment */
extern int memoryGetSize (const void *ptr);

/* prints memory statistics */
extern void memoryStatistics (void);

#endif

/*============================================================================*
 *                             that's all folks                               *
 *============================================================================*/
