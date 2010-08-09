/*
 * $Id$
 *
 */

#ifndef __USOCK_GETOPT_H_
#define __USOCK_GETOPT_H_

/* A getopt() function for UNIX-like command-line parsing. */

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int MPID_USOCK_getopt(int nargc, char * const *nargv, const char *ostr); 
void MPID_USOCK_resetGetOpt();

extern char *_optarg;
extern int _optind, _opterr, _optopt;

#ifdef __cplusplus
}
#endif

/* 

NAME
     getopt - get option letter from argument vector

SYNOPSIS
     #include "getopt.h"

     int getopt(int argc, char * const *argv,
          const char *optstring);

     extern char *_optarg;
     extern int _optind, _opterr, _optopt;

MT-LEVEL
     Unsafe

DESCRIPTION
     getopt() returns the next option letter in argv that matches
     a  letter  in  optstring.   It supports all the rules of the
     command syntax standard (see intro(1)).  Since all new  com-
     mands are intended to adhere to the command syntax standard,
     they should use getopts(1), getopt(3C) or  getsubopt(3C)  to
     parse  positional  parameters and check for options that are
     legal for that command.

     optstring must contain the option letters the command  using
     getopt() will recognize; if a letter is followed by a colon,
     the option is expected to have  an  argument,  or  group  of
     arguments,  which  may  be separated from it by white space.
     _optarg is set to point to the start of the  option  argument
     on return from getopt().

     getopt() places in _optind the argv index of the  next  argu-
     ment to be processed.  _optind is external and is initialized
     to 1 before the first call to getopt().   When  all  options
     have  been  processed  (that  is, up to the first non-option
     argument), getopt() returns EOF.  The special option " - - "
     (two hyphens) may be used to delimit the end of the options;
     when it is encountered, EOF  is  returned  and  " - - "'  is
     skipped.   This is useful in delimiting non-option arguments
     that begin with ``-'' (hyphen).

RETURN VALUES
     getopt() prints an error message on the standard  error  and
     returns  a "?"  (question mark) when it encounters an option
     letter not included in optstring or  no  argument  after  an
     option that expects one.  This error message may be disabled
     by setting _opterr to 0.  The value  of  the  character  that
     caused the error is in _optopt.
*/

#endif
