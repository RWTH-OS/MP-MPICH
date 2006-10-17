/* $Id$
 * (c) 2005 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de

 *  macros for debugging messages
 */

#ifndef __RDEBUG_H_
#define __RDEBUG_H_
#include <stdio.h>

extern char RDEBUG_dbgprefix[256]; /* name of the host returned by gethostbyname() */
extern char RDEBUG_serr[512];      /* string for formating debug/error messages */
extern int  RDEBUG_verbose;        /* switch for turning on verbose output */

/* debugging messages 
#ifndef ROUTER_DEBUG
#define ROUTER_DEBUG
#endif
*/

#ifdef ROUTER_DEBUG
#define RDEBUG(a) fprintf(stderr, a); fflush(stderr);
#define RDEBUG1(a,b) fprintf(stderr, a, b); fflush(stderr);
#define RDEBUG2(a,b,c) fprintf(stderr, a, b, c); fflush(stderr);
#define RDEBUG3(a,b,c,d) fprintf(stderr, a, b, c, d); fflush(stderr);
#define RDEBUG4(a,b,c,d,e) fprintf(stderr, a, b, c, d, e); fflush(stderr);
#define RDEBUG5(a,b,c,d,e,f) fprintf(stderr, a, b, c, d, e, f); fflush(stderr);

#define PRDEBUG(a) fprintf(stderr, "%s", RDEBUG_dbgprefix); fprintf(stderr, a); fflush(stderr);
#define PRDEBUG1(a,b) fprintf(stderr, "%s", RDEBUG_dbgprefix); fprintf(stderr, a, b); fflush(stderr);
#define PRDEBUG2(a,b,c)  fprintf(stderr, "%s", RDEBUG_dbgprefix);  fprintf(stderr, a, b, c); fflush(stderr);
#define PRDEBUG3(a,b,c,d)  fprintf(stderr, "%s", RDEBUG_dbgprefix); fprintf(stderr, a, b, c, d); fflush(stderr);
#define PRDEBUG4(a,b,c,d,e) fprintf(stderr, "%s", RDEBUG_dbgprefix);  fprintf(stderr, a, b, c, d, e); fflush(stderr);
#define PRDEBUG5(a,b,c,d,e,f) fprintf(stderr, "%s", RDEBUG_dbgprefix);  fprintf(stderr, a, b, c, d, e, f); fflush(stderr);

#else
#define RDEBUG(a) 
#define RDEBUG1(a,b)
#define RDEBUG2(a,b,c)
#define RDEBUG3(a,b,c,d)
#define RDEBUG4(a,b,c,d,e)
#define RDEBUG5(a,b,c,d,e,f)

#define PRDEBUG(a) 
#define PRDEBUG1(a,b)
#define PRDEBUG2(a,b,c)
#define PRDEBUG3(a,b,c,d)
#define PRDEBUG4(a,b,c,d,e)
#define PRDEBUG5(a,b,c,d,e,f)
#endif


#define RVERBOSE(a) if (RDEBUG_verbose) {fprintf(stderr, a); fflush(stderr);}
#define RVERBOSE1(a,b) if (RDEBUG_verbose) {fprintf(stderr, a, b); fflush(stderr);}
#define RVERBOSE2(a,b,c) if (RDEBUG_verbose) {fprintf(stderr, a, b, c); fflush(stderr);}
#define RVERBOSE3(a,b,c,d) if (RDEBUG_verbose) {fprintf(stderr, a, b, c, d); fflush(stderr);}
#define RVERBOSE4(a,b,c,d,e) if (RDEBUG_verbose) {fprintf(stderr, a, b, c, d, e); fflush(stderr);}
#define RVERBOSE5(a,b,c,d,e,f) if (RDEBUG_verbose) {fprintf(stderr, a, b, c, d, e, f); fflush(stderr);}

#define PRVERBOSE(a) if (RDEBUG_verbose) {fprintf(stderr, "%s", RDEBUG_dbgprefix); fprintf(stderr, a); fflush(stderr);}
#define PRVERBOSE1(a,b) if (RDEBUG_verbose) {fprintf(stderr, "%s", RDEBUG_dbgprefix); fprintf(stderr, a, b); fflush(stderr);}
#define PRVERBOSE2(a,b,c)  if (RDEBUG_verbose) {fprintf(stderr, "%s", RDEBUG_dbgprefix);  fprintf(stderr, a, b, c); fflush(stderr);}
#define PRVERBOSE3(a,b,c,d)  if (RDEBUG_verbose) {fprintf(stderr, "%s", RDEBUG_dbgprefix); fprintf(stderr, a, b, c, d); fflush(stderr);}
#define PRVERBOSE4(a,b,c,d,e) if (RDEBUG_verbose) {fprintf(stderr, "%s", RDEBUG_dbgprefix);  fprintf(stderr, a, b, c, d, e); fflush(stderr);}
#define PRVERBOSE5(a,b,c,d,e,f) if (RDEBUG_verbose) {fprintf(stderr, "%s", RDEBUG_dbgprefix);  fprintf(stderr, a, b, c, d, e, f); fflush(stderr);}


#define RERROR(a) fprintf(stderr, a); fflush(NULL);
#define RERROR1(a,b) fprintf(stderr, a, b); fflush(NULL);
#define RERROR2(a,b,c) fprintf(stderr, a, b, c); fflush(NULL);
#define RERROR3(a,b,c,d) fprintf(stderr, a, b, c, d); fflush(NULL);
#define RERROR4(a,b,c,d,e) fprintf(stderr, a, b, c, d, e); fflush(NULL);

#define PRERROR(a) fprintf(stderr, "%s", RDEBUG_dbgprefix);  fprintf(stderr, a); fflush(NULL);
#define PRERROR1(a,b)  fprintf(stderr, "%s", RDEBUG_dbgprefix); fprintf(stderr, a, b); fflush(NULL);
#define PRERROR2(a,b,c)  fprintf(stderr, "%s", RDEBUG_dbgprefix); fprintf(stderr, a, b, c); fflush(NULL);

#ifdef FN_DEBUG
#define FN_IN_DEBUG(a) fprintf(stderr, "%sentering %s\n", hostname, a); fflush(stderr);
#define FN_OUT_DEBUG(a) fprintf(stderr, "%sleaving %s\n", hostname, a); fflush(stderr);
#else
#define FN_IN_DEBUG(a) 
#define FN_OUT_DEBUG(a)
#endif
#endif

