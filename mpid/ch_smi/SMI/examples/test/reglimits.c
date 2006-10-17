/* $Id$ */
/* reglimits.c - find the shared memory limits of the system (number and 
   size of shared memory regions)  */

#include <stdio.h>

#include "smi.h"

#define NAMELEN 256
#define MAX_NBR_SGMTS 1024

int main (int argc, char *argv[]) 
{
    smi_region_info_t reginfo;
    smi_error_t err;
    int rank, size, proc, pagesize;
    unsigned long test_size, upper_size, lower_size, nbr_pages, max_size;
    int too_big, nbr_sgmts, i;
    int shreg_id; 
    int test_shreg_id[MAX_NBR_SGMTS];
    char nodename[NAMELEN], *shreg_addr;    
    size_t len;
    
    SMI_Init(&argc, &argv);

    SMI_Proc_rank(&rank);
    SMI_Proc_size(&size);
    len = NAMELEN;
    SMI_Get_node_name(nodename, &len);
    SMI_Page_size(&pagesize);

    /* test maximum size of local SCI memory segments a process can create individually */
    for (proc = 0; proc < size; proc++) {
	if (rank == proc) {
	    printf ("*** [%d] testing for max. local segment size on node %s\nKB: ", rank, nodename);
	    fflush(stdout);
	    
	    for (test_size = pagesize, err = SMI_SUCCESS; err == SMI_SUCCESS; test_size *= 2) {
		SMI_Init_reginfo(&reginfo, test_size, 0, proc, SMI_ADPT_DEFAULT, 0, 0, NULL);
		err = SMI_Create_shreg(SMI_SHM_LOCAL, &reginfo, &shreg_id, (void **)&shreg_addr);
		
		if (err == SMI_SUCCESS) {
		    printf ("%d: ok - ", test_size/1024); fflush (stdout);
		    /* remember last working test size for later */
		    max_size = test_size;
		    SMI_Free_shreg(shreg_id);
		} else {
		    printf ("%d: no - ", test_size/1024); fflush (stdout);
		    break;
		}
	    }
	    /* Now we found a region size that is too big, and one that is half this size and 
	       can be used. We do a binary search for the maximum region size with pagesize
	       granularity. */
	    upper_size = test_size;
	    lower_size = test_size/2;
	    max_size   = lower_size; 
	    nbr_pages = (upper_size-lower_size)/pagesize;
	    test_size = lower_size + pagesize*nbr_pages/2;
	    while (nbr_pages > 1) {
		SMI_Init_reginfo(&reginfo, test_size, 0, proc, SMI_ADPT_DEFAULT, 0, 0, NULL);
		err = SMI_Create_shreg(SMI_SHM_LOCAL, &reginfo, &shreg_id, (void **)&shreg_addr);
		
		too_big = (err != SMI_SUCCESS);
		if (!too_big) {
		    printf ("%d: ok - ", test_size/1024); fflush (stdout);
		    /* remember last working test size for later */
		    max_size = test_size;
		    SMI_Free_shreg(shreg_id);
		    /* determine next test size which is bigger than this one */
		    nbr_pages = (upper_size - test_size)/pagesize;
		    lower_size = test_size;
		    test_size += pagesize*nbr_pages/2;
		} else {
		    /* determine next test size which is smaller than this one */
		    printf ("%d: no - ", test_size/1024); fflush (stdout);
		    nbr_pages = (test_size - lower_size)/pagesize;
		    upper_size = test_size;
		    test_size -= pagesize*nbr_pages/2;
		}
	    }
	    printf ("\n>>> [%d] maximum local segment size on node %s is %i kB\n", rank, nodename, max_size>>10);
	    fflush(stdout);
	}
	SMI_Barrier();
    }
    
    /* test maximum size of SCI memory segments the process can create collectively */
    for (proc = 0; proc < size; proc++) {
	if (rank == proc) {
	    printf ("\n*** [%d] testing for max. common segment size for segments located on node %s\nKB: ", rank, nodename);
	    fflush(stdout);
	} 
	
	for (test_size = pagesize, err = SMI_SUCCESS; err == SMI_SUCCESS; test_size *= 2) {
	    SMI_Init_reginfo(&reginfo, test_size, 0, proc, SMI_ADPT_DEFAULT, 0, 0, NULL);
	    err = SMI_Create_shreg(SMI_SHM_UNDIVIDED|SMI_SHM_NONFIXED, &reginfo, &shreg_id, (void **)&shreg_addr);
	    
	    if (err == SMI_SUCCESS) {
	      if (rank == proc)
	        printf ("%d: ok - ", test_size/1024); fflush (stdout);
		/* remember last working test size for later */
		max_size = test_size;
		SMI_Free_shreg(shreg_id);
	    } else {
	      if (rank == proc)
	        printf ("%d: no - ", test_size/1024); fflush (stdout);
		break;
	    }
	}
	/* Now we found a region size that is too big, and one that is half this size and 
	   can be used. We do a binary search for the maximum region size with pagesize
	   granularity. */
	upper_size = test_size;
	lower_size = test_size/2;
	max_size   = lower_size; 
	nbr_pages = (upper_size-lower_size)/pagesize;
	test_size = lower_size + pagesize*nbr_pages/2;
	while (nbr_pages > 1) {
	    SMI_Init_reginfo(&reginfo, test_size, 0, proc, SMI_ADPT_DEFAULT, 0, 0, NULL);
	    err = SMI_Create_shreg(SMI_SHM_UNDIVIDED|SMI_SHM_NONFIXED, &reginfo, &shreg_id, (void **)&shreg_addr);
	    
	    too_big = (err != SMI_SUCCESS);
	    if (!too_big) {
	      if (rank == proc)
	        printf ("%d: ok - ", test_size/1024); fflush (stdout);
		/* remember last working test size for later */
		max_size = test_size;
		SMI_Free_shreg(shreg_id);
		/* determine next test size which is bigger than this one */
		nbr_pages = (upper_size - test_size)/pagesize;
		lower_size = test_size;
		test_size += pagesize*nbr_pages/2;
	    } else {
	      if (rank == proc)
	        printf ("%d: no - ", test_size/1024); fflush (stdout);
		/* determine next test size which is smaller than this one */
		nbr_pages = (test_size - lower_size)/pagesize;
		upper_size = test_size;
		test_size -= pagesize*nbr_pages/2;
	    }
	}
	if (rank == proc) {
	    printf ("\n>>> [%d] maximum common segment size for segments located on node %s is %i kB\n", 
		    rank, nodename, max_size>>10);
	    fflush(stdout);
	}
	SMI_Barrier();
    }

    /* test maximum number of SCI memory segments a process can create on its machine */
    for (proc = 0; proc < size; proc++) {
	if (rank == proc) {
	  printf ("\n*** [%d] testing for maximum number of segments on node %s\n", rank, nodename);
	  fflush(stdout);
	
	    nbr_sgmts = 0;
	    SMI_Init_reginfo(&reginfo, pagesize, 0, proc, SMI_ADPT_DEFAULT, 0, 0, NULL);
	    for (i = 0, err = SMI_SUCCESS; (err == SMI_SUCCESS) && (nbr_sgmts < MAX_NBR_SGMTS); i++) {
		err = SMI_Create_shreg(SMI_SHM_LOCAL, &reginfo, &test_shreg_id[i], (void **)&shreg_addr);
		if (err == SMI_SUCCESS)
		    nbr_sgmts++;
	    }
	    
	    for (i = 0; i < nbr_sgmts; i++) 
		SMI_Free_shreg(test_shreg_id[i]);
	    
	    printf (">>> [%d] maximum nunber of segments on node %s is %d \n", rank, nodename, nbr_sgmts);
	    fflush(stdout);
	}
	SMI_Barrier();
    }
    
    SMI_Finalize();
    return 0;
}
