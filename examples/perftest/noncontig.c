/* pingpong - measure effective bandwidth and latency
   this is a variant to test for non-contignous data 
   (using MPI vector type) */

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "mpi.h"


typedef enum { vector, doublestrided, complicated } testtype_t;

typedef struct {
    char *name;
    int  n_types;
    MPI_Datatype *types;
} type_list_t;

#define DEF_BL_START 8
#define DEF_BL_END 131072
#define DEF_SIZE 262144
#define DEF_STRIDE 2
#define DEF_REPEAT 100
#define DEF_FACTOR 2
#define DEF_ALIGN 1

int build_dt (testtype_t ttype,
	      char *fname,
	      type_list_t *tlist,
	      MPI_Datatype *type,
	      MPI_Datatype *contig,
	      int count,
	      int blocklen,
	      int stride,
	      int dstride,
	      int size,
	      int c_size,
	      int myrank,
	      int align)
 
{

    MPI_Datatype tmp,tmp2;
    int extend, res_size;

    int st_blens[3];
    MPI_Aint st_disps[3];
    MPI_Datatype st_types[3] = { MPI_CHAR, MPI_CHAR , MPI_UB};
    
    int i_blens[2] = { 3, 1 };
    MPI_Aint i_disps[2] = { 0, 5 };

    switch (ttype) {
    case vector:
	MPI_Type_vector (count, blocklen, stride*blocklen, MPI_CHAR, type);
	MPI_Type_commit (type);
	if ( myrank == 0 ) 
	    printf ("%8d %8d %8d ",count,blocklen,0);
	break;
    case doublestrided:
	MPI_Type_vector (count/dstride, blocklen, stride*blocklen, MPI_CHAR, &tmp);
	MPI_Type_commit (&tmp);
	
	MPI_Type_extent (tmp, &extend);
	
	MPI_Type_vector (dstride, 1, stride, tmp, type);
	MPI_Type_commit (type);
	MPI_Type_free (&tmp);
	if ( myrank == 0 ) 
	    printf ("%8d %8d %8d ",count,blocklen,0);
	break;
    case complicated:
	st_disps[0] = 0;
	st_disps[1] = (c_size*(stride+1))+align-((c_size*(stride+1))%align);
	st_disps[2] = st_disps[1]+blocklen-c_size+align-((st_disps[1]+blocklen-c_size)%align)+1; 
	st_blens[0] = c_size;
	st_blens[1] = blocklen; 
        st_blens[2] = 1;
	MPI_Type_struct (2, st_blens, st_disps, st_types, &tmp);
	MPI_Type_commit (&tmp);
	MPI_Type_indexed (2, i_blens, i_disps, tmp, &tmp2);
	MPI_Type_commit (&tmp2);
	
	MPI_Type_vector (count/4, 1, stride, tmp2, type);
	MPI_Type_commit (type);
	MPI_Type_free (&tmp);
	MPI_Type_free (&tmp2); 
	if ( myrank == 0 ) 
	    printf ("%8d %8d %8d ",count,c_size,blocklen);
	break;
    }

    MPI_Type_size (*type,&res_size);
    MPI_Type_contiguous (res_size, MPI_CHAR, contig);
    MPI_Type_commit (contig);
    
    return 0;
}

int free_dt (MPI_Datatype *non_contig, MPI_Datatype *contig, type_list_t *list) 
{
    MPI_Type_free (contig);
    MPI_Type_free (non_contig);
}

int verify_buf (MPI_Datatype type, char *src, char *dest)
{
    
    char *packbuf, *vbuf;
    int psize, extent;
    int bpacked, status, i, error;

    MPI_Type_size(type,&psize);
    MPI_Type_extent(type,&extent);

    packbuf = (void *) malloc (psize);
    vbuf = (void *) malloc (extent);

    bpacked = 0;
    status = 0;

    for (i=0;i<extent;i++)
	vbuf[i] = 0;

    MPI_Pack (src,1,type,packbuf,psize,&bpacked,MPI_COMM_WORLD);
    MPI_Unpack (packbuf,bpacked,&status,vbuf,1,type,MPI_COMM_WORLD); 
    
    error = 0;
    for (i=0;i<extent;i++)
	if (dest[i]!=vbuf[i])
	    error = 1;
    
    if (error)
	printf (" error");
    else
	printf ("    ok");

    free (packbuf);
    free (vbuf);

    return 0;
}
    
int main(int argc, char **argv) {
    MPI_Status status;
    MPI_Datatype noncontig, contig;

    int myrank, mysize;
    int size, c_size, bls, ble, count, stride, blocklen, repeats, i,j,total_extent, total_size, f, dstride;
    testtype_t testtype;
    char *buffer, *recv;
    double bandwidth, latency, wtime, start;
    char c;
    char verify;
    int align;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &mysize);

    if (mysize < 2) {
	printf ("At least 2 processes are required. Aborting.\n");
	MPI_Finalize();	
	exit (1);
    }

    /* MPI_Errhandler_create (err_abort, &errhandler);
       MPI_Errhandler_set (MPI_COMM_WORLD, errhandler);
    */

    /* default values */
    size = DEF_SIZE;
    bls = DEF_BL_START;
    ble = DEF_BL_END;
    f = DEF_FACTOR;
    stride = DEF_STRIDE;
    repeats = DEF_REPEAT;
    align = DEF_ALIGN;
    testtype = vector;
    verify = 0;

    /* args:
       -s size of data to send
       -b blocklen to start with
       -e blocklen to end with
       -f factor to increase the blocklen in each run
       -x stride as multiply of blocklen
       -r nbr of repeats
       -a alignment of blocks
    */

    while((c = getopt(argc, argv, "s:b:e:f:d:x:r:c:a:?v")) != EOF) {
	switch(c) {
	case 's':
	    size = atoi(optarg);
	    break;
	case 'b':
	    bls = atoi(optarg);
	    break;
	case 'e':
	    ble = atoi(optarg);
	    break;
	case 'f':
	    f = atoi(optarg);
	    break;
	case 'x':
	    stride = atoi(optarg);
	    break;
	case 'd':
	    dstride = atoi(optarg);
	    testtype = doublestrided;
	    break;
	case 'r':
	    repeats = atoi(optarg);
	    break;
	case 'c':
	    c_size = atoi(optarg);
	    testtype = complicated;
	    break;
	case 'a':
	    align = atoi(optarg);
            break;
	case 'v':
	    verify = 1;
	    break;
	case '?':
	    if (myrank == 0) {
		printf ("ncpingpong - test a pingpong with non-contig datatype (vector) \n\
Arguments:\n\
 -s size : overall size of DATA to send (gross amount)\n\
 -b blocklen_start : start test with this blocklen\n\
 -e blocklen_end : end test with this blocklen\n\
 -f factor  : factor to increase the blocklen in each run\n\
 -x stride  : stride between blocks, in multiply of actual blocklen\n\
 -c size    : test with a 'complicated' data type (second basic block 'size')\n\
 -d dstride : test with a doublestrided vector with double-stride 'dstride'\n\
 -r repeats : repeat the test <repeats> times\n\
 -a align   : align the blocks to <align>-borders\n\
 -v         : verify the data after pingpong\n");
	  }
	    exit (0);
	    break;
	}
    }

    /* Testing the settings... */
    if ((bls == 0) || (ble == 0) || (size == 0) || (f == 0) || (stride == 0) || (repeats == 0)) {
	if ( myrank == 0)
	    printf ("You've set some value to ZERO, that can't work...\n");
	return 1;
    }

    for (i=bls;i<=ble;i*=f) {
	if (size%i) {
	    if ( myrank == 0)
		printf ("# All blocklengths should divide size, expect problems...\n");
	    break;
	}
    }

    if (testtype==complicated) {
	while (bls <= c_size) {
	    if ( myrank == 0)
		printf ("# ADJUSTING setting: blen_start must be greater c_size (%d <= %d)!\n",bls,c_size);
	    bls *= f;
	}
	
	while (ble >= size/4) {
	    if ( myrank == 0)
		printf ("# ADJUSTING setting: blen_end must be less size/4 (%d >= %d)!\n",ble,size/4);
	    ble /= f;
	}
    }

    if ( myrank == 0) {
	
	printf ("# Running with this settings: \n\
# size:\t\t%d\n\
# blen_start:\t%d\n\
# blen_end:\t%d\n\
# factor:\t%d\n\
# stride:\t%d\n\
# repeats:\t%d\n#\n",size,bls,ble,f,stride,repeats);
	printf ("# ---> Building a ");
	switch (testtype) {
	case vector:
	    if (myrank == 0)
		printf ("simple Vector\n#\n");
	    break;
	case doublestrided:
	    if (myrank == 0)
		printf ("doublestrided Vector\n#\n");
	    break;
	case complicated:
	    if (myrank == 0) {
		printf ("complicated Type\n");
		printf ("#      => Vector of Indexed of Struct\n");
		printf ("#         (with two basic blocklengths)\n#\n");
	    }
	    break;
	}
	printf ("#  Count   BBlen1   BBlen2  Data Size Buffer Size  Contig Non-Cont Verify\n");
	printf ("#         (Bytes)  (Bytes)    (Bytes)     (Bytes)  (MB/s)   (MB/s)\n");      
	printf ("#------------------------------------------------------------------------\n");
	fflush (stdout);
    }

    if (ble > size)
	ble = size;

    for (blocklen = bls; blocklen <= ble; blocklen *= f) {

	count = size / blocklen;

	build_dt (testtype, NULL, NULL, &noncontig, &contig, count, blocklen, stride, dstride, size, c_size, myrank, align);

	MPI_Type_size (noncontig, (MPI_Aint *)&total_size);
	MPI_Type_extent (noncontig, (MPI_Aint *)&total_extent); 

	buffer = (char *)malloc (total_extent);
	recv = (char *)malloc (total_extent);
	
	fflush (stdout);

	if ( myrank == 0 ) {

	    if (total_size != size)
		printf ("%9d! %11d ",total_size,total_extent);
	    else
		printf ("%10d %11d ",total_size,total_extent);

	    fflush (stdout);

	    wtime = 0.0;
	    start = MPI_Wtime();
	    for (i = 0; i < repeats; i++) {
		MPI_Send(buffer, 1, contig, 1, 0, MPI_COMM_WORLD);
		MPI_Recv(recv, 1, contig, 1, 0, MPI_COMM_WORLD, &status);
	    }
	    wtime += (MPI_Wtime() - start);
	    bandwidth = (total_size * repeats * 2)/(wtime*1024*1024);
	    latency = wtime / (repeats * 2) * 1e+6;
	    printf ("%7.3f ", bandwidth);
	    fflush (stdout);
	    
	    for (i=0;i<total_extent;i++) {
		buffer[i] = i % 127;
		recv[i] = 0;
	    }

	    wtime = 0.0;
	    start = MPI_Wtime();
	    for (i = 0; i < repeats; i++) {
		MPI_Send(buffer, 1, noncontig, 1, 0, MPI_COMM_WORLD);
		MPI_Recv(recv, 1, noncontig, 1, 0, MPI_COMM_WORLD, &status);
	    }
	    wtime += (MPI_Wtime() - start);
	    bandwidth = (total_size * repeats * 2)/(wtime*1024*1024);
	    latency = wtime / (repeats * 2) * 1e+6;
	    printf ("%8.3f ", bandwidth);

	    fflush (stdout);

	    if (verify) 
		verify_buf(noncontig, (char *) buffer, (char *) recv);

	    printf ("\n");

	    fflush (stdout);
 	} else {
	    if ( myrank == 1 ) {
		for (i = 0; i < repeats; i++) {
		    MPI_Recv(recv, 1, contig, 0, 0, MPI_COMM_WORLD, &status);
		    MPI_Send(recv, 1, contig, 0, 0, MPI_COMM_WORLD);
		}
		for (i = 0; i < repeats; i++) {
		    MPI_Recv(recv, 1, noncontig, 0, 0, MPI_COMM_WORLD, &status);
		    MPI_Send(recv, 1, noncontig, 0, 0, MPI_COMM_WORLD);
		}
	    }
	} 
	
	free_dt (&noncontig, &contig, NULL);
	free (buffer);
	free (recv);
    }
    
    MPI_Finalize();
}


