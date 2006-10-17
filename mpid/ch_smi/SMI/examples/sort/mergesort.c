/* $Id$
   A parallel sort algorithm based on merge-split sort 
*/

#include <stdlib.h>
#include <stdio.h>

#include "smi.h"

#define VECTOR_SIZE (2*3*5*7*256)
#define VERBOSE 1

#if VERBOSE
#define ECHO(text) fprintf(stderr,"%s\n",text); fflush(stderr);
#define ECHO1(text, int1) fprintf(stderr,"%s %d\n",text, int1); fflush(stderr);
#define ECHO2(text, int1, int2) fprintf(stderr,"%s %d %d\n",text, int1, int2); fflush(stderr);
#else
#define ECHO(text) 
#define ECHO1(text, int1) 
#define ECHO2(text, int1, int2) 
#endif

int             sgmt_size;
int             nbr_elements;
int            *swap;
int            *sgmt;
int            *sgmt2;
int            sgmt_id;
int            sgmt_id2;
int             nbr_procs;
int             my_rank;


static int compare_int(int *a, int *b)
{ 
  int result;
  result = ((*a) - (*b));
  return(result);
}

static void merge(int phase)
{
    int             i, j, k;
    int iType;
    
    iType = (my_rank + phase) % 2;
    if (iType == 1) {
      if (my_rank == 0)
	iType = 2;
    }
    else {
      if (my_rank == nbr_procs - 1)
	iType = 2;
    }

    if (iType == 2) {
      SMI_Barrier();
      return;
    }
	
    ECHO1("This is process", my_rank);
    ECHO("...Sending memory to partner...");
    
    if (iType == 0) {
      for (k=0; k < sgmt_size; k++) {
	sgmt2[(my_rank + 1) * sgmt_size + k] = sgmt[my_rank * sgmt_size + k];
      }
      SMI_Barrier();
    }
    else{
      for (k=0; k < sgmt_size; k++) {
	sgmt2[(my_rank - 1) * sgmt_size +k] = sgmt[my_rank * sgmt_size + k];
      }
      SMI_Barrier();
    }

    ECHO("...Merging...");
    if (iType == 0) { 
      i = 0; 
      j = 0;
      for (k = 0; k < sgmt_size; k++) {
	if (sgmt[my_rank * sgmt_size + i] < sgmt2[my_rank *sgmt_size + j])
	  swap[k] = sgmt[my_rank * sgmt_size + (i++)];
	else
	  swap[k] = sgmt2[my_rank * sgmt_size + (j++)];
      }
    }
    else {
      i = sgmt_size-1;
      j = sgmt_size-1;
      for (k = sgmt_size-1; k >= 0; k--) {
	if (sgmt[my_rank * sgmt_size + i] > sgmt2[my_rank * sgmt_size + j])
	  swap[k] = sgmt[my_rank * sgmt_size + (i--)];
	else
	  swap[k] = sgmt2[my_rank * sgmt_size + (j--)];
      }
    }
    
    for (k = 0; k < sgmt_size; k++)
      sgmt[my_rank * sgmt_size + k] = swap[k];
    
    ECHO("...done");
}

int main(int argc, char **argv)
{
    smi_region_info_t regdesc;
    smi_error_t         error;
    int             i, j, comp;
    double	    t0,t1;

    error = SMI_Init(&argc, &argv);
    if (error != SMI_SUCCESS) {
	ECHO ("SMI_Init() failed");
	exit (-1);
    }

    SMI_Proc_size(&nbr_procs);
    SMI_Proc_rank(&my_rank);

    nbr_elements = VECTOR_SIZE;
    sgmt_size = nbr_elements / nbr_procs;
    if (nbr_elements % nbr_procs != 0)
      sgmt_size++;

    swap = (int *) malloc(2 * sgmt_size * sizeof(int));
    if (swap == NULL) {
	ECHO("Not enough local memory!");
	SMI_Abort(-1);
    }

    SMI_Init_reginfo(&regdesc, sgmt_size * sizeof(int) * nbr_procs, 0, 0, SMI_ADPT_DEFAULT, 0, 0, NULL);
    regdesc.adapter = SMI_ADPT_DEFAULT;
    error = SMI_Create_shreg(SMI_SHM_BLOCKED, &regdesc, &sgmt_id, (void **) (&sgmt));
    if (error != SMI_SUCCESS) {
      fprintf(stderr,"errorcode: %x\n",error);
      ECHO("Not enough shared memory!\n");
      SMI_Abort(-1);
    }

    SMI_Init_reginfo(&regdesc, sgmt_size * sizeof(int) * nbr_procs, 0, 0, SMI_ADPT_DEFAULT, 0, 0, NULL);
    regdesc.adapter = SMI_ADPT_DEFAULT;
    error = SMI_Create_shreg(SMI_SHM_BLOCKED, &regdesc, &sgmt_id2, (void **) &(sgmt2));
    if (error != SMI_SUCCESS) {
      fprintf(stderr,"errorcode: %x\n",error);
      ECHO("Not enough shared memory!\n");
      SMI_Abort(-1);
    }
    
    ECHO("generating random elements...");
    for (i = 0; i < sgmt_size; i++) {
#ifdef WIN32
      sgmt[my_rank * sgmt_size + i] = rand();
#else
      sgmt[my_rank * sgmt_size + i] = lrand48();
#endif
    }

    t0 = SMI_Wtime();
    ECHO("performing qsort on this proc's elements");
    qsort(&(sgmt[my_rank * sgmt_size]), sgmt_size, sizeof(int), (int (*) (const void *, const void *)) compare_int);
    SMI_Barrier();

    ECHO("starting split-merge operation...");
    for (i = 0; i < nbr_procs - (1 - nbr_procs % 2); i++) {
	ECHO1("...performing merging step", i);
	merge(i % 2);
	SMI_Barrier();
    }

    t1 = SMI_Wtime();

    if (my_rank == 0) {
	ECHO("Testing if sort was correct");
	comp = 0;
	for (i = 0; i < nbr_procs; i++) {
	    for (j = 0; j < sgmt_size; j++) {
		if (comp > sgmt[i * sgmt_size + j]) {
		    ECHO2("process  %d, position", i, j);
		    ECHO("*** error in sort!");
		}
		comp = sgmt[i * sgmt_size + j];
		if (j % 10000 == 0) {
		    ECHO1 ("sample ", comp);
		}
	    }
	}
	fprintf(stderr, "computing time for a sort of %d integers on %d processes: %f seconds\n",
		VECTOR_SIZE, nbr_procs, t1-t0); 
    }

    ECHO("Freeing Resources...");
    SMI_Free_shreg(sgmt_id);
    SMI_Free_shreg(sgmt_id2); 
    
    SMI_Finalize();

    ECHO("END OF PROCESS");
}
