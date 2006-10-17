/* $Id: smicheck.c,v 1.7 2003/07/23 15:59:54 joachim Exp $ */

#include "smicheck.h"

#ifdef MPID_SMI_CHECK_CRC16

/* the "official" CRC32 algorithm */

static unsigned long crc16_table[256];

/* Build auxiliary table for parallel byte-at-a-time CRC-16. */
void MPID_SMI_csum_crc16_init(void)
{
    unsigned long c;
    int n, k;
    
    for (n = 0; n < 256; n++) {
	c = (unsigned long) n;
	for (k = 0; k < 8; k++) {
	    if (c & 1) {
		c = 0xedb88320L ^ (c >> 1);
	    } else {
		c = c >> 1;
	    }
	}
	crc16_table[n] = c;
    }

    return;
}

unsigned int MPID_SMI_csum_crc16_gen(unsigned char *buf, int len, unsigned char id)
{
    unsigned int crc = 0xffffffffL;
    int n;
    
    /* The maximum length that will be checked is the shortbuf-size. Longer length
       mean that the length itself is corrupted. */
    if (len > MPID_SMI_SHORTSIZE)
	return 0;
    
    for (n = 0; n < len; n++) {
	crc = crc_table16[(crc ^ buf[n]) & 0xff] ^ (crc >> 8);
    }
    crc = crc_table16[(crc ^ id) & 0xff] ^ (crc >> 8);
    
    return crc ^ 0xffffffffL;
}

#elif defined MPID_SMI_CHECK_CRC32

/* the "official" CRC32 algorithm */

static unsigned long crc32_table[256];

/* Build auxiliary table for parallel byte-at-a-time CRC-32. */
void MPID_SMI_csum_crc32_init(void)
{
    int i, j;
    unsigned long c;
    
    for (i = 0; i < 256; ++i) {
	for (c = i << 24, j = 8; j > 0; --j)
	    c = c & 0x80000000 ? (c << 1) ^ CRC32_POLY : (c << 1);
	crc32_table[i] = c;
    }

    return;
}

unsigned int MPID_SMI_csum_crc32_gen(unsigned char *buf, int len, unsigned char id)
{
    unsigned char *p;
    unsigned int crc;
    MPID_STAT_ENTRY(calc_csum);

    /* The maximum length that will be checked is the shortbuf-size. Longer length
       mean that the length itself is corrupted. */
    if (len > MPID_SMI_SHORTSIZE) {
      fprintf (stderr, "[%d] illegal checksum length %d\n", MPID_SMI_myid, len);
      return 0xffffffff;
    }
    
    crc = 0xffffffff;       /* preload shift register, per CRC-32 spec */
    for (p = buf; len > 0; p += CRC32_STRIDE, len -= CRC32_STRIDE)
	crc = (crc << 8) ^ crc32_table[(crc >> 24) ^ *p];
    
    /* finally, include the msg id into the CRC calculation to make
       each CRC unique even if the control packets are identical */
    crc = (crc << 8) ^ crc32_table[(crc >> 24) ^ id];
    
    MPID_STAT_EXIT(calc_csum);
    return ~crc;            /* transmit complement, per CRC-32 spec */
}


#elif defined (MPID_SMI_CHECK_FITS)

#define UNROLL 0
#define UNROLL_THRESHOLD 32

/* from R.L. Seaman, W.D. Pence: "FITS Checksum Proposal" */
unsigned int MPID_SMI_csum_fits_gen(unsigned char *buf, int len, unsigned char id)
{
    unsigned int hi, lo, hicarry, locarry;
#if UNROLL
    unsigned int hi_u[4], lo_u[4];
#endif
    int odd_rest, l, ll, i;
    unsigned short *sbuf = (unsigned short *)buf;

    MPID_STAT_ENTRY(calc_csum);

    /* The maximum length that will be checked is the shortbuf-size. Longer length
       mean that the length itself is corrupted. */
    if (len > MPID_SMI_SHORTSIZE)
	return 0;
    
    /* first process even number of values, odd bytes are treated seperately */
    l = 2*(len / 4);
    odd_rest = len % 4;
          
#if UNROLL
    if (l > UNROLL_THRESHOLD) {
	/* inner loop to calc csum - loop unrolling should speed things up! */
	hi_u[0] = 0; hi_u[1] = 0; hi_u[2] = 0; hi_u[3] = 0;
	lo_u[0] = 0; lo_u[1] = 0; lo_u[2] = 0; lo_u[3] = 0;

	ll = l;	i = 0;
	while (ll >= 8) {
	    hi_u[0] += sbuf[i];
	    lo_u[0] += sbuf[i+1];
	    hi_u[1] += sbuf[i+2];
	    lo_u[1] += sbuf[i+3];
	    hi_u[2] += sbuf[i+4];
	    lo_u[2] += sbuf[i+5];
	    hi_u[3] += sbuf[i+6];
	    lo_u[3] += sbuf[i+7];
	    
	    i += 8; ll -= 8;
	}
	while (ll >= 4) {
	    hi_u[0] += sbuf[i];
	    lo_u[0] += sbuf[i+1];
	    hi_u[1] += sbuf[i+2];
	    lo_u[1] += sbuf[i+3];
	    
	    i += 4; ll -= 4;
	}
	while (ll > 0) {
	    hi_u[0] += sbuf[i];
	    lo_u[0] += sbuf[i+1];
	    
	    i += 2; ll -= 2;
	}
	hi = hi_u[0] + hi_u[1] + hi_u[2] + hi_u[3];
	lo = lo_u[0] + lo_u[1] + lo_u[2] + lo_u[3];
    } else 
#endif
	/* conventional loop */
	for (i = 0, hi = 0, lo = 0; i < l; i += 2) {
	    hi += sbuf[i];
	    lo += sbuf[i+1];
	}

    if (odd_rest >= 1) hi += buf[2*l] << 8;
    if (odd_rest >= 2) hi += buf[2*l + 1];
    if (odd_rest == 3) lo += buf[2*l + 2] << 8;

#if 0
    /*  special for ch_smi: add id */
    lo += id;
#endif

    /* fold carry bits in */
    hicarry = hi >> 16;
    locarry = lo >> 16;
    while (hicarry || locarry) {
	hi = (hi & 0xffff) + locarry;
	lo = (lo & 0xffff) + hicarry;
	hicarry = hi >> 16;
	locarry = lo >> 16;
    }

    MPID_STAT_EXIT(calc_csum);
    return ((hi << 16) + lo);
}

#elif defined (MPID_SMI_CHECK_INET)

/* "Internet"-checksum from RFC 1071 */
CSUM_VALUE_TYPE MPID_SMI_csum_inet_gen(unsigned int *buf, int len, unsigned char id)
{
    register long sum = 0;
    MPID_STAT_ENTRY (calc_csum);
    
    /* The maximum length that will be checked is the shortbuf-size. Longer length
       mean that the length itself is corrupted. */
    if (len > MPID_SMI_SHORTSIZE)
	return 0;
    
    while( len > 1 )  {
	/*  This is the inner loop */
	sum += *(unsigned short *)buf++;
	len -= 2;
    }
    
    /*  Add left-over byte, if any */
    if (len > 0)
	sum += *(unsigned char *)buf;
    
    /*  special for ch_smi: add id */
    sum += id;

    /*  Fold 32-bit sum to 16 bits */
    while (sum >> 16)
	sum = (sum & 0xffff) + (sum >> 16);
    
    MPID_STAT_EXIT(calc_csum);
    return ~(CSUM_VALUE_TYPE)sum;
}

int MPID_SMI_csum_inet_verify(CSUM_VALUE_TYPE csum, unsigned int *buf, int len, unsigned char id)
{
    register long sum = 0;
    MPID_STAT_ENTRY (calc_csum);
    
    while( len > 1 )  {
	/*  This is the inner loop */
	sum += *(unsigned short *)buf++;
	len -= 2;
    }
    
    /*  Add left-over byte, if any */
    if (len > 0)
	sum += *(unsigned char *)buf;
    
    /*  special for ch_smi: add id */
    sum += id;

    /* for the verification, the checksum itself needs to be involved in the 
       computation, too */
    sum += csum;

    /*  Fold 32-bit sum to 16 bits */
    while (sum >> 16)
	sum = (sum & 0xffff) + (sum >> 16);
    
    MPID_STAT_EXIT(calc_csum);
    return ((~(CSUM_VALUE_TYPE)sum) & 0xffff == 0xffff);
}
#elif defined (MPID_SMI_CHECK_NETDEV)

#if 0
/* implemented in checksum.s */

/* this code was taken from the Linux netdev mailing list - it is probably used 
   in the Linux kernel in this form or another. */

CSUM_VALUE_TYPE MPID_SMI_csum_netdev_gen(const unsigned char * buff, int len, CSUM_VALUE_TYPE sum)
 {
    MPID_STAT_ENTRY (calc_csum);
         __asm__(
            "testl $2, %%esi"
            "jnz 30f "
"10:"
            "# Do a Duff's device style jump into the unrolled loop "
            "movl %%ecx, %%edx"
            "movl %%ecx, %%ebx"
            "andl $0x7c, %%ebx"
            "shrl $7, %%ecx"
            "addl %%ebx,%%esi"
            "shrl $2, %%ebx  "
            "negl %%ebx"
            "lea 45f(%%ebx,%%ebx,2), %%ebx"
            "testl %%esi, %%esi"
            "jmp %%ebx"

            "# Handle word-aligned regions "
"20:         addw (%%esi), %%ax"
            "lea 2(%%esi), %%esi"
            "adcl $0, %%eax"
            "jmp 10b"

"30:         subl $2, %%ecx          "
            "ja 20b "                
            "je 32f"
            "movzbl (%%esi),%%ebx # csum 1 byte, 2-aligned"
            "addl %%ebx, %%eax"
            "adcl $0, %%eax"
            "jmp 80f"
"32:"
            "addw (%%esi), %%ax # csum 2 bytes, 2-aligned"
            "adcl $0, %%eax"
            "jmp 80f"
"40:" 
            "addl -128(%%esi), %%eax"
            "adcl -124(%%esi), %%eax"
            "adcl -120(%%esi), %%eax"
            "adcl -116(%%esi), %%eax"   
            "adcl -112(%%esi), %%eax"   
            "adcl -108(%%esi), %%eax"
            "adcl -104(%%esi), %%eax"
            "adcl -100(%%esi), %%eax"
            "adcl -96(%%esi), %%eax"
            "adcl -92(%%esi), %%eax"
            "adcl -88(%%esi), %%eax"
            "adcl -84(%%esi), %%eax"
            "adcl -80(%%esi), %%eax"
            "adcl -76(%%esi), %%eax"
            "adcl -72(%%esi), %%eax"
            "adcl -68(%%esi), %%eax"
            "adcl -64(%%esi), %%eax"     
            "adcl -60(%%esi), %%eax"     
            "adcl -56(%%esi), %%eax"     
            "adcl -52(%%esi), %%eax"   
            "adcl -48(%%esi), %%eax"   
            "adcl -44(%%esi), %%eax"
            "adcl -40(%%esi), %%eax"
            "adcl -36(%%esi), %%eax"
            "adcl -32(%%esi), %%eax"
            "adcl -28(%%esi), %%eax"
            "adcl -24(%%esi), %%eax"
            "adcl -20(%%esi), %%eax"
            "adcl -16(%%esi), %%eax"
            "adcl -12(%%esi), %%eax"
            "adcl -8(%%esi), %%eax"
            "adcl -4(%%esi), %%eax"
"45:"
            "lea 128(%%esi), %%esi"
            "adcl $0, %%eax"
            "dec %%ecx"
            "jge 40b"
            "movl %%edx, %%ecx"
"50:         andl $3, %%ecx"
            "jz 80f"

            "# Handle the last 1-3 bytes without jumping"
            "notl %%ecx            # 1->2, 2->1, 3->0, higher bits are masked"
            "movl $0xffffff,%%ebx  # by the shll and shrl instructions"
            "shll $3,%%ecx"
            "shrl %%cl,%%ebx"
            "andl -128(%%esi),%%ebx # esi is 4-aligned so should be ok"
            "addl %%ebx,%%eax"
            "adcl $0,%%eax"
"80: "
        : "=a"(sum)
        : "0"(sum), "C"(len), "S"(buff)
        : "bx", "cx", "dx", "si");
);

       MPID_STAT_EXIT(calc_csum);
       return(sum);
}
#endif

#endif
