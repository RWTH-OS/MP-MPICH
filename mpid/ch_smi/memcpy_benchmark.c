/* $Id$ */

/* Test performance of memcpy functions implemented in ch_smi.
   compile:
   gcc -o memcpy_benchmark memcpy_benchmark.c -lch_smi -L../../lib 
*/

#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif


#define PKT_START (1)
#define MAX_PKT (64*1024*1024)

void* _mpid_smi_mmx_memcpy(void *d, const void* s, size_t l);
void* _mpid_smi_mmx32_memcpy(void *d, const void* s, size_t l);
void* _mpid_smi_mmx64_memcpy(void *d, const void* s, size_t l);
void* _mpid_smi_sse32_memcpy(void *d, const void* s, size_t l);
void* _mpid_smi_sse64_memcpy(void *d, const void* s, size_t l);
void* _mpid_smi_wc32_memcpy(void *d, const void* s, size_t l);
void* _mpid_smi_wc64_memcpy(void *d, const void* s, size_t l);
/* void* _mpid_smi_alpha_memcpy(void *d, const void* s, size_t l); */
void* _mpid_smi_mmx_prefetchnta_memcpy(void *d, const void* s, size_t l);

typedef void* (*pt2Function)(void *d, const void* s, size_t l);
#define MAX_MEMCPY_FUNCTIONS 9
pt2Function memcpy_func[MAX_MEMCPY_FUNCTIONS] = {
    memcpy,
    _mpid_smi_mmx_memcpy,
    _mpid_smi_mmx32_memcpy,
    _mpid_smi_mmx64_memcpy,
    _mpid_smi_sse32_memcpy,
    _mpid_smi_sse64_memcpy,
    _mpid_smi_wc32_memcpy,
    _mpid_smi_wc64_memcpy,
/*     _mpid_smi_alpha_memcpy, */
    _mpid_smi_mmx_prefetchnta_memcpy
};


/* calculate the elapsed time between two t1 and t2 in microseconds */
static double elapsed_time(struct timeval* t1, struct timeval* t2)
{
    return(((double)(t2->tv_sec - t1->tv_sec))*1000000.0
	   +  (double)(t2->tv_usec - t1->tv_usec));
}


int main(int argc, char **argv) {
    double transmit_time;
    struct timeval t1;
    struct timeval t2;
    int retries = 1;
    int j, size;
    char *buf1, *buf2;
    int cpy_version;

    if ( (buf1 = (char*)malloc(MAX_PKT)) == NULL ) {
	printf("error: malloc()\n");
	return -1;
    }
    if ( (buf2 = (char*)malloc(MAX_PKT)) == NULL ) {
	printf("error: malloc()\n");
	return -1;
    }

    for (cpy_version = 0; cpy_version<MAX_MEMCPY_FUNCTIONS; cpy_version++) {
	printf ("memcpy nbr.: %i\n", cpy_version);
	printf ("[packet size] [lateny in usec] [bandwidth Mb/s]\n");
	for (size = PKT_START; size <= MAX_PKT; size *= 2) {
	    double timesum=0;
	    for (j = 0; j < retries; j++) {
		memset(buf1, 0, MAX_PKT);
		memset(buf2, 0, MAX_PKT);
		gettimeofday(&t1, NULL);
		memcpy_func[cpy_version](buf1, buf2, size);
		gettimeofday(&t2, NULL);
		transmit_time =  elapsed_time(&t1, &t2);
		timesum+=transmit_time;
	    }
	    
	    printf("%11d %17.2f %16.3f\n", size, timesum/retries, size*retries/timesum*1000000/1024/1024);
	    timesum=0;
	}
    }

    free(buf1);
    free(buf2);
    return 0;
}
