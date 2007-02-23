/* 
   
   commspeedclassfinder.c
   
   When running MPI-Applications on heterogenous environments (e.g. SMP, Nodes with different interconnects, META) 
   not all possible p2p connections between processes have the same speed (in this case bandwidth).
   This benchmark will try to find groups of processes which have the same connection speed between each pair of processes in the group.
   The groups are ordered in classes which represent similar connection speeds (bandwiths).
   
*/

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <stdio.h>
#include <string.h>
#include "mpi.h"

#include "MLB_common.h"

#undef DEBUG
//#define DEB(a) {a}
#define DEB(a)

/* minimal amount of data to transfer for benchmark in byte */
#define MINTRANSFER (8 * 1024 * 1024) 

/* Size of packets used for ping-pong */
#define PACKETSIZE (64 * 1024)

/* relative tolerance for detecting speedclasses */
#define CLASSTOLERANCE MLB_HIERARCHY_TOLERANCE

static int rank;
static int size;
static int* c_table;

typedef struct t_pairspeed_ {
    int a;
    int b;
    double bw;
    int class;
    int group;
} t_pairspeed;


/* set begin */ 

/* 
   This is an simple implementation of the adt set and some of it's operations.
   It's perfect for having a very limited number of potential elements
   
   Some terminology:
   U:        The Universe U is the set of Elements a set may contain.
             In this implementation only integer values are supported.
   N:        N is an integer valuer representing the number of elements in the Universe of the set
             The Universe is defined by the set of integers (0...N-1)
   n:        n is the number of Elements the set actually contains

   This implementations memory usage for a set is O(N)
   The used internal integer array is of size N+2, where
   A[0] := N,  A[1] := n and A[i+2] := (integer i is contained ? 1 := yes, 0 := no)
*/

typedef int* t_set;

/* cleaset removes all elements from set in O(N) */
void clearset(t_set set) {
    int i;
    for(i=0; i<set[0]; i++)
	set[i+2] = 0;
    set[1] = 0;
}

/* createset allocates memory for set and clears it in O(N) */
t_set createset(int N) {
    t_set set = (t_set) malloc((N+2) * sizeof(int));
    set[0] = N;
    clearset(set);
    return set;
}

/* killset frees allocated memory for set in O(1) */
t_set killset(t_set set) {
    free(set);

    return NULL;
}

/* addset adds element to set in O(1) */
/* if elem is not in U or elem already is in set no Operation is performed */
void addset(t_set set, int elem) {
    if ((elem >= 0)&&(elem < set[0])) {
	if (!set[elem+2]) {
	    set[1]++;
	    set[elem+2] =  1;
	}
    }
}

/* delset removes an alements from set in O(1) */
/* if elem is not in set (or even not in U) no operation is performed */
void delset(t_set set, int elem) {
    if ((elem >= 0)&&(elem < set[0])) {
	if (set[elem+2]) {
	    set[1]--;
	    set[elem+2] =  0;
	}
    }
}

/* inset checks if an element is contained in set in O(1) */
/* if elem is not in set (or even not in U) it's reported as not contained */
int inset(t_set set, int elem) {
    if ((elem >= 0)&&(elem < set[0]))
	return set[elem+2];
    else
	return 0;
}

/* countset return actual number of contained elements (n) in O(1) */
int countset(t_set set) {
    return set[1];
}

/* isemptyset will report an emptyset if n is 0 in O(1)*/
int isemptyset(t_set set) {
    return(set[1] == 0);
}

/* printset will print all contained elements to stdout in O(N)*/
void printset(t_set set) {
    int i;
    if (!isemptyset(set)) {
	for(i=0; i < set[0]; i++) {
	    if (inset(set,i))
		printf("%d ", i);
	}
    }
}

/* merge will merge 2 sets in O(Na) where Na is the number of elements in the Universe of set a (Ua)*/
/* Na dominates over Nb, because the union is stored in set a, thus if having different Universes */
/* the following cases mayoccur: */
/* Na > Nb: after processing set a will contain the union between set a and set b */
/* Na < Nb: after processing set a will contain the union between set a and (set b intersected with Ua) */
/* in all cases set b will be cleared */
void merge(t_set seta, t_set setb) {
    int i;
    for(i=0; i < seta[0]; i++) {
	if (inset(setb,i))
	    addset(seta,i);
    }
    clearset(setb);
}

/* set end */

/* remove_empty_sets will remove all empty sets from the array of sets pSets, this function is used for garbage collection */
int remove_empty_sets(t_set* pSets, int num) {
    int i,j,todo;
    i = 0;
    todo = num;
    while (todo > 0) {
	if (isemptyset(pSets[i])) {
	    killset(pSets[i]);
	    num --;
	    for(j=i; j < num; j++) {
		pSets[j]=pSets[j+1];
		c_table[j] = c_table[j+1];
	    }
	}
	else {
	    i++;
	}
	todo--;
    }
    return num;
}

/* the following functions have a more specific term of use, thus in the following a set is called */
/* group (of communicating processes) in a certain speed-class */

/* automerge will first create a new group for a certain class containing 2 endpoints elem1 and elem2 */
/* Afterwards all groups of same class containing one of the 2 endpoints will be merged */
/* all groups empty after merging will be freed */
/* automerge returns the number of remaining groups */
int automerge(t_set* pSets, int num, int elem1, int elem2, int class){
    int i,last = -1;
    
    /* create a new group containing the new found elements */
    pSets[num] = createset(size);
    addset(pSets[num], elem1);
    addset(pSets[num], elem2);
    c_table[num] = class;
    num++;
    
    /* connect all groups of same class  containing at least one of the elements */
    for (i=0; i<num; i++) {
	if (c_table[i] == class) {
	    if ( (inset(pSets[i], elem1)) || (inset(pSets[i], elem2)) ) {
		if (last != -1) {
		    merge(pSets[last], pSets[i]);
		}
		else {
		    last = i;
		}
	    }
	}
    }
    
    /* remove obsolete (empty) groups */
    return remove_empty_sets(pSets, num);
}

/* insets will find out if a certain endpoint is cointained in any grup of a certain class */
/* return value is the id of a group of class class having endpoint as member or -1 if no such group is found */
int insets(t_set* pSets, int num, int elem, int class) {
    int i;
    for (i=0; i < num; i++) {
	if (c_table[i] == class) {
	    if (inset(pSets[i], elem))
		return(i);
	}
    }
    return -1;
}


/* bwcompare is used by qsort to order elements of speedpairs by bandwidth descending */
int bwcompare(const void *p1, const void *p2)
{
    double a = ((t_pairspeed *)p1)->bw;
    double b = ((t_pairspeed *)p2)->bw;
    if (a > b)
	return (-1); /* swapped for reverse order */
    if (a < b)
	return (1);
    return (0);
}


/* will findoout the bandwidth between 2 certain processes when using packetsize packetsize */
/* This function IS COLLECTIVE and has to be called by all processes with the same parameters */
/* after benchmarking the bandwidth all processes will receive the same bandwidth as return value */
double bandwidth(int proca, int procb, int packetsize) {
    MPI_Status status;
    char* dummy;
    double t1,t2,bw;
    int runs,i;
    
    dummy = (char*) malloc(packetsize * sizeof(char));
    /* Although we don't care about the contents of the buffer transferred, we initialize it to supress error messages when */
    /* using debug tools such as valgrind, which otherwise will complain about reading data with unknown contents */
    memset(dummy,0,sizeof(char)*packetsize);
    
    /* assure transferring a minimum of MINTRANSFER bytes by increasing runs dependent on packet size */
    runs = (MINTRANSFER / packetsize) + 1;
    
    t1 = MPI_Wtime();
    for(i=0; i<runs; i++) {
	
	/* Ping */
	if (rank == proca)
	    MPI_Send(dummy, packetsize, MPI_BYTE, procb, 0, MPI_COMM_WORLD);
	if (rank == procb)
	    MPI_Recv(dummy, packetsize, MPI_BYTE, proca, 0, MPI_COMM_WORLD, &status);
	
	/* Pong */
	if (rank == procb)
	    MPI_Send(dummy, packetsize, MPI_BYTE, proca, 0, MPI_COMM_WORLD);
	if (rank == proca)
	    MPI_Recv(dummy, packetsize, MPI_BYTE, procb, 0, MPI_COMM_WORLD, &status);
	
    }
    t2 = MPI_Wtime();
    
    bw = (((double)(packetsize * 2 * runs)) / (t2 - t1)) / ((double)(1024*1024)); 
    
    MPI_Bcast(&bw, 1, MPI_DOUBLE, proca, MPI_COMM_WORLD);
    
    free(dummy);
    
    return bw;
}

void MLB_Init_comm(MPI_Comm *local_comm, MPI_Comm *inter_comm, int from_file)
{
    int i,j,k,class = 0,group= 0,groups = 0,gcand= 0;
    int s_tab_size;
    double diff;
    t_pairspeed* s_table;
    t_set* g_table;
    double* bw_table;

    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    
if (!from_file)
{
    /* this is the number of communication pairs for all to all p2p communication */
    s_tab_size = (size *(size-1)) / 2;
   
    /* if there is no pair, we're finished */
    if (s_tab_size < 1)
	return;
    
    /* s_table is the table of pairspeed data */
    s_table = (t_pairspeed*) malloc(s_tab_size * sizeof(t_pairspeed));
    
    /* g_table is a table sets to find out grouping of processes, it cannot contain more groups than measures */
    g_table = (t_set*) malloc(s_tab_size * sizeof(t_set));
    
    /* c_table is a table of classes c_table[group] := class, thus cannot contain more entries than g_table */
    c_table = (int*) malloc (s_tab_size * sizeof(int));
    /* undefined classes are preset -1 */
    for(i=0; i<s_tab_size; i++) {
	c_table[i] = -1;
    }
    
    /* bw_table is a table of bandwidths assosiated to classes, it cannot contain more classes than measures */
    bw_table = (double*) malloc (s_tab_size * sizeof(double));
    for(i=0; i<s_tab_size; i++) {
	/* undefined bandwidths are preset 0 */
	bw_table[i] = 0;
    }
    
    /* get the bandwidths of the (P(P-1))/2 possible p2p communication pairs */  
    for (i=0,k=0; i<size; i++) {
	for(j=i+1; j<size; j++,k++) {
	    s_table[k].a = i;
	    s_table[k].b = j;
	    s_table[k].bw = bandwidth(i,j,PACKETSIZE);

#ifdef DEBUG
	    if (rank == 0) {
		printf("bw %d <-> %d : %.2f MB/s\n",
		       s_table[k].a,
		       s_table[k].b,
		       s_table[k].bw);
	    }
#endif
	    MPI_Barrier(MPI_COMM_WORLD);
	}
    }

    /* sort speedpairs by bandwidth */
    qsort((void *)s_table, s_tab_size, sizeof(t_pairspeed), bwcompare);
    
    /* find speed-classes and create group sets of communicating processes in a certain speed-class */
    if (rank == 0) {
	class = 0;
	bw_table[class] = s_table[0].bw;
	
	groups = automerge(g_table, 0, s_table[0].a, s_table[0].b, class);
	s_table[0].class = class;
	
	for (k=1; k < s_tab_size; k++) {
	    diff =  s_table[k-1].bw / s_table[k].bw;
	    if ((diff < (1-CLASSTOLERANCE)) || ( diff > (1+CLASSTOLERANCE))) {
		bw_table[++class] = s_table[k].bw;
	    }
	    
	    groups = automerge(g_table, groups, s_table[k].a, s_table[k].b, class);
	    s_table[k].class = class;
	}
    }
    
#ifdef DEBUG
    if (rank == 0) {
	
	/* assign group numbers to benchmark  pairs */
	for (k=0; k < s_tab_size; k++) {
	    group = -1;
	    class = s_table[k].class;
	    gcand = insets(g_table, groups, s_table[k].a,class);
	    if (gcand != -1)
		group = gcand;
	    else {
		gcand = insets(g_table, groups, s_table[k].b,class);
		if (gcand != -1)
		    group = gcand;
	    }
	    s_table[k].group = group;
	}
	
	/* print all measured benchmark pairs */
	for (k=0; k < s_tab_size; k++) {
	    printf("bw %d <-> %d (c:%d) (g:%d): %.2f MB/s\n",
		   s_table[k].a,
		   s_table[k].b,
		   s_table[k].class,
		   s_table[k].group,
		   s_table[k].bw);
	}
    }
#endif

    /* print found classes, groups */
    if (rank == 0) {
        printf("\n");
	for (k=0; k < groups; k++) {
	    printf("MLB: class %d (%.2f MB/s), group %d ( ", c_table[k], bw_table[c_table[k]], k);
	    printset(g_table[k]);
	    printf(")\n");
	}
    }
}

    /****************************************************/

    {
      int i, j, k, l;

      double tmp;
      int ab_flag;
      int best_pair;
      int meta_group_A, meta_group_B;
      int meta_leader_A = -1, meta_leader_B = -1;
      int *keybuf; //defines new ranks for new comms
      int *colorbuf; //defines which MPI_COMM_WORLD ranks belong to the 2 classes

    keybuf = (int*)malloc(size*sizeof(int));
    colorbuf = (int*)malloc(size*sizeof(int));

  if (!from_file)
  {
      if(rank == 0)
      {
	for(i=0; i < groups; i++)
	{
	  for(j=0; j < groups; j++)
	  {
	    l=0;
	    for(k=0; k < size; k++)
	    {
	      if( (inset(g_table[i],k)) ^ (inset(g_table[j],k)) ) l++;
	    }
	    if(l==size)
	    {
	      printf("MLB: found meta groups: %d / %d\n", i, j);
	      meta_group_A = i;
	      meta_group_B = j;
	      i = groups;
	      j = groups;
	    }
	  }
	}

	tmp = 0.0;
	best_pair = -1;
	ab_flag = 0;
	for(k=0; k < s_tab_size; k++)
	{
	  if((inset(g_table[meta_group_A], s_table[k].a)) && (inset(g_table[meta_group_B], s_table[k].b)))
	  {
	    ab_flag = 0;
	    if(s_table[k].bw>tmp)
	    {
	      tmp = s_table[k].bw;
	      best_pair = k;
	    }
	  }
	  if((inset(g_table[meta_group_A], s_table[k].b)) && (inset(g_table[meta_group_B], s_table[k].a)))
	  {
	    ab_flag = 1;
	    if(s_table[k].bw>tmp)
	    {
	      tmp = s_table[k].bw;
	      best_pair = k;
	    }
	  }

	}
	if(best_pair<0)
	{
	  printf("MLB-ERROR: Best Pair = -1\n"); fflush(stdout);
	  MPI_Abort(MPI_COMM_WORLD, 0);
	}

	if(ab_flag == 0)
	{
	  meta_leader_A = s_table[best_pair].a;
	  meta_leader_B = s_table[best_pair].b;
	}
	else
	{
	  meta_leader_A = s_table[best_pair].b;
	  meta_leader_B = s_table[best_pair].a;
	}

	printf("MLB: best inter-comm pair is:  %d <-> %d\n", meta_leader_A, meta_leader_B);
      } /* if rank==0 */

     MPI_Bcast(&meta_leader_A, 1, MPI_INT, 0, MPI_COMM_WORLD);
     MPI_Bcast(&meta_leader_B, 1, MPI_INT, 0, MPI_COMM_WORLD);

    }
  else if (from_file)
  {
	char hostname_read[MPI_MAX_PROCESSOR_NAME];
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	char* result = NULL;
	int namelen;
	FILE *f = NULL;
	char fname[80];
	char leader = 'X';
    int color;
	
	
	MPI_Get_processor_name(processor_name, &namelen);

	for (i = 1; i <= 2; i++)
	{
		k = 1;
		sprintf(fname, "mh%d.txt", i);
		f = fopen(fname, "r");
		if( f == NULL ) {
   			fprintf(stderr, "[%d] Could not open file %s for reading processor naming information.\n", rank, fname);
   			break; //Error
		}
		while ((fscanf(f, "%s\n", hostname_read)) != EOF )
		{
			/* check if hostname_read is in processor_name, or vice versa */
			if (strlen(hostname_read) > strlen(processor_name))
				result = strstr(hostname_read, processor_name);
			else
				result = strstr(processor_name, hostname_read);
	
			if (result){
				DEB(printf("[%d] (%s) Found myself\n", rank, processor_name);)
				color = i-1; /* I am in group i-1 */
				break;
			}
			k++;
		}
		fclose(f);
		if (result)
			break;
	}
	
	/* gather information about which process is in which group (color) */
	MPI_Allgather(&color, 1, MPI_INT, colorbuf, 1, MPI_INT, MPI_COMM_WORLD);


	DEB(if (rank == 0){\
	printf("colorbuf: ");\
	for (i = 0; i < size; i++)\
		printf("%d",colorbuf[i]);\
	printf("\n");\
	})
	
	/* look for first 0 or 1 in colorbuf[] to find the meta_leader_A, and B, respectively */ 
	for (i=size-1; i > -1; i--)
	{
		if (colorbuf[i] == 0)
			meta_leader_A = i;
		else if (colorbuf[i] == 1)
			meta_leader_B = i;
	}

	DEB(printf("meta_leader_A=%d, meta_leader_B=%d\n", meta_leader_A, meta_leader_B);)
	
  }
	
      if(rank == 0)
      {
		for(i=0; i<size; i++) keybuf[i] = 1;
		keybuf[meta_leader_A] = 0;
		keybuf[meta_leader_B] = 0;
		
		if(!from_file){	
			for(i=0; i<size; i++)
		  		if(inset(g_table[meta_group_A],i)) colorbuf[i]=0;
	  			else colorbuf[i]=1;
		}
      }
	
      MPI_Bcast(keybuf, size, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Bcast(colorbuf, size, MPI_INT, 0, MPI_COMM_WORLD);

      MPI_Comm_split(MPI_COMM_WORLD, colorbuf[rank], keybuf[rank], local_comm);

      if(colorbuf[rank] == 0)
	MPI_Intercomm_create(*local_comm, 0, MPI_COMM_WORLD, meta_leader_B, MLB_INTER_TAG, inter_comm);
      else
	MPI_Intercomm_create(*local_comm, 0, MPI_COMM_WORLD, meta_leader_A, MLB_INTER_TAG, inter_comm);
      
      free(keybuf);
      free(colorbuf);
    } 

#ifdef DEBUG
    {
      int dummy;
      char name[64];
      int local_rank, local_size;
      
      MPI_Comm_rank(*local_comm, &local_rank);
      MPI_Comm_size(*local_comm, &local_size);

      MPI_Get_processor_name(name, &dummy);

      printf("[%s] (%d) local_rank: %d / local_size=%d\n", name, rank, local_rank, local_size); fflush(stdout);
    }
#endif


    /****************************************************/

    
    /* kill all remaining created sets */
	if(!from_file)
	{    
	    for(i=0; i<groups; i++) { 
		killset(g_table[i]);
    	}
	    free(s_table);
	    free(c_table);
	    free(g_table);
	    free(bw_table);
	}
    return;
}
