/* MYDEBUG DEFINES
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

#if defined(_DEBUG) /* Debugging informationen nur falls gewuenscht einbauen */

#include <stdio.h>

#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

/* Global im Gesamten Project, die main Moduldatei muss dann die eigentliche Variable enthalten */
extern int D_REC_DEPTH;
extern char D_REC_CHAR;
extern int D_DO_DEBUG;

/* textuelles darstellen der Rekursionstiefe */
static void DMARK(){     
  static int i;
  for (i=0; i< D_REC_DEPTH; i++)
    fprintf(stderr,"%c",D_REC_CHAR);
}

/* das gleiche fuer Fehler ('*' anstelle '-') */
static void DEMARK(){ 
  static int i;
  for (i=0; i< D_REC_DEPTH; i++)
    fprintf(stderr,"*");
}

#define DSETON { D_DO_DEBUG = TRUE; }
#define DSETOFF { D_DO_DEBUG = FALSE; }

#define DSECTION(section)  static char D_SECTION_NAME[] = section; \
static int DRECI = FALSE; \
static int DMESI = FALSE; \
static int DDATA = 0

/*
// auskommentierte Sektion
// es werden nur Fehlerinformationen ausgegeben
// keine NOTICES und keine sektions ein und austritte gemeldet oder dargestellt
*/
#define REMDSECTION(section)  static char D_SECTION_NAME[] = section; \
static int DRECI = TRUE; \
static int DMESI = TRUE; \
static int DDATA = 0

/*
// es werden lediglich keine NOTICES ausgegeben
// wohl aber sektions ein und austritte und Fehlerinformationen
*/
#define NNDSECTION(section)  static char D_SECTION_NAME[] = section; \
static int DRECI = FALSE; \
static int DMESI = TRUE; \
static int DDATA = 0

/*
// Zur ausgabe von Hinweisen, was der Code gerade macht
// diese Hinweise sollten keinerlei Fehlerbeschreibungskarakter haben
*/
#define DNOTICE(meldung) \
{ \
  if(D_DO_DEBUG) { \
    if (!DMESI) {  \
      DMARK(); \
      fprintf(stderr," %s: %s\n",D_SECTION_NAME,meldung); fflush(stderr);  \
    } \
  } \
}
/*
// Zur ausgabe von Hinweisen, was der Code gerade macht plus einem Integer
// diese Hinweise sollten keinerlei Fehlerbeschreibungskarakter haben
*/
#define DNOTICEI(meldung,integer) \
{ \
  if(D_DO_DEBUG) { \
    if (!DMESI) { \
      DMARK(); \
      fprintf(stderr," %s: %s %d\n",D_SECTION_NAME,meldung,integer); fflush(stderr); \
    } \
  } \
}
/*
// Zur ausgabe von Hinweisen, was der Code gerade macht plus einem Pointer
// diese Hinweise sollten keinerlei Fehlerbeschreibungskarakter haben
*/
#define DNOTICEP(meldung,pointer) \
{ \
  if(D_DO_DEBUG) { \
    if (!DMESI) { \
      DMARK(); \
      fprintf(stderr," %s: %s %x\n",D_SECTION_NAME,meldung,pointer); fflush(stderr); \
    } \
  } \
}
/*
// Zur ausgabe von Hinweisen, was der Code gerade macht plus einem String
// diese Hinweise sollten keinerlei Fehlerbeschreibungskarakter haben
*/
#define DNOTICES(meldung,string) \
{ \
  if (D_DO_DEBUG) { \
    if (!DMESI) { \
      DMARK(); \
      fprintf(stderr," %s: %s %s\n",D_SECTION_NAME,meldung,string); fflush(stderr); \
    } \
  } \
}

/* Zur ausgabe von Fehlerinformationen, die auch im nicht DEBUG Fall angezeigt werden sollen */
#define DERROR(meldung) \
{ \
  if(D_DO_DEBUG) { \
    DEMARK(); \
    fprintf(stderr,"ERROR in Section '%s': %s\n",D_SECTION_NAME,meldung); \
    fflush(stderr); \
  } \
  else { \
    fprintf(stderr,"ERROR: %s\n",meldung); fflush(stderr); \
  } \
}

/* "Offizieller" Einsprungpunkt einer Sektion */
#define DSECTENTRYPOINT \
{ \
  if (D_DO_DEBUG) { \
    if (!DRECI) { \
      D_REC_DEPTH++; \
      fprintf(stderr,"\n"); \
      DMARK(); \
      fprintf(stderr, " ENTRY OF SECTION: %s\n",D_SECTION_NAME); fflush(stderr); \
    } \
  } \
}

/* "Offizieller" Ausprungpunkt */
#define DSECTLEAVE \
{ \
  if (D_DO_DEBUG) { \
    if (!DRECI) { \
      DMARK(); \
      fprintf(stderr, " LEAVING SECTION:  %s\n\n", D_SECTION_NAME); \
      D_REC_DEPTH--; \
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

#ifndef __cplusplus
#define DSECTION(m)  static int _xdummy_
#define REMDSECTION(m) static int _xdummy_
#define NNDSECTION(m) static int _xdummy_
#else
#define DSECTION(m)
#define REMDSECTION(m)
#define NNDSECTION(m)
#endif __cplusplus

#define DNOTICE(m)
#define DNOTICES(m,s)
#define DNOTICEI(m,i)
#define DNOTICEP(m,p)
#define DIFNOTICE(m) 
#define DWARNING(m)
#define DERROR(message) { fprintf(stderr,"ERROR: %s\n",message); fflush(stderr); }
#define DERRORI(message,integer) {fprintf(stderr,"ERROR: %s %d\n",message,integer); fflush(stderr);}
#define DERRORP(message,pointer) {fprintf(stderr,"ERROR: %s %x\n",message,pointer); fflush(stderr);}
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
#endif _DEBUG
#endif MY_DEBUG_H

