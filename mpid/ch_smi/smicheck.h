/* $Id: smicheck.h,v 1.10 2002/10/28 21:17:47 joachim Exp $ */

#ifndef _MPID_SMI_CHECK
#define _MPID_SMI_CHECK

#include <stdio.h>

#include "smidef.h"
#include "smistat.h"

#define MAX_CSUM_RETRIES 1000000

/* select macros used to check for data integrity in short messages - set in smidef.h */

#define DO_CHECK(partner, len) (MPID_SMI_CHECK_SMP || MPID_SMI_is_remote[partner] && ((len) > 0))

#ifdef MPID_SMI_CHECK_NONE
/* no check is performed - for performance comparison */
#define CSUM_VALUE_TYPE unsigned int
#define CSUM_LEN_TYPE unsigned int

#define MPID_SMI_CSUM_INIT
#define MPID_SMI_CSUM_GEN(csum, buf, len, id, partner) (csum) = 1
#define MPID_SMI_CSUM_OK(csum, buf, len, id, partner, ok) (ok) = 1

#elif defined MPID_SMI_CHECK_CRC32
/* standard (real!) CRC32 check - quite slow, but very secure */
#define CSUM_VALUE_TYPE unsigned int
#define CSUM_LEN_TYPE unsigned int

#define CRC32_POLY 0x04c11db7     /* AUTODIN II, Ethernet, & FDDI */
#define CRC32_STRIDE 1            /* experimental: as the atomicity of SCI transfers is 'int' (4 bytes),
				     we do not need to include *all* bytes of a buffer in our CRC, but
				     only one byte of each int in the buffer to ensure correctness.
				     This may saves time. */

void MPID_SMI_csum_crc32_init(void);
CSUM_VALUE_TYPE MPID_SMI_csum_crc32_gen(unsigned char *buf, int len, unsigned char id);

#define MPID_SMI_CSUM_INIT MPID_SMI_csum_crc32_init()
#define MPID_SMI_CSUM_GEN(csum, buf, len, id, partner) csum = DO_CHECK(partner, len) ? \
                         MPID_SMI_csum_crc32_gen((unsigned char *)(buf), len, id) : 1;
#define MPID_SMI_CSUM_OK(csum, buf, len, id, partner, ok) ok =  DO_CHECK(partner, len) ? \
                        ((csum) == MPID_SMI_csum_crc32_gen((unsigned char *)(buf), len, id)) : 1;

#elif defined MPID_SMI_CHECK_FITS
/* quite simply unsigend-add-with-overflow - quite fast, but not as secure as CRC32 */

#define CSUM_VALUE_TYPE unsigned int
#define CSUM_LEN_TYPE unsigned int

CSUM_VALUE_TYPE MPID_SMI_csum_fits_gen(unsigned char *buf, int len, unsigned char id);

#define MPID_SMI_CSUM_INIT 
#define MPID_SMI_CSUM_GEN(csum, buf, len, id, partner) csum = DO_CHECK(partner, len) ? \
                          MPID_SMI_csum_fits_gen((unsigned char *)(buf), len, id) : 1;
#define MPID_SMI_CSUM_OK(csum, buf, len, id, partner, ok) ok = DO_CHECK(partner, len) ? \
                          ((csum) == MPID_SMI_csum_fits_gen((unsigned char *)(buf), len, id)) : 1;

#elif defined MPID_SMI_CHECK_INET
/* IP checksum (does not yet work) */
#define CSUM_VALUE_TYPE unsigned int
#define CSUM_LEN_TYPE unsigned int

CSUM_VALUE_TYPE MPID_SMI_csum_inet_gen(unsigned int *buf, int len, unsigned char id);
int MPID_SMI_csum_inet_verify(CSUM_VALUE_TYPE csum, unsigned int *buf, int len, unsigned char id);

#define MPID_SMI_CSUM_INIT 
#define MPID_SMI_CSUM_GEN(csum, buf, len, id, partner) csum = DO_CHECK(partner, len) ? \
                          MPID_SMI_csum_inet_gen((unsigned int *)(buf), len, id) : 1;
#define MPID_SMI_CSUM_OK(csum, buf, len, id, partner, ok) ok = DO_CHECK(partner, len) ? \
                          MPID_SMI_csum_inet_verify(csum, (unsigned int *)(buf), len, id) : 1;

#elif defined MPID_SMI_CHECK_NETDEV

#define CSUM_VALUE_TYPE unsigned int
#define CSUM_LEN_TYPE unsigned int

CSUM_VALUE_TYPE MPID_SMI_csum_netdev_gen(unsigned char *buf, int len, CSUM_VALUE_TYPE sum);
/* taken from the Linux kernel - net device checksum */
#define MPID_SMI_CSUM_INIT 
#define MPID_SMI_CSUM_GEN(csum, buf, len, id, partner) csum = DO_CHECK(partner, len) ? \
                          MPID_SMI_csum_netdev_gen((unsigned char *)(buf), len, 0) : 1; 
#define MPID_SMI_CSUM_OK(csum, buf, len, id, partner, ok)  ok = DO_CHECK(partner, len) ? \
                          (csum == MPID_SMI_csum_netdev_gen((unsigned char *)(buf), len, 0)): 1; 
#endif

typedef struct {
    volatile CSUM_VALUE_TYPE csum; 
    volatile CSUM_LEN_TYPE csum_len;
} MPID_CSUM_T;

#define CSUM_SIZE sizeof(CSUM_VALUE_TYPE)
#define CSUMHEAD_SIZE (sizeof(CSUM_VALUE_TYPE)+sizeof(CSUM_LEN_TYPE))


#endif
