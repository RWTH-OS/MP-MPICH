/*--------------------------------------------------------------------------*/
/*                                                                          */
/* PhD-Project                                                              */
/* (c) 1998 Martin Schulz                                                   */
/*                                                                          */
/* Global header file: basetype.h                                           */
/* Defines all global types for all modules                                 */
/*                                                                          */
/* Headerfile (exported to all other modules)                               */
/*                                                                          */
/*--------------------------------------------------------------------------*/

#ifndef _GLOB_BASETYPE
#define _GLOB_BASETYPE

/*--------------------------------------------------------------------------*/
/* Integertypes */

#ifdef WIN32
typedef unsigned long		uint;
#endif

#ifndef _SYS_TYPES_H
#ifdef LINUX
typedef unsigned long		uint;
#endif
#endif

typedef uint *uint_p;

#ifdef LINUX
typedef unsigned int  BOOL;
typedef int           SOCKET;

#ifndef FALSE
#define TRUE  1
#define FALSE 0
#endif
#endif


/*--------------------------------------------------------------------------*/
/* The End. */

#endif
