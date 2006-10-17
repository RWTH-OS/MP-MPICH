/* MYDEBUG DEFINES
// Sehen, was ein Code macht!
//
// Version 0.1 vom 29.10.1998
// Autor: Nicolas Berr, LFBS RWTH-AACHEN
//
// Version 0.2 vom 13.11.98
// DLOOPSET und DLOOPZERO eingefuegt
//
// Version 0.3 vom 01.03.99
// DSETPROCID eingefuegt, flush an jede Ausgabe angehaengt
//
// Version 0.4 vom 02.03.99
// DIFNOTICE eingefuegt
//
// Version 0.5 vom 20.04.99
// DSETOUTPUT und DSETOUTPUTF eingefuegt
//
// Version 0.6 vom 23.09.99
// _DEBUG nun unabhaengig von _DEBUG_EXTERN_REC und _DEBUG_MAIN_REC
// Anpassungen fuer Compatibilitaet zum C-Interface (DNOTICEI, DNOTICEP, DNOTICES)
*/

#ifndef MY_DEBUG_H
#define MY_DEBUG_H

/* #define _DEBUG   */

#ifdef _DEBUG_EXTERN_REC   /* fuer die Zwecke einer Moduluebergreifenden Rekursionsueberwachung */
/* #define _DEBUG */
#endif

#ifdef _DEBUG_MAIN_REC     /* bei Moduluebergreifender Rekursionsueberwachung dies im MAIN modul benutzen */
/* #define _DEBUG */
#endif

#if defined(_DEBUG) && defined(__cplusplus)      /* Debugging informationen nur falls gewuenscht einbauen */

#include <iostream>
#include <fstream>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE !FALSE
#endif

// Global im Gesamten Project, die main Moduldatei muss dann die eigentliche Variable enthalten
extern int D_REC_DEPTH;
extern char D_REC_CHAR;
extern std::ostream* D_COUT;

// textuelles darstellen der Rekursionstiefe
inline void DMARK(){                  
  for (int i=0; i< D_REC_DEPTH; i++)
    (*D_COUT) << D_REC_CHAR;
}

// das gleiche fuer Fehler ('*' anstelle '-')
inline void DEMARK() {               
  for (int i=0; i< D_REC_DEPTH; i++)
    (*D_COUT) << "*";
}

// Setzen des Ausgabestreams
#define DSETOUTPUT(output) \
{\
  D_COUT = &(output);\
}

// Setzen des einer Datei als Ausgabestream
#define DSETOUTPUTF(fname) \
{\
   D_COUT = new std::ofstream(fname);\
}

// Setzen der ProcID fuer den Einsatz bei Mehrprozessprogrammen
// Man sollte sie so frueh wie moeglich setzen
#define DSETPROCID(proc_id) \
{\
   if(proc_id < 0)\
     {\
	DERROR("negative Process ID!!!");\
	exit(-1);\
     }\
     \
     D_REC_DEPTH++;\
     \
     if(proc_id < 10)\
       D_REC_CHAR = '0' + proc_id;\
     else\
       D_REC_CHAR = 'a' + proc_id;\
     if(proc_id >= 16)\
       DNOTICE("You should Not Debug more than 16 Processes!");\
     (*D_COUT) << std::endl;\
     DMARK();\
     (*D_COUT) << " ENTRY OF SECTION: " << D_SECTION_NAME << std::endl << std::flush;\
}

// **** Definition einer Sektion, und deren Eigenschaften
// **** Sektionsdefinitionen gelten innerhalb eines ganzen Blockes { }
// **** in jedem Block kann ein unterblock mit eigener Sektionsdefinition angelegt werden

// normale Sektion
// ausgabe aller debug informationen
// ueberwachung der Rekursionstiefe
#define DSECTION(section)  static char D_SECTION_NAME[] = section;                        \
                           static int DRECI = FALSE;                                      \
                           static int DMESI = FALSE;                                      \
                           static int DDATA = 0;

// auskommentierte Sektion
// es werden nur Fehlerinformationen ausgegeben
// keine NOTICES und keine sektions ein und austritte gemeldet oder dargestellt
#define REMDSECTION(section) static char D_SECTION_NAME[] = section;                        \
                             static int DRECI = TRUE;                                       \
                             static int DMESI = TRUE;                                       \
                             static int DDATA = 0;

// es werden lediglich keine NOTICES ausgegeben
// wohl aber sektions ein und austritte und Fehlerinformationen
#define NNDSECTION(section)  static char D_SECTION_NAME[] = section;                        \
                             static int DRECI = FALSE;                                      \
                             static int DMESI = TRUE;                                       \
                             static int DDATA = 0;

#define DLOOPSET               int DLOOPZ = 0;
#define DLOOPSETN(loopname)     int loopname = 0;

#define DLOOPZERO              ( DLOOPZ++ == 0)
#define DLOOPZERON(loopname)    ((loopname)++ == 0)


#define DLOOPDIV(div)          ((DLOOPZ++ % (div)) == 0)
#define DLOOPDIVN(loopname,div) (((loopname)++ % (div)) == 0)

// Zur ausgabe von Hinweisen, was der Code gerade macht
// diese Hinweise sollten keinerlei Fehlerbeschreibungskarakter haben
#define DNOTICE(meldung) { if (!DMESI) {                                                           \
			       DMARK();                                                            \
			       (*D_COUT) << " " << D_SECTION_NAME << ": " << meldung << std::endl << std::flush;  \
			   }                                                                       \
			 }


// die folgenden NOTICE versionen sind nur aus kompatibilitaetsgruenden zum C-Interface vorhanden
// sie sollten in cplusplus code nicht unbedingt benutzt werden
#define DNOTICEI(meldung,integer) { if (!DMESI) {                                                           \
			       DMARK();                                                            \
			       (*D_COUT) << " " << D_SECTION_NAME << ": " << meldung << " " << integer << std::endl << std::flush;  \
			   }                                                                       \
			 }
#define DNOTICEP(meldung,pointer) { if (!DMESI) {                                                           \
			       DMARK();                                                            \
			       (*D_COUT) << " " << D_SECTION_NAME << ": " << meldung << " " << (void*) pointer << std::endl << std::flush;  \
			   }                                                                       \
			 }
#define DNOTICES(meldung,string) { if (!DMESI) {                                             \
			       DMARK();                                                      \
			       (*D_COUT) << " " << D_SECTION_NAME << ": " << meldung << " "  \
					<< string << std::endl << std::flush;                \
			   }                                                                 \
			 }


// Zum Ausfuehren eines Commandos (z.B. sleep(1); in einer Schleife) im Falle
// von eingeschalteten Debugausgaben
#define DIFNOTICE(befehl) { if (!DMESI) {befehl;} } 

// Zur ausgabe von Fehlerinformationen, die im nicht DEBUG Fall nicht angezeigt werden sollen
#define DERROR(meldung)  {                                                                      \
			       DEMARK();                                                        \
			       (*D_COUT) << " ERROR in Section \'" << D_SECTION_NAME << "\': "  \
				    << meldung << std::endl << std::flush;                      \
			 }

// "Offizieller" Einsprungpunkt einer Sektion
#define DSECTENTRYPOINT { if (!DRECI) {                                                           \
			        D_REC_DEPTH++;                                                    \
				(*D_COUT) << std::endl;                                           \
				DMARK();                                                          \
				(*D_COUT) << " ENTRY OF SECTION: " << D_SECTION_NAME <<           \
					std::endl << std::flush;                                  \
			  }                                                                       \
			}

// "Offizieller" Einsprungpunkt einer LoopSection
#define DLOOPENTRYPOINT(start) {                                                                    \
                                  if (!DRECI) {                                                     \
				    D_REC_DEPTH++;                                                  \
				    if ((start)){                                                   \
				      (*D_COUT) << std::endl;                                       \
				      DMARK();                                                      \
				      (*D_COUT) << " START OF LOOP: " << D_SECTION_NAME             \
					   << std::endl << std::endl << std::flush;                 \
				      DDATA = 0;                                                    \
				    }                                                               \
				    DMARK();                                                        \
				    (*D_COUT) << " LOOP PASS " << DDATA << std::endl << std::flush; \
				  }                                                                 \
			     }

// "Offizieller" Ausprungpunkt 
#define DSECTLEAVE {    if (!DRECI) {                                                         \
			     DMARK();                                                         \
			     (*D_COUT) << " LEAVING SECTION:  " << D_SECTION_NAME             \
				  << std::endl << std::endl << std::flush;                    \
			     D_REC_DEPTH--;                                                   \
			}                                                                     \
		   }

// "Offizieller" Ausprungpunkt 
#define DLOOPLEAVE {    if (!DRECI) {                                                         \
			     (*D_COUT) << std::endl << std::flush;                            \
			     DDATA++;                                                         \
			     D_REC_DEPTH--;                                                   \
			}                                                                     \
		   }

// Vordefinierte Sektion, um alle Makros auch ohne spezielle Sektionsdefinition nutzen zu koennen
DSECTION("GLOBAL DEBUG SECTION");

// Falls keine Debuginformationen gewuenscht werden alle vorkommen im Sourcecode durch 
// leertext ersetzen...
#else

#define DSETOUTPUT(m)
#define DSETOUTPUTF(m)
#define DSETPROCID(m)

#ifndef __cplusplus
#define DSECTION(m) static int _xdummy_
#define REMDSECTION(m) static int _xdummy_
#define NNDSECTION(m) static int _xdummy_
#else
#define DSECTION(m)
#define REMDSECTION(m)
#define NNDSECTION(m)
#endif /*__cplusplus*/

#define DNOTICE(m)
#define DNOTICES(m,s)
#define DNOTICEI(m,i)
#define DNOTICEP(m,p)
#define DIFNOTICE(m) 
#define DWARNING(m)
#define DERROR(m) { (cerr) << "ERROR: " << m << std::endl << std::flush;}
#define DERRORI(message,integer) { (cerr) << "ERROR: " << message << " " << integer << std::endl << std::flush;}
#define DERRORP(message,pointer) { (cerr) << "ERROR: " << message << " " << (void*) pointer << std::endl << std::flush;}
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
#endif /* MY_DEBUG_H */

