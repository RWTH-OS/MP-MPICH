/* $Id$ */

/* Test memory allocation via MPI_Alloc_mem(), including some attributes
   which may or may not be supported. */

#include <stdio.h>
/*#include <unistd.h>*/
/*#include <getopt.h>*/
#include <unistd.h>

#include <mpi.h>


int main (int argc, char *argv[]) {
    MPI_Info info;
    int alignment = -1, len = 1024, shared = 0, nbr_blocks = 1;    
    int c, rank, i, j, do_test = 1;
    char value[16];
    char **ptrs;

    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);

    if (rank == 0) {
	while((c = getopt(argc, argv, "l:a:n:sp?")) != EOF) {
	    switch(c) {	
	    case 'a':
		alignment = atoi(optarg);
		break;
	    case 'l':
		len = atoi(optarg);
		break;
	    case 'n':
		nbr_blocks = atoi(optarg);
		break;
	    case 's':
		shared = 1;
		break;
	    case 'p':
		shared = -1;
		break;
	    case '?':
	      printf("%s - test memory allocation via MPI_Alloc_mem\n\
usage:\n\
 -a alignment  : align memory address accordingly (0 for auto-alignent)\n\
 -l len        : length of block to allocate (bytes)\n\
 -n number     : number of blocks to allocate\n\
 -s            : memory must be shared memory\n\
 -p            : memory must be private memory\n", argv[0]);
	      do_test = 0;
	      break;
	    }
	}
    }
	
    if ((rank == 0) && do_test ) {
	/* these attributes need not be supported by all implementations */
	MPI_Info_create (&info);
	if (alignment >= 0) {
	  sprintf(value, "%d", alignment);
	  MPI_Info_set (info, "alignment", value);
	}
	if (shared == 1) {
	  MPI_Info_set (info, "type", "shared");
	}
	if (shared == -1) {
	  MPI_Info_set (info, "type", "private");
	}
	
	ptrs = (char **)malloc (nbr_blocks * sizeof(char *));
	
	for (i = 0; i < nbr_blocks; i++) {
	  if (MPI_Alloc_mem (len, info, &ptrs[i]) != MPI_SUCCESS) {
	    fprintf (stderr, "%d: allocation of %d bytes failed.\n", i+1, len);
	    break;
	  } else {
	    printf ("%d: allocation of %d bytes returned address 0x%x\n", i+1, len, ptrs[i]); 
	    memset (ptrs[i], 0, len);
	    printf (" -> access check o.k.\n");
	  }
	}
	
	for (j = 0; j < i; j++) {
	  MPI_Free_mem (ptrs[j]);
	}
	
	free (ptrs);
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    MPI_Finalize();
    return 0;
}
   
