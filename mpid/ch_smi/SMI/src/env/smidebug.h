/* $Id$ */
/* SMI debug & trace macros - derived from:
// MYDEBUG DEFINES
// Sehen, was ein Code macht!
//
// Version 0.1 vom 29.10.1998
// Autor: Nicolas Berr, LFBS RWTH-AACHEN DUMMY-Version
//
// Version 0.2 vom 23.09.1999
// erste implementationen fuer c-code (DSECTION, DNOTICE etc)
//
*/

#ifndef MY_DEBUG_C_H
#define MY_DEBUG_C_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_DEBUG) /* Debugging informationen nur falls gewuenscht einbauen */

#include <stdio.h>

double SMI_Wtime(void);

#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

/* these are defined in general_definitions.c */
extern int D_REC_DEPTH;
extern char D_REC_CHAR;
extern int D_DO_DEBUG;
extern int D_SHOW_TIME;
extern double D_GLOB_TIME;

#define DSETON  {D_DO_DEBUG = TRUE;}
#define DSETOFF {D_DO_DEBUG = FALSE;}
#define DTIMEON {D_SHOW_TIME = TRUE;}
#define DTIMEOFF {D_SHOW_TIME = FALSE;}
#define DTIMERESET {D_GLOB_TIME = SMI_Wtime();}

/* textuelles darstellen der Rekursionstiefe */
static void DMARK(void)
{
  static int i;
  for (i=0; i< D_REC_DEPTH; i++)
    fprintf(stderr,"%c",D_REC_CHAR);
}

/* das gleiche fuer Fehler ('*' anstelle '-') */
static void DEMARK(void)
{
  static int i;
  for (i=0; i< D_REC_DEPTH; i++)
    fprintf(stderr,"*");
}

#define DSECTION(section)  static char D_SECTION_NAME[] = section; \
int VERIFY_RD = D_REC_DEPTH; \
static int DRECI = FALSE; \
static int DMESI = FALSE; \
static double DTIMEIN, DTIMEOUT; \
static int DDATA = 0

/*
// auskommentierte Sektion
// es werden nur Fehlerinformationen ausgegeben
// keine NOTICES und keine sektions ein und austritte gemeldet oder dargestellt
*/
#define REMDSECTION(section)  static char D_SECTION_NAME[] = section; \
int VERIFY_RD = D_REC_DEPTH; \
static int DRECI = TRUE; \
static int DMESI = TRUE; \
static double DTIMEIN, DTIMEOUT; \
static int DDATA = 0

/*
// es werden lediglich keine NOTICES ausgegeben
// wohl aber sektions ein und austritte und Fehlerinformationen
*/
#define NNDSECTION(section)  static char D_SECTION_NAME[] = section; \
int VERIFY_RD = D_REC_DEPTH; \
static int DRECI = FALSE; \
static int DMESI = TRUE; \
static double DTIMEIN, DTIMEOUT; \
static int DDATA = 0

/*
// Zur ausgabe von Hinweisen, was der Code gerade macht
// diese Hinweise sollten keinerlei Fehlerbeschreibungskarakter haben
*/
#define DNOTICE(message) \
{ \
  if(D_DO_DEBUG) { \
    if (!DMESI) {  \
      DMARK(); \
      fprintf(stderr,"[%d] SMI NOTICE, %s: %s\n",_smi_my_proc_rank,D_SECTION_NAME,message);\
      fflush(stderr);  \
    } \
  } \
}

/*
// Zur ausgabe von Hinweisen, was der Code gerade macht plus einem Integer
// diese Hinweise sollten keinerlei Fehlerbeschreibungskarakter haben
*/
#define DNOTICEI(message,integer) \
{ \
  if(D_DO_DEBUG) { \
    if (!DMESI) { \
      DMARK(); \
      fprintf(stderr,"[%d] SMI NOTICE, %s: %s %d\n",_smi_my_proc_rank,D_SECTION_NAME,message,integer);\
      fflush(stderr); \
    } \
  } \
}
/*
// Zur ausgabe von Hinweisen, was der Code gerade macht plus einem Pointer
// diese Hinweise sollten keinerlei Fehlerbeschreibungskarakter haben
*/
#define DNOTICEP(message,pointer) \
{ \
  if(D_DO_DEBUG) { \
    if (!DMESI) { \
      DMARK(); \
      fprintf(stderr,"[%d] SMI NOTICE, %s: %s 0x%p\n",_smi_my_proc_rank,D_SECTION_NAME,message,pointer); \
      fflush(stderr); \
    } \
  } \
}
/*
// Zur Ausgabe von Hinweisen, was der Code gerade macht plus einem String
// diese Hinweise sollten keinerlei Fehlerbeschreibungskarakter haben
*/
#define DNOTICES(message,string) \
{ \
  if (D_DO_DEBUG) { \
    if (!DMESI) { \
      DMARK(); \
      fprintf(stderr,"[%d] SMI NOTICE, %s: %s %s\n",_smi_my_proc_rank,D_SECTION_NAME,message,string);\
      fflush(stderr); \
    } \
  } \
}

/*
// Zur Ausgabe von Hinweisen auf "unnormale Zustaende", die jedoch nicht den
// Abbruch des Programms erfordern
*/
#define DWARNING(message) \
{ \
  if(D_DO_DEBUG) { \
      DMARK(); \
      fprintf(stderr,"[%d] SMI WARNING, %s: %s\n",_smi_my_proc_rank,D_SECTION_NAME,message); \
      fflush(stderr);\
  } \
}

#define DWARNINGI(message, int_value) \
{ \
  if(D_DO_DEBUG) { \
      DMARK(); \
      fprintf(stderr,"[%d] SMI WARNING, %s: %s %d\n",_smi_my_proc_rank,D_SECTION_NAME,message, int_value); \
      fflush(stderr);\
  } \
}

/* Zur ausgabe von Fehlerinformationen, die auch im nicht DEBUG Fall angezeigt werden sollen */
#define DERROR(message) \
{ \
  if(D_DO_DEBUG) { \
    DEMARK(); \
    fprintf(stderr,"\n[%d] SMI ERROR, %s: %s\n",_smi_my_proc_rank,D_SECTION_NAME,message); \
    DEMARK(); \
    fprintf(stderr,"(%s:%d)\n",__FILE__,__LINE__); \
    fflush(stderr); \
  } \
  else { \
    fprintf(stderr,"\n[%d] SMI ERROR: %s\n",_smi_my_proc_rank,message); fflush(stderr); \
  } \
}

#define DERRORP(message,pointer) \
{ \
  if(D_DO_DEBUG) { \
    DEMARK(); \
    fprintf(stderr,"\n[%d] SMI ERROR, %s: %s 0x%p\n",_smi_my_proc_rank,D_SECTION_NAME,message,pointer);\
    DEMARK(); \
    fprintf(stderr,"(%s:%d)\n",__FILE__,__LINE__); \
    fflush(stderr); \
  } \
  else { \
    fprintf(stderr,"\n[%d] SMI ERROR: %s 0x%p\n",_smi_my_proc_rank,message,pointer); fflush(stderr); \
  } \
}

#define DERRORI(message, integer) \
{ \
  if(D_DO_DEBUG) { \
    DEMARK(); \
    fprintf(stderr,"\n[%d] SMI ERROR, %s: %s %d \n",_smi_my_proc_rank,D_SECTION_NAME,message,integer); \
    DEMARK(); \
    fprintf(stderr,"(%s:%d)\n",__FILE__,__LINE__); \
    fflush(stderr); \
  } \
  else { \
    fprintf(stderr,"\n[%d] SMI ERROR: %s %d\n",_smi_my_proc_rank,message,integer); fflush(stderr); \
  } \
}

/* Zur ausgabe von Fehlerinformationen, die auch im nicht DEBUG Fall angezeigt werden sollen */
#define DPROBLEM(message) \
{ \
  if(D_DO_DEBUG) { \
    DEMARK(); \
    fprintf(stderr,"[%d] SMI PROBLEM, %s: %s\n",_smi_my_proc_rank,D_SECTION_NAME,message); \
    DEMARK(); \
    fprintf(stderr,"(%s:%d)\n",__FILE__,__LINE__); \
    fflush(stderr); \
  } \
}

#define DPROBLEMP(message,pointer) \
{ \
  if(D_DO_DEBUG) { \
    DEMARK(); \
    fprintf(stderr,"[%d] SMI PROBLEM, %s: %s 0x%p\n",_smi_my_proc_rank,D_SECTION_NAME,message,pointer);\
    DEMARK(); \
    fprintf(stderr,"(%s:%d)\n",__FILE__,__LINE__); \
    fflush(stderr); \
  } \
}

#define DPROBLEMI(message, integer) \
{ \
  if(D_DO_DEBUG) { \
    DEMARK(); \
    fprintf(stderr,"[%d] SMI PROBLEM, %s: %s %d\n",_smi_my_proc_rank,D_SECTION_NAME,message,integer); \
    DEMARK(); \
    fprintf(stderr,"(%s:%d)\n",__FILE__,__LINE__); \
    fflush(stderr); \
  } \
}

/* "Offizieller" Einsprungpunkt einer Sektion */
#define DSECTENTRYPOINT \
{ \
  if (D_DO_DEBUG || D_SHOW_TIME) { \
    if (!DRECI) { \
      D_REC_DEPTH++; \
      fprintf(stderr,"\n"); \
      DMARK(); \
      if (D_SHOW_TIME) { \
	DTIMEIN = SMI_Wtime(); \
       fprintf(stderr, "[%d] (%.3f ms) SMI: ENTRY OF SECTION: %s\n",_smi_my_proc_rank,(DTIMEIN - D_GLOB_TIME) * 1000,D_SECTION_NAME);\
      } \
      else \
        fprintf(stderr, "[%d] SMI: ENTRY OF SECTION: %s\n",_smi_my_proc_rank,D_SECTION_NAME); fflush(stderr); \
    } \
  } \
}

/* "Offizieller" Ausprungpunkt */
#define DSECTLEAVE \
{ \
  if (D_DO_DEBUG || D_SHOW_TIME) { \
    if (!DRECI) { \
      DMARK(); \
      if (D_SHOW_TIME) { \
	DTIMEOUT = SMI_Wtime(); \
        fprintf(stderr, "[%d] (%.3f ms, %.0f us) SMI: LEAVING SECTION:  %s\n\n",_smi_my_proc_rank, \
	      (DTIMEOUT - D_GLOB_TIME) * 1000,(DTIMEOUT - DTIMEIN) * 1000000,D_SECTION_NAME); \
      } \
      else \
        fprintf(stderr, "[%d] SMI: LEAVING SECTION:  %s\n\n",_smi_my_proc_rank,D_SECTION_NAME); \
      D_REC_DEPTH--; \
      if (VERIFY_RD != D_REC_DEPTH) { \
        /* DERROR("DEBUGINTERNAL: recursion depth is invalid!");*/ \
        D_REC_DEPTH = VERIFY_RD; \
      } \
    } \
  } \
}

/* zur Ausgabe der aktuellen Laufzeit, an bestimmten Punkten. Kann nur innerhalb 
 * einer Sektion aufgerufen werden */
#define DTIMESTAMP(message) \
{ \
  if(D_SHOW_TIME) { \
    if (!DRECI) {  \
      DMARK(); \
	DTIMEOUT = SMI_Wtime(); \
      fprintf(stderr, "[%d] (%.3f ms) SMI TIMESTAMP, %s: %s\n",_smi_my_proc_rank, \
                 (DTIMEOUT - D_GLOB_TIME) * 1000,D_SECTION_NAME,message); \
      fflush(stderr);  \
    } \
  } \
}

/* Standard definierte DEBUG SECTION, ueber das gamze modul */
static char D_SECTION_NAME[] =  "GLOBAL DEBUG SECTION";        \
static int DRECI = FALSE;                                      \
static int DMESI = FALSE;                                      \
static int DDATA = 0;

/* nichtimplementierte Makros durch Leertext ersetzen */

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

/*
// Falls keine Debuginformationen gewuenscht werden alle vorkommen im Sourcecode durch 
// leertext ersetzen... 
*/
#else

#define DSETOUTPUT(m)
#define DSETOUTPUTF(m)
#define DSETPROCID(m)
#define DSECTION(m)
#define REMDSECTION(m)
#define NNDSECTION(m)
#define DNOTICE(m)
#define DNOTICES(m,s)
#define DNOTICEI(m,i)
#define DNOTICEP(m,p)
#define DIFNOTICE(m) 
#define DWARNING(m)
#define DWARNINGI(m,i)
#define DERROR(message) { fprintf(stderr,"\n[%d] SMI ERROR: %s\n",_smi_my_proc_rank,message); fflush(stderr); }
#define DERRORI(message,integer) {fprintf(stderr,"\n[%d] SMI ERROR: %s %d\n",_smi_my_proc_rank,message,integer); fflush(stderr);}
#define DERRORP(message,pointer) {fprintf(stderr,"\n[%d] SMI ERROR: %s %p\n",_smi_my_proc_rank,message,pointer); fflush(stderr);}
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
#define DTIMEON 
#define DTIMEOFF 
#define DTIMERESET
#define DTIMESTAMP(m)
#endif

#ifdef __cplusplus
}
#endif

#endif
