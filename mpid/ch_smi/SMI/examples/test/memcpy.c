/* $Id$
   test the low-level PIO memory performance: compare the standard
   memcpy() and the custom _smi_mmx_memcpy() (MMX-optimized) and 
   _smi_wc_memcpy() (optimized for write-combining in x86 CPUs) for
   local and remote memory.

   Note: _smi_mmx_memcpy() and _smi_wc_memcpy() are non-public SMI functions.
   They have the same prototypes as memcpy() - but use them at your own risk!
*/

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#include "smi.h"

/* these presets may be altered for experiments */
#define LOOPS    1000
#define MAX_SIZE (2048*1024)
#define DISALIGN 0       /* disalign memory bounds  ? */
#define FLUSH    0       /* flush cache before each memcpy() ? */
#define VERIFY   0       /* verify correctness */
#define INC_OP   *= 2    /* buffer size increment operation */

#define FLUSH_SIZE (2048*1024)

#if DISALIGN
#define SRC src+1
#define DST dst+1
#else
#define SRC src
#define DST dst
#endif

#define LOCAL  1
#define REMOTE 0

#define SIGNAL 1

#if 0

void _smi_mmx_memcpy(void *dst, void *src, int n);
void _smi_wc_memcpy(void *dst, void *src, int n);

/* this function is actually used for comparison with memcpy() */
#define REMOTE_MEMCPY _smi_mmx_memcpy
#define REMOTE_MEMCPY_NAME "_smi_mmx_memcpy()"

#else

#define REMOTE_MEMCPY(_d, _s, _size) SMI_Memcpy( _d, _s, _size, SMI_MEMCPY_LP_RS );
#define REMOTE_MEMCPY_NAME "SMI_Memcpy()"

#endif

int main (int argc, char *argv[]) 
{
	smi_region_info_t smi_region;
	double sys_time, custom_time;
	size_t size;
	int nprocs, myrank, mode, shreg_id;
	int loops, min_size, max_size, i, j, l, errors;
	char *src, *dst, *loc_dst, *rmt_dst, *s, *d;
	int *flush;

	SMI_Init (&argc, &argv);
	SMI_Proc_size(&nprocs);
	SMI_Proc_rank(&myrank);

	if (argc == 4) {
		min_size = atoi(argv[1]);
		max_size = atoi(argv[2]);
		loops = atoi(argv[3]);
	} else {
		min_size = 1;
		max_size = MAX_SIZE;
		loops = LOOPS;
	}

#if FLUSH
	flush = (int *)malloc(FLUSH_SIZE);
	printf ("# copying through a COLD cache\n");
#else
	printf ("# copying through a HOT cache\n");
#endif
	
#if DISALIGN
	printf( "# data is DISALIGNED\n" );
#else
	printf( "# data is NOT DISALIGNED\n" );
#endif

#if VERIFY
	printf( "# correctness is VERIFIED\n" );
#else
	printf( "# correctness is NOT VERIFIED\n" );
#endif

	printf( "# Custom copy function: %s\n", REMOTE_MEMCPY_NAME );

	fflush( stdout );

	src = (char *)malloc(FLUSH_SIZE);
	s = SRC;
	dst = 0;
	smi_region.size   = FLUSH_SIZE;
	smi_region.owner  = REMOTE;
	smi_region.offset = 0;
	smi_region.adapter = SMI_ADPT_DEFAULT;

	if (myrank == LOCAL) {
		for (mode = 0; mode < nprocs; mode++) {
			switch (mode) {
			case LOCAL:
#if 1
				smi_region.owner  = LOCAL;
				SMI_Create_shreg(SMI_SHM_LOCAL, &smi_region, &shreg_id, (void **)&loc_dst);
				src = loc_dst;
#else
				loc_dst = (char *)malloc(FLUSH_SIZE);
#endif
				dst = loc_dst;
				d = DST;
				printf ("# local memory\n");
				fflush( stdout );
				break;
			case REMOTE:
#if SIGNAL
				SMI_Signal_send (REMOTE|SMI_SIGNAL_ANY);
#else
				SMI_Barrier();
#endif
				smi_region.owner  = REMOTE;
				SMI_Create_shreg(SMI_SHM_UNDIVIDED, &smi_region, &shreg_id, (void **)&rmt_dst);
				dst = rmt_dst;
				d = DST;
				printf ("# remote memory 0x%x\n", dst);
				fflush( stdout );
				break;
			}

			printf ("#    size \t\tsrc \t\tdst \t\t memcpy() [us] \t\t[MB/s]\t REMOTE_MEMCPY [us]\t[MB/s]\t errors\n");
			for (size = min_size; size <= max_size; size INC_OP) {
	    
#if !FLUSH
				/* warm up */
				memcpy (DST, SRC, size);
				memcpy (DST, SRC, size);
#endif
	    
				/* system memcpy() */
				sys_time = SMI_Wtime();
				for (l = 0; l < loops; l++) {
#if FLUSH
					s += (size < 128 ? 128 : size);
					if ((unsigned int)s + size - (unsigned int)src > FLUSH_SIZE)
						s = src;
					d += (size < 128 ? 128 : size);
					if ((unsigned int)d + size - (unsigned int)dst > FLUSH_SIZE)
						d = dst;
#endif
					memcpy (d, s, size);
				}
				sys_time = SMI_Wtime() - sys_time;
	    
#if VERIFY
				errors = 0;
#else
				errors = -1;
#endif
				/* custom memcpy function */
				custom_time = SMI_Wtime();
				for (l = 0; l < loops; l++) {
#if FLUSH
					s += (size < 128 ? 128 : size);
					if ((unsigned int)s + size - (unsigned int)src > FLUSH_SIZE)
						s = src;
					d += (size < 128 ? 128 : size);
					if ((unsigned int)d + size - (unsigned int)dst > FLUSH_SIZE)
						d = dst;
#endif
#if VERIFY
					/* prepare src and dst buffers */
					for (i = 0; i < size; i++) {
						(d)[i] = 0;
						(s)[i] = 170;  /* binary: 10101010 */
					}
#endif

					REMOTE_MEMCPY (d, s, size);

#if VERIFY
					for (i = 0; i < size; i++) {
						if ((d)[i] != (s)[i])
							errors++;
					}
#endif
				}
				custom_time = SMI_Wtime() - custom_time;
	    
				printf ("%9d \t %p \t %p \t %10.3f\t\t %10.3f \t %10.3f\t %10.3f \t %d \n",
						size, SRC, DST, 
						sys_time*1e6/loops, ((double)size*loops)/(1024*1024*sys_time),
						custom_time*1e6/loops, ((double)size*loops)/(1024*1024*custom_time), 
						errors);
	    
			} /* for (size = min_size; size <= max_size; size INC_OP) */
		} /* for (mode = 0; mode < nprocs; mode++) */

#if SIGNAL	
		SMI_Signal_send (REMOTE|SMI_SIGNAL_ANY);
#else
		SMI_Barrier();
#endif

	} /* if (myrank == LOCAL) */
	else {

#if SIGNAL	
		SMI_Signal_wait (SMI_SIGNAL_ANY);
#else
		SMI_Barrier();
#endif

		SMI_Create_shreg(SMI_SHM_UNDIVIDED, &smi_region, &shreg_id, (void **)&dst);

#if SIGNAL	
		SMI_Signal_wait (SMI_SIGNAL_ANY);
#else
		SMI_Barrier();
#endif

	}
    
#if 0
	free (src);
	free (loc_dst);
#endif
	SMI_Finalize();
	return 0;
}

/*
 * XEmacs/vim will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:tw=0:ts=4:wm=0:
 */
