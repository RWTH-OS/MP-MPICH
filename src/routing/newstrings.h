/* $Id$ 
 * (c) 2005 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de 
 */
 
#ifndef NEWSTRINGS_H
#define NEWSTRINGS_H

/* These are functions for dynamic string allocation and copying.
 * The string returned has always the exact needed size and has to be
 * free()'d.
 */

char *newString(char* src);
char *newStringSize(char* old, size_t size);
char *newStringAdd(char* s1, char* s2);
char *newStringAdd3(char* s1, char* s2, char* s3);
char *newStringAdd4(char* s1, char* s2, char* s3, char* s4);
char *stringAppend(char* s1, const char* s2);
char *stringAppend3(char* s1, const char* s2, const char *s3);
char *stringAppend4(char* s1, const char* s2, const char *s3, const char *s4);

/*
 * ALLOC_SPRINTF sprintf's in a allocated char buffer. This buffer is reallocated
 * to the exact size of the produced string. A _must_ be a char pointer allocated by an alloc!
 */

#define ALLOC_SPRINTF1( A,F, P1 ) { int len=snprintf(A,0,F,P1); A=realloc(A, sizeof(char)*(len+1)); \
 					snprintf(A,len+1,F,P1); }
#define ALLOC_SPRINTF2( A,F, P1 , P2) { int len=snprintf(A,0,F,P1, P2); A=realloc(A, sizeof(char)*(len+1)); \
 					snprintf(A,len+1,F,P1, P2); }
#define ALLOC_SPRINTF3( A,F, P1 , P2, P3) { int len=snprintf(A,0,F,P1, P2, P3); A=realloc(A, sizeof(char)*(len+1)); \
 					snprintf(A,len+1,F,P1, P2, P3); }
#define ALLOC_SPRINTF4( A,F, P1 , P2, P3, P4) { int len=snprintf(A,0,F,P1, P2, P3, P4); A=realloc(A, sizeof(char)*(len+1)); \
 					snprintf(A,len+1,F,P1, P2, P3, P4); }
#define ALLOC_SPRINTF5( A,F, P1, P2, P3, P4, P5 ) { int len=snprintf(A,0,F,P1, P2, P3, P4, P5); A=realloc(A, sizeof(char)*(len+1)); \
 					snprintf(A,len+1,F,P1, P2, P3, P4, P5); }
					
#endif /* NEWSTRINGS_H */
