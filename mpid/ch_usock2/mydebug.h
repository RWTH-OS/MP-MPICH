#ifndef _MYDEBUG_H
#define _MYDEBUG_H

#undef _MYDEBUG_ENABLED

#ifdef _MYDEBUG_ENABLED

#define DSTREAM stderr

#define DSECTION(section)  static char D_SECTION_NAME[] = section

#define DSECTENTRYPOINT fprintf(DSTREAM, "*** ENTRY OF SECTION: %s\n", D_SECTION_NAME); fflush(DSTREAM);

#define DSECTLEAVE fprintf(DSTREAM, "*** LEAVING SECTION:  %s\n\n", D_SECTION_NAME); fflush(DSTREAM);

#define DNOTICE(message) fprintf(DSTREAM,"*** [%s] %s\n", D_SECTION_NAME, message); fflush(DSTREAM);

#define DNOTICEI(message, integer) fprintf(DSTREAM,"*** [%s] %s %d\n", D_SECTION_NAME, message, integer); fflush(DSTREAM);

#define DNOTICES(message, string) fprintf(DSTREAM,"*** [%s] %s %s\n", D_SECTION_NAME, message, string); fflush(DSTREAM);

#define DERROR(message) fprintf(stderr,"!!! ERROR [%s] %s\n", D_SECTION_NAME, message); fflush(DSTREAM);

#define DWARNING(message) fprintf(stderr,"!!! WARNING [%s] %s\n", D_SECTION_NAME, message); fflush(DSTREAM);

#else

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






