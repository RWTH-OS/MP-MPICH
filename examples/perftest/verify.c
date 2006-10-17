/* $Id$ */

/* verify data for mpptest */

#ifndef WIN32
 
/* not necessary when using Microsoft Visual Studio */
#include <string.h>

#endif

#include <stdio.h>
#ifdef WIN32
#include <memory.h>
#endif
#include <sys/types.h>

#define FIX_VALUE  17
#define MULT_VALUE 7
#define MOD_VALUE  3915733

int do_verify = 0;

void init_rbuffer (void *buf, size_t len) 
{
    if (do_verify) {
	memset (buf, 0, len);
    }
    return;
}

void init_sbuffer (void *buf, size_t len)
{
    int *intbuf = (int *)buf, i;

    if (do_verify) {
	/* first set all bytes != zero, then write a pattern into the 
	   the first len/sizeof(int) integers */
	memset (buf, FIX_VALUE, len);
	for (i = 0; i < len/sizeof(int); i++)
	    intbuf[i] = (FIX_VALUE + MULT_VALUE*i) % MOD_VALUE;
    }
    return;
}

int check_rbuffer (void *buf, size_t len)
{
    int *intbuf = (int *)buf, i;
    char *charbuf, nbr_chars;
    int nbr_errs = 0;

    if (do_verify) {
	for (i = 0; i < len/sizeof(int); i++) {
	    if (intbuf[i] != (FIX_VALUE + MULT_VALUE*i) % MOD_VALUE) {
#if 0
		fprintf (stderr, "recv error: expected %d, got %d\n", 
			 (FIX_VALUE + MULT_VALUE*i) % MOD_VALUE, intbuf[i]);
#endif
		nbr_errs++;
	    }
	}
	
	nbr_chars = len - (len/sizeof(int))*sizeof(int);
	if (nbr_chars > 0) {
	    charbuf = (char *)(intbuf + len/sizeof(int));
	    for (i = 0; i < nbr_chars; i++)
		if (charbuf[i] != FIX_VALUE)
		    nbr_errs++;
	}
	/* re-init for the next iteration */
	init_rbuffer (buf, len);
    }
    
    return (nbr_errs);
}
