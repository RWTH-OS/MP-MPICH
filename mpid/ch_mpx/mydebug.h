#ifndef _MYDEBUG_H
#define _MYDEBUG_H

#ifndef _NO_DEBUG
#undef _MYDEBUG_ENABLED
#else
#undef _MYDEBUG_ENABLED
#endif

#ifdef _MYDEBUG_ENABLED

static int MYDEBUG_RANK=-1;

#define DSTREAM stderr

#define DSETRANK(rank) MYDEBUG_RANK=rank;

#define DSECTION(section)  static char D_SECTION_NAME[] = section

#define DSECTENTRYPOINT fprintf(DSTREAM, "(%d) *** ENTRY OF SECTION: %s\n", MYDEBUG_RANK, D_SECTION_NAME); fflush(DSTREAM);

#define DSECTLEAVE fprintf(DSTREAM, "(%d) *** LEAVING SECTION:  %s\n\n", MYDEBUG_RANK, D_SECTION_NAME); fflush(DSTREAM);

#define DNOTICE(message) fprintf(DSTREAM,"(%d) *** [%s] %s\n", MYDEBUG_RANK, D_SECTION_NAME, message); fflush(DSTREAM);

#define DNOTICEI(message, integer) fprintf(DSTREAM,"(%d) *** [%s] %s %d\n", MYDEBUG_RANK, D_SECTION_NAME, message, integer); fflush(DSTREAM);

#define DNOTICES(message, string) fprintf(DSTREAM,"(%d) *** [%s] %s %s\n", MYDEBUG_RANK, D_SECTION_NAME, message, string); fflush(DSTREAM);

#define DERROR(message) fprintf(stderr,"(%d) !!! ERROR [%s] %s\n", MYDEBUG_RANK, D_SECTION_NAME, message); fflush(DSTREAM);

#define DWARNING(message) fprintf(stderr,"(%d) !!! WARNING [%s] %s\n", MYDEBUG_RANK, D_SECTION_NAME, message); fflush(DSTREAM);

#else

#define DSETRANK(rank)
#define DSECTION(section)  static char __dummy__
#define DSECTENTRYPOINT
#define DSECTLEAVE
#define DNOTICE(message)
#define DNOTICEI(message, integer)
#define DNOTICES(message, string)
#define DERROR(message)
#define DWARNING(message)

#endif

#endif






