/* $Id$ 
   memcpy-function for SCI remote writes on system which have
   write-combining enabled. This code was taken from the SISCI
   memcpy-functions. */

#include <string.h>

#include "env/general_definitions.h"
#include "env/smidebug.h"

static unsigned int WC_dummy = 0;

/* length of the write-combining buffer in the CPU (in integers) */
#define WC_BUFFER_LEN  8
#define DO_WC_FLUSHING 0
#define DO_DEST_ALIGN  1

#if DO_WC_FLUSHING
#ifdef __GNUC__
/* Solaris or Linux with Gnu C compiler */
#define WC_FLUSH { asm("xchg %%eax, WC_dummy" ::: "eax"); }
#elif defined WIN32
/* Win32 platform (Visual C++) */
#define WC_FLUSH { __asm {  xchg  eax, dword ptr [WC_dummy] } }
#elif (defined SOLARIS) && (defined X86)
/* Solaris x86 with Sun C compiler */
#define WC_FLUSH { asm("xchg %eax, [WC_dummy]"); }
#else
/* unknown */
#define WC_FLUSH 
#endif

#else
#define WC_FLUSH 
#endif

void *_smi_wc_memcpy (void *dest, const void *src, size_t len)
{
    DSECTION ("wc_memcpy");
    size_t j;
    volatile unsigned int *idest, *isrc;    
    volatile char *cdest, *csrc;

    cdest = (volatile char *)dest;
    csrc  = (volatile char *)src;

#ifdef DO_DEST_ALIGN
    /* align destination address to multiple of ints */
    j = sizeof(unsigned int) - ((size_t)dest % sizeof(unsigned int));
    if (j != sizeof(unsigned int)) {
	DWARNING ("unaligned destination address");
	memcpy (dest, src, j);
	WC_FLUSH;
	cdest += j;
	csrc  += j;
	len   -= j;
    }
#endif

    idest = (volatile unsigned int *)cdest;
    isrc  = (volatile unsigned int *)csrc;
    for (j = 0; j < len/sizeof(unsigned int); j++) {
#if DO_WC_FLUSHING
	/* flush the WC-buffer when it's full */
	if (j && !(j & (WC_BUFFER_LEN-1))) {
	    WC_FLUSH;
	}
#endif
	idest[j] = isrc[j];
    }

    /* copy the rest (maximum of 3 bytes) */
    if (len % sizeof(unsigned int) != 0) {
	DWARNING ("copy size not aligned");
	memcpy ((void *)&idest[j], (void *)&isrc[j], len % sizeof(unsigned int));
    }
    WC_FLUSH;

    DSECTLEAVE; return dest;
}
