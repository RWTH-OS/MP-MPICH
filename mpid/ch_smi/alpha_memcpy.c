/* $Id$ 

   Optimized memcpy function for Alpha Processor Platform. 
   Initially taken from /usr/src/linux/arch/alpha/lib/memcpy.c */

#include <sys/types.h>

/*
 * This should be done in one go with ldq_u*2/mask/stq_u. Do it
 * with a macro so that we can fix it up later..
 */
#define ALIGN_DEST_TO8(d,s,n) \
        while (d & 7) { \
                if (n <= 0) return; \
                n--; \
                *(char *) d = *(char *) s; \
                d++; s++; \
        }

/*
 * This should similarly be done with ldq_u*2/mask/stq. The destination
 * is aligned, but we don't fill in a full quad-word
 */
#define DO_REST(d,s,n) \
        while (n > 0) { \
                n--; \
                *(char *) d = *(char *) s; \
                d++; s++; \
        }

/*
 * This should be done with ldq/mask/stq. The source and destination are
 * aligned, but we don't fill in a full quad-word
 */
#define DO_REST_ALIGNED(d,s,n) DO_REST(d,s,n)

/*
 * This does unaligned memory copies. We want to avoid storing to
 * an unaligned address, as that would do a read-modify-write cycle.
 * We also want to avoid double-reading the unaligned reads.
 *
 * Note the ordering to try to avoid load (and address generation) latencies.
 */
static inline void _mpid_smi_alpha_memcpy_unaligned(unsigned long d, unsigned long s, long n)
{
        ALIGN_DEST_TO8(d,s,n);
        n -= 8;                 /* to avoid compare against 8 in the loop */
        if (n >= 0) {
                unsigned long low_word, high_word;
                __asm__("ldq_u %0,%1":"=r" (low_word):"m" (*(unsigned long *) s));
                do {
                        unsigned long tmp;
                        __asm__("ldq_u %0,%1":"=r" (high_word):"m" (*(unsigned long *)(s+8)));
                        n -= 8;
                        __asm__("extql %1,%2,%0"
                                :"=r" (low_word)
                                :"r" (low_word), "r" (s));
                        __asm__("extqh %1,%2,%0"
                                :"=r" (tmp)
                                :"r" (high_word), "r" (s));
                        s += 8;
                        *(unsigned long *) d = low_word | tmp;
                        d += 8;
                        low_word = high_word;
                } while (n >= 0);
        }
        n += 8;
        DO_REST(d,s,n);
}

/*
 * Hmm.. Strange. The __asm__ here is there to make gcc use an integer register
 * for the load-store. I don't know why, but it would seem that using a floating
 * point register for the move seems to slow things down (very small difference,
 * though).
 *
 * Note the ordering to try to avoid load (and address generation) latencies.
 */
static inline void _mpid_smi_alpha_memcpy_aligned(unsigned long d, unsigned long s, long n)
{
        ALIGN_DEST_TO8(d,s,n);
        n -= 8;
        while (n >= 0) {
                unsigned long tmp;
                __asm__("ldq %0,%1":"=r" (tmp):"m" (*(unsigned long *) s));
                n -= 8;
                s += 8;
                *(unsigned long *) d = tmp;
                d += 8;
        }
        n += 8;
        DO_REST_ALIGNED(d,s,n);
}


void _mpid_smi_alpha_memcpy(void * dest, const void *src, size_t n)
{
#if 1
        if (!(((unsigned long) dest ^ (unsigned long) src) & 7)) {
                _mpid_smi_alpha_memcpy_aligned((unsigned long) dest, (unsigned long) src, n);
                return;
        }
        _mpid_smi_alpha_memcpy_unaligned((unsigned long) dest, (unsigned long) src, n);
        return;
#else
	/* XXX experimental */
	int loop = n/8;
	int i;
	for (i = 0; i < loop; i++) {
	  ((long *)dest)[i] = ((long *)src)[i];
	}
	return;
#endif
}
