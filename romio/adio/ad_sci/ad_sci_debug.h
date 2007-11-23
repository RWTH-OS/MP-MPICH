/* 
 * $Id: ad_sci_debug.h 1328 2001-11-22 13:55:10Z nicolas $
 *
 * debug and tracing macros
 */

#ifndef _ADSCI_DEBUG_H
#define _ADSCI_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif


#if defined(ADSCI_DEBUG)

#include <stdio.h>
#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

/* these are defined in ad_sci_debug.c */
extern int _adsci_REC_DEPTH;
extern int _adsci_DO_DEBUG;

#define DSETON  {_adsci_DO_DEBUG = TRUE;}
#define DSETOFF {_adsci_DO_DEBUG = FALSE;}

void _adsci_DMARK();
void _adsci_DEMARK();

#define DSECTION(section)  static char D_SECTION_NAME[] = section; \
static int DRECI = FALSE; \
static int DMESI = FALSE; \
static int DDATA = 0

/* only print error messages for this section, no tracing */
#define REMDSECTION(section)  static char D_SECTION_NAME[] = section; \
static int DRECI = TRUE; \
static int DMESI = TRUE; \
static int DDATA = 0

/* less verbose tracing (no notices) */
#define NNDSECTION(section)  static char D_SECTION_NAME[] = section; \
static int DRECI = FALSE; \
static int DMESI = TRUE; \
static int DDATA = 0

/* give a notice about what the code is doing */
#define DNOTICE(message) \
{ \
  if(_adsci_DO_DEBUG) { \
    if (!DMESI) {  \
      _adsci_DMARK(); \
      fprintf(stderr,"[%d] AD_SCI NOTICE, %s: %s\n",_adsci_proc_rank,D_SECTION_NAME,message);\
      fflush(stderr);  \
    } \
  } \
}

/* give a notice, and print an additional integer value */
#define DNOTICEI(message,integer) \
{ \
  if(_adsci_DO_DEBUG) { \
    if (!DMESI) { \
      _adsci_DMARK(); \
      fprintf(stderr,"[%d] AD_SCI NOTICE, %s: %s %d\n",_adsci_proc_rank,D_SECTION_NAME,message,integer);\
      fflush(stderr); \
    } \
  } \
}

/* print a notice with a ptr */
#define DNOTICEP(message,pointer) \
{ \
  if(_adsci_DO_DEBUG) { \
    if (!DMESI) { \
      _adsci_DMARK(); \
      fprintf(stderr,"[%d] AD_SCI NOTICE, %s: %s 0x%x\n",_adsci_proc_rank,D_SECTION_NAME,message,pointer);\
      fflush(stderr); \
    } \
  } \
}

/* print a notice with a string */
#define DNOTICES(message,string) \
{ \
  if (_adsci_DO_DEBUG) { \
    if (!DMESI) { \
      _adsci_DMARK(); \
      fprintf(stderr,"[%d] AD_SCI NOTICE, %s: %s %s\n",_adsci_proc_rank,D_SECTION_NAME,message,string);\
      fflush(stderr); \
    } \
  } \
}

/* print a warning (something did not work the normal way, but we can continue) */
#define DWARNING(message) \
{ \
  if(_adsci_DO_DEBUG) { \
    if (!DMESI) {  \
      _adsci_DMARK(); \
      fprintf(stderr,"[%d] AD_SCI WARNING, %s: %s\n",_adsci_proc_rank,D_SECTION_NAME,message);\
      fflush(stderr);\
    } \
  } \
}

/* a real error occured, continuation is not possible */
#define DERROR(message) \
{ \
  if(_adsci_DO_DEBUG) { \
    _adsci_DEMARK(); \
    fprintf(stderr,"\n[%d] AD_SCI ERROR, %s: %s\n",_adsci_proc_rank,D_SECTION_NAME,message); \
    _adsci_DEMARK(); \
    fprintf(stderr,"(%s:%d)\n",__FILE__,__LINE__); \
    fflush(stderr); \
  } \
  else { \
    fprintf(stderr,"\n[%d] AD_SCI ERROR: %s\n",_adsci_proc_rank,message); fflush(stderr); \
  } \
}

/* a real error occured, continuation is not possible; also print a ptr */
#define DERRORP(message,pointer) \
{ \
  if(_adsci_DO_DEBUG) { \
    _adsci_DEMARK(); \
    fprintf(stderr,"\n[%d] AD_SCI ERROR, %s: %s 0x%x\n",_adsci_proc_rank,D_SECTION_NAME,message,pointer);\
    _adsci_DEMARK(); \
    fprintf(stderr,"(%s:%d)\n",__FILE__,__LINE__); \
    fflush(stderr); \
  } \
  else { \
    fprintf(stderr,"\n[%d] AD_SCI ERROR: %s 0x%x\n",_adsci_proc_rank,message,pointer); fflush(stderr); \
  } \
}

/* a real error occured, continuation is not possible; also print an integer (error nbr) */
#define DERRORI(message, integer) \
{ \
  if(_adsci_DO_DEBUG) { \
    _adsci_DEMARK(); \
    fprintf(stderr,"\n[%d] AD_SCI ERROR, %s: %s %d \n",_adsci_proc_rank,D_SECTION_NAME,message,integer); \
    _adsci_DEMARK(); \
    fprintf(stderr,"(%s:%d)\n",__FILE__,__LINE__); \
    fflush(stderr); \
  } \
  else { \
    fprintf(stderr,"\n[%d] AD_SCI ERROR: %s %d\n",_adsci_proc_rank,message,integer); fflush(stderr); \
  } \
}

/* an error occured in a sub-system. This error will be processed by upper layers and will probably 
   lead to the termination of the process */
#define DPROBLEM(message) \
{ \
  if(_adsci_DO_DEBUG) { \
    _adsci_DEMARK(); \
    fprintf(stderr,"[%d] AD_SCI PROBLEM, %s: %s\n",_adsci_proc_rank,D_SECTION_NAME,message); \
    _adsci_DEMARK(); \
    fprintf(stderr,"(%s:%d)\n",__FILE__,__LINE__); \
    fflush(stderr); \
  } \
}

#define DPROBLEMP(message,pointer) \
{ \
  if(_adsci_DO_DEBUG) { \
    _adsci_DEMARK(); \
    fprintf(stderr,"[%d] AD_SCI PROBLEM, %s: %s 0x%x\n",_adsci_proc_rank,D_SECTION_NAME,message,pointer);\
    _adsci_DEMARK(); \
    fprintf(stderr,"(%s:%d)\n",__FILE__,__LINE__); \
    fflush(stderr); \
  } \
}

#define DPROBLEMI(message, integer) \
{ \
  if(_adsci_DO_DEBUG) { \
    _adsci_DEMARK(); \
    fprintf(stderr,"[%d] AD_SCI PROBLEM, %s: %s %d\n",_adsci_proc_rank,D_SECTION_NAME,message,integer); \
    _adsci_DEMARK(); \
    fprintf(stderr,"(%s:%d)\n",__FILE__,__LINE__); \
    fflush(stderr); \
  } \
}

/* start of a section */
#define DSECTENTRYPOINT \
{ \
  if (_adsci_DO_DEBUG) { \
    if (!DRECI) { \
      _adsci_REC_DEPTH++; \
      fprintf(stderr,"\n"); \
      _adsci_DMARK(); \
      fprintf(stderr, "[%d] AD_SCI: ENTRY OF SECTION: %s\n",_adsci_proc_rank,D_SECTION_NAME); fflush(stderr); \
    } \
  } \
}

/* end of a section */
#define DSECTLEAVE \
{ \
  if (_adsci_DO_DEBUG) { \
    if (!DRECI) { \
      _adsci_DMARK(); \
      fprintf(stderr, "[%d] AD_SCI: LEAVING SECTION:  %s\n\n",_adsci_proc_rank,D_SECTION_NAME); \
      _adsci_REC_DEPTH--; \
    } \
  } \
}

/* default DEBUG SECTION for this module */
static char D_SECTION_NAME[] =  "GLOBAL DEBUG SECTION";        \
static int DRECI = FALSE;                                      \
static int DMESI = FALSE;                                      \
static int DDATA = 0;

/* non-implemented macros */
#define DSETOUTPUT(m)
#define DSETOUTPUTF(m)
#define DSETPROCID(m) 
#define DIFNOTICE(m) 
#define DLOOPENTRYPOINT(m)
#define DLOOPLEAVE
#define DLOOPSET
#define DLOOPSETN(m)
#define DLOOPZERO
#define DLOOPZERON(m)
#define DLOOPDIV(m)
#define DLOOPDIVN(m,n)

#else
/* debugging/tracing is disabled */

#define DSETOUTPUT(m)
#define DSETOUTPUTF(m)
#define DSETPROCID(m)

#ifndef __cplusplus
#define DSECTION(m)  static int _xdummy_
#define REMDSECTION(m) static int _xdummy_
#define NNDSECTION(m) static int _xdummy_
#else
#define DSECTION(m)
#define REMDSECTION(m)
#define NNDSECTION(m)
#endif /* __cplusplus */

#define DNOTICE(m)
#define DNOTICES(m,s)
#define DNOTICEI(m,i)
#define DNOTICEP(m,p)
#define DIFNOTICE(m) 
#define DWARNING(m)
#define DERROR(message) { fprintf(stderr,"\n[%d] AD_SCI ERROR: %s\n",_adsci_proc_rank,message); fflush(stderr); }
#define DERRORI(message,integer) {fprintf(stderr,"\n[%d] AD_SCI ERROR: %s %d\n",_adsci_proc_rank,message,integer); fflush(stderr);}
#define DERRORP(message,pointer) {fprintf(stderr,"\n[%d] AD_SCI ERROR: %s %x\n",_adsci_proc_rank,message,pointer); fflush(stderr);}
#define DPROBLEM(message)
#define DPROBLEMI(message,integer)
#define DPROBLEMP(message,pointer)
#define DSECTENTRYPOINT
#define DSECTLEAVE
#define DLOOPENTRYPOINT(m)
#define DLOOPLEAVE
#define DLOOPSET
#define DLOOPSETN(m)
#define DLOOPZERO
#define DLOOPZERON(m)
#define DLOOPDIV(m)
#define DLOOPDIVN(m,n)
#define DSETON
#define DSETOFF

#endif /* _DEBUG */

#ifdef __cplusplus
}
#endif

#endif /* _ADSCI_DEBUG_H */




// 
// Overrides for XEmacs and vim so that we get a uniform tabbing style.
// XEmacs/vim will notice this stuff at the end of the file and automatically
// adjust the settings for this buffer only.  This must remain at the end
// of the file.
// ---------------------------------------------------------------------------
// Local variables:
// c-indent-level: 3
// c-basic-offset: 3
// tab-width: 3
// End:
// vim:tw=0:ts=3:wm=0:
// 
