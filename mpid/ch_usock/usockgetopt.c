/*
 * $Id$
 *
 */

/*!
\file usockgetopt.cpp
\brief Contains the MPID_USOCK_getopt() function.

 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#define _DEBUG_EXTERN_REC
#include "mydebug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define rindex strrchr
#define index strchr
#endif

/*
 * get option letter from argument vector
 */
int	_opterr = 1,		/* if error message should be printed */
	        _optind = 1,		/* index into parent argv vector */
         	_optopt;			/* character checked for validity */
char	*_optarg;		/* argument associated with option */


#define	BADCH	(int)'?'
#define	EMSG	""

static char *place = EMSG;		/* option letter processing */

/*!
This is used by CTCPCommunicator if any following layers
want to use MPID_USOCK_getopt() as well.
*/
void MPID_USOCK_resetGetOpt() {
	_opterr=1;
	_optind=1;
	_optopt=0;
	_optarg=0;
	place=EMSG;
}

/*!
this is the well known function used to pares commandline parameters.
We ported this function to NT.
*/

int
MPID_USOCK_getopt(int nargc, char * const *nargv, const char *ostr)
{
    	DSECTION("MPID_USOCK_getopt");
	
	register char *oli;			/* option letter list index */
	char *p;

	DSECTENTRYPOINT;

	if (!*place) {				/* update scanning pointer */
		if (_optind >= nargc || *(place = nargv[_optind]) != '-') {
			place = EMSG;
			DSECTLEAVE
			    return(EOF);
		}
		if (place[1] && *++place == '-') {	/* found "--" */
			++_optind;
			place = EMSG;
			DSECTLEAVE
			    return(EOF);
		}
	}					/* option letter okay? */
	if ((_optopt = (int)*place++) == (int)':' ||
	    !(oli = index(ostr, _optopt))) {
		/*
		 * if the user didn't specify '-' as an option,
		 * assume it means EOF.
		 */
	    	if (_optopt == (int)'-') {
		    DSECTLEAVE
			return(EOF);
		}
		if (!*place)
			++_optind;
		if (_opterr) {
			if (!(p = rindex(*nargv, '/')))
				p = *nargv;
			else
				++p;
			(void)fprintf(stderr, "%s: illegal option -- %c\n",
			    p, _optopt);
		}
		DSECTLEAVE
		    return(BADCH);
	}
	if (*++oli != ':') {			/* don't need argument */
		_optarg = NULL;
		if (!*place)
			++_optind;
	}
	else {					/* need an argument */
		if (*place)			/* no white space */
			_optarg = place;
		else if (nargc <= ++_optind) {	/* no arg */
			place = EMSG;
			if (!(p = rindex(*nargv, '/')))
				p = *nargv;
			else
				++p;
			if (_opterr)
				(void)fprintf(stderr,
				    "%s: option requires an argument -- %c\n",
				    p, _optopt);
			DSECTLEAVE
			    return(BADCH);
		}
	 	else				/* white space */
			_optarg = nargv[_optind];
		place = EMSG;
		++_optind;
	}
	DSECTLEAVE
	    return(_optopt);				/* dump back option letter */
}
