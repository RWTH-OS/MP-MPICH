/* $Id$ */

#include <string.h>
#include <stdlib.h>

#include "newstrings.h"

char *newString( char* src )
{
  char *dest;
  
  if ( src == 0 ) {
    return 0;
  }

  dest = (char *)malloc( sizeof(char) * (strlen(src)+1) );
  strcpy( dest, src );

  return dest;
}


char *newStringSize( char *old, size_t size )
{
  if( old )
    old = (char *)realloc( old, size * sizeof(char) );
  else 
    old = (char *)calloc( size,  sizeof(char) );

  return old;
}


char *newStringAdd( char* s1, char* s2 )
{
  char *dest;

  dest = (char * )malloc( sizeof(char) * (strlen(s1)+strlen(s2)+1) );

  strcpy( dest,s1 );
  strcat( dest,s2 );

  return dest;
}


char *newStringAdd3( char* s1, char* s2, char* s3 )
{
  char *dest;

  dest = (char *)malloc( sizeof(char) * (strlen(s1)+strlen(s2)+strlen(s3)+1) );

  strcpy( dest, s1 );
  strcat( dest, s2 );
  strcat( dest, s3 );

  return dest;
}


char *newStringAdd4( char* s1, char* s2, char* s3, char* s4 )
{
  char *dest;

  dest = (char *)malloc( sizeof(char) * (strlen(s1)+strlen(s2)+strlen(s3)+strlen(s4)+1) );

  strcpy(dest,s1);
  strcat(dest,s2);
  strcat(dest,s3);
  strcat(dest,s4);
  
  return dest;
}


char *stringAppend( char* s1, const char* s2 )
{
  char *dest;

  dest = (char *)realloc( s1, sizeof(char) * (strlen(s1)+strlen(s2)+1) );

  strcat( dest, s2 );

  return dest;
}


char *stringAppend3( char* s1, const char* s2, const char *s3 )
{
  char *dest;

  dest = (char *)realloc( s1, sizeof(char) * (strlen(s1)+strlen(s2)+strlen(s3)+1) );

  strcat(dest,s2);
  strcat(dest,s3);
  
  return dest;
}


char *stringAppend4( char* s1, const char* s2, const char *s3, const char *s4 ) {
  char *dest;

  dest = (char *)realloc( s1, sizeof(char) * (strlen(s1)+strlen(s2)+strlen(s3)+strlen(s4)+1) );

  strcat(dest,s2);
  strcat(dest,s3);
  strcat(dest,s4);  

  return dest;
}
