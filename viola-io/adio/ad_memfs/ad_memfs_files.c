/**************************************************************************
* MEMFS                                                                   * 
***************************************************************************
*                                                                         * 
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              * 
*                                                                         * 
* See COPYRIGHT notice in base directory for details                      * 
**************************************************************************/
/**************************************************************************
* File:         ad_memfs_files.c                                          * 
* Description:  This is the "memfs filesystem"                            *
* Author(s):    Jan Seidel <Jan.Seidel@fh-bonn-rhein-sieg.de>             * 
* 		Marcel Birkner <Marcel.Birkner@fh-bonn-rhein-sieg.de>     *      
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/  

/*============================================================================*/
/* INCLUDES */

#include "ad_memfs_files.h"

/*============================================================================*/
/* DEFINES */

/* all errors begin with -3 */
#define ERROR_NO_FILE_EXISTS -301
#define ERROR_ALLOCATING_SPACE -302
#define ERROR_FILETABLE_FULL -303

/* all file statuses begin with 1 */
#define FREE 101
#define CLOSED 102

#ifdef TIMEMEASURE
/* Defines for time measures */
#define OPEN 0
#define CLOSE 1
#define WRITE 2
#define READ 3
#define DELETE 4
#define READIO 5
#define WRITEIO 6
#define MEMCPY 7
#define RESIZE 8
#define ALLOC 9
#endif


/*  1 MB (1024 * 1024) */
/* #define DATAINDEX_SIZE 1048576  see ad_memfs.h */


/* ACCESS MODES */
#if 0
#define MPI_MODE_RDONLY              2  /* ADIO_RDONLY */
#define MPI_MODE_RDWR                8  /* ADIO_RDWR  */
#define MPI_MODE_WRONLY              4  /* ADIO_WRONLY  */
#define MPI_MODE_CREATE              1  /* ADIO_CREATE */
#define MPI_MODE_EXCL               64  /* ADIO_EXCL */
#define MPI_MODE_DELETE_ON_CLOSE    16  /* ADIO_DELETE_ON_CLOSE */
#define MPI_MODE_UNIQUE_OPEN        32  /* ADIO_UNIQUE_OPEN */
#define MPI_MODE_APPEND            128  /* ADIO_APPEND */
#define MPI_MODE_SEQUENTIAL        256  /* ADIO_SEQUENTIAL */
#endif


/*============================================================================*/
/* TYPEDEFS */

typedef struct {
    int maj;
    int min;
} memfs_version_t;

/*  1 dataIndex is 1 MB and the Information if the space is allocated */
typedef struct {
    int isAllocated;
    //int locked;
    char *data; 	/* the data (e.g. the content) of the file */
      #if 0
      data = (char *)malloc(sizeof(char) * 65563); /*  this 1 MB */
      #endif
} dataIndex; 


/*  filehandle for 1 File */
typedef struct {
    dataIndex *content; /*  points to an array of dataIndices */
    int numDataIndices; /* number of dataIndices used by the file */
    int allocatedDataIndices; /* number of dataIndices allocated for the file */

    char *filename; /*  filename */
    int status; /* slot free, allocated, closed, open for read, open for write */
    int open_count; /* number of open calls (i.e. number of nodes that called open) */

    int exclusive;  /* 1=true, 0=false; flag to allow writes/reads without locks  */   
    int* locked;
    int totalLocks;
 
    int64_t filepointer; /* current read/write position in file (<=size) */
    int64_t filesize; /* current file size (<= numDataIndices * DATAINDEX_SIZE) */
    int64_t dataindex_size; /* size of memory blocks that are allocated for this file */
    
} memfs_file_t;

/* filetable consisting of a vector of filehandles */
typedef memfs_file_t *memfs_filetable_t;


/*==========================================================================================================*/
/* VARIABLES */

/* initially the file table is empty */
static memfs_filetable_t filetable = NULL;

/*  at the beginning, there is space for 10 files. This number is increased, when more than 10 files are created. */
#define N 10

/* size of memory blocks which are allocated for files */
/* static int dataindex_size = 1000000; */

static int numfiles = 0;
static int filearraysize = N;
static int comm_rank;

#ifdef TIMEMEASURE
static int timeFunctions = 10;
static double measuredTime[10];
static int timeInitialized = 0;
#endif


/*==========================================================================================================*/
/* STATIC FUNCTIONS */

/*
    Function to measure time usage
*/
/*
double gettime(void) {
  struct timeval t;

  (void)gettimeofday(&t, NULL);
  return t.tv_sec + t.tv_usec*1e-6;
}
*/


/*
    helper function to print out the actual filetable
*/
static void printfiletable(memfs_filetable_t filetable, int size) {
    int i;
    fprintf(stderr, "content of the filetable:\n");
    fprintf(stderr, "=========================\n");

    /* Precondition:                            */
    /* size can not be smaller than 0           */
    /* filetable has to be != NULL              */
    assert(size>=0);
    assert(filetable!=NULL);

    for(i = 0; i < size; i++) {
        fprintf(stderr, "i: %d, filename: %s\n", i, filetable[i].filename);
    }
}


/* 
    resize the filetable 
*/
static memfs_filetable_t resize(memfs_filetable_t filetable) {

    memfs_filetable_t NewArray = NULL;

    #ifdef TIMEMEASURE
    	double t0, t1;
    	t0 = gettime();
    #endif

    
    /* If filetable empty then create filetable */
    if(filetable == NULL) {

      	/* Precondition:                                */
       	/* sizeof(memfs_file_t) larger than 0           */
       	assert( (sizeof(memfs_file_t)) > 0);

        /* fprintf(stderr, "creating filetable\n");     */
        NewArray = (memfs_filetable_t)malloc(filearraysize * sizeof(memfs_file_t));

       	/* Postcondition:                                       */
       	/* NewArray != NULL, otherwise malloc did not work      */
       	assert( NewArray != NULL);
        
    } else {
	#ifdef DEBUG_MEMFS
        	fprintf(stderr, "Special case: numfiles >= filearraysize! Increasing the number of possible files\n");
	#endif
    
        /*  add capabiltity of N filehandles to the filetable array to allow the storage of additional files. */
        filearraysize += N;
        /*  printf("sizeof memfs_file_t: %d\n", sizeof(memfs_file_t)); */

        /* Precondition:                                */
        /* sizeof(memfs_file_t) larger than 0           */
        /* filetable != NULL				*/
	assert( (sizeof(memfs_file_t)) > 0);
	assert( filetable != NULL );

        NewArray = (memfs_filetable_t)malloc (filearraysize * sizeof(memfs_file_t));
        memcpy(NewArray, filetable, (filearraysize - N) * sizeof(memfs_file_t));
        free(filetable);
        filetable = NULL;
	
	/* Postcondition					*/
	/* NewArray != NULL, otherwise malloc did not work	*/
	/* filetable == NULL, otherwise free did not work	*/
	assert( NewArray != NULL );
	assert( filetable == NULL );
    }

    if (NewArray == NULL) {
        /*  realloc didn't work correctly! */
        fprintf(stderr, "memfs resize filetable: fatal ERROR!\n");
        return NULL;
        /*  do error handling here */
    } else {
        int i;
	#ifdef DEBUG_MEMFS
        	fprintf(stderr, "initializing values for elements <%d..%d>\n", numfiles, filearraysize);
	#endif

        for(i = numfiles; i < filearraysize; i++) {
            /*  initialize all new elements as empty */
            NewArray[i].filename = NULL;
            NewArray[i].status = FREE;
            NewArray[i].content = NULL;
            NewArray[i].open_count = 0;
	    NewArray[i].exclusive = 0;
	    NewArray[i].locked = NULL;
	    NewArray[i].totalLocks = 0;
            NewArray[i].numDataIndices = 0;
            NewArray[i].filepointer = 0;
            NewArray[i].filesize = 0;
        }

#ifdef TIMEMEASURE
       		t1 = gettime();
       		measuredTime[RESIZE] += t1 - t0;
#endif
    	return NewArray;
    }
}


/* 
    Check if file.filename == name 
*/
static int searchFile(char *name) {
    int i;

    /* Postcondition                                        */
    /* filetable != NULL, otherwise searchFile will not work       */
    assert( filetable != NULL );

    if(filetable == NULL) 
	return -1;
    
    for(i = 0; i < filearraysize; i++) {
        /* printf("comparing %s and %s\n", filetable[i].filename, name); */
        if(filetable[i].filename == NULL) continue;
        if(strcmp(filetable[i].filename, name) == 0) {

	#ifdef DEBUG_MEMFS
            fprintf(stderr, "File: %s already exists at index %d\n", name, i);
	#endif
            return i;
        }
    }
    #ifdef DEBUG_MEMFS
    	fprintf(stderr, "file %s does not exist\n", name);
    #endif
        return -1;
}


/*
    get a free slot in the filetable
*/
static int getFreeSlot(memfs_filetable_t filetable) {
    int i;

    /* Precondition                                     	   		*/
    /* filetable != NULL, otherwise nothing was assigned to the function  	*/
    assert( filetable != NULL );
    
    for(i = 0; i < filearraysize; i++) {
        if(filetable[i].status == FREE)
            return i;
    }
    
    return ERROR_FILETABLE_FULL;
}


/* 
    Copy an array of char from src to dest and begin at given indices in the two arrays. Copy "size" elements 
*/
static void arrayCopy(char *dest, char *src, int beginAtDest, int beginAtSrc, int size) {

    #ifdef TIMEMEASURE
    double t0, t1;
    t0 = gettime();
    #endif

    /* Precondition                     			*/
    /* dest and src can not be NULL				*/
    /* beginAtDest and beginAtSrc can not be smaller that 0	*/
    /* size has to be >= 0					*/
    assert( dest != NULL);
    assert( src != NULL);
    assert( beginAtDest >= 0 );
    assert( beginAtSrc >= 0);
    assert( size >= 0);

    /* copy from src+beginAtSrc to dest+beginAtDest. Copy size bytes */
    #ifdef DEBUG_MEMFS
    fprintf(stderr, "arrayCopy: copy %d bytes from pos %d of src to pos %d of dest\n", size, beginAtSrc, beginAtDest);
    #endif
    memcpy(dest+beginAtDest, src+beginAtSrc, size);

#ifdef TIMEMEASURE
    t1 = gettime();
    measuredTime[MEMCPY] += t1 - t0;
#endif

}


/*
     Init variables for time measurement
*/
static void initTime() {
#ifdef TIMEMEASURE
    int i;
    fprintf(stderr, "Initializing variables for Time measurement\n");
    timeInitialized = 1;
    for(i = 0; i < timeFunctions; i++) {
        measuredTime[i] = 0;
    }
#endif
}


/*
    Print results of time measurement
*/
static void printTime() {
#ifdef TIMEMEASURE
    int i;
    assert(timeInitialized);
    fprintf(stderr, "Results of Time Measurement:\n");
    fprintf(stderr, "Time used in OPEN   : %f\n", measuredTime[0]);
    fprintf(stderr, "Time used in CLOSE  : %f\n", measuredTime[1]);
    fprintf(stderr, "Time used in WRITE  : %f\n", measuredTime[2]);
    fprintf(stderr, "Time used in READ   : %f\n", measuredTime[3]);
    fprintf(stderr, "Time used in DELETE : %f\n", measuredTime[4]);
    fprintf(stderr, "Time used in READIO : %f\n", measuredTime[5]);
    fprintf(stderr, "Time used in WRITEIO: %f\n", measuredTime[6]);
    fprintf(stderr, "Time used in MEMCPY : %f\n", measuredTime[7]);
    fprintf(stderr, "Time used in RESIZE : %f\n", measuredTime[8]);
    fprintf(stderr, "Time used in ALLOC  : %f\n", measuredTime[9]);
#endif
}


/*==========================================================================================================*/
/* NON-STATIC FUNCTIONS */


/* 
    Open a file and return it's unique identificator (it's position in the filetable) 
    If file already exists only return it's unique identificator 
      position = -1    There is no spezific place for the file in the filetable
      position > 0     Place of the file in the filetable
*/
int memfs_open(char *name, int accessmode, int64_t blocksize, int position) {

    int pos = 0;
    int slot= 0;

    MPI_Comm_rank(MPI_COMM_META_REDUCED, &comm_rank);

#ifdef TIMEMEASURE
    double t0, t1;
    if(!timeInitialized)
        initTime();
    t0 = gettime();
#endif

    /*
        if (accessmode & ADIO_CREATE) printf("MPI_MODE_CREATE is set\n");
        if (accessmode & ADIO_RDWR) printf("MPI_MODE_RDWR is set\n");
        if (accessmode & ADIO_WRONLY) printf("MPI_MODE_WRONLY is set\n"); 
    */

    /* If filetable empty then create filetable */
    if(filetable == NULL) {
    	/* Precondition                                                 	*/
   	/* filetable == NULL, otherwise nothing was assigned to the function    */
   	assert( filetable == NULL );

	filetable = resize(filetable);

        /* Postcondition		                                             	*/
        /* filetable != NULL, otherwise nothing was assign to the function      */
        assert( filetable != NULL );
    }

    /* Precondition                                  */
    /* pos==position or position not specified (-1)  */
    assert( pos == 0 );

    /* Check if file already exists 	*/
    pos = searchFile(name);
    
    /* Postcondition			*/
    /*   pos == position or 
         position not specified or
         no file found -1               */
    assert( pos == position || position == -1 || pos == -1 );

    if(pos == -1) { /* File does not exist and has to be created */
        if (accessmode & ADIO_CREATE) {
            if(numfiles < filearraysize) { /* filetable is not full. file can be created. */

        	/* Precondition                         */
        	/* slot == 0                            */
        	assert( slot == 0);
		
		/* Position of file is not specified */
		if( position == -1) {
                	slot = getFreeSlot(filetable);
		} else {
		/* Position of file is specified and is returned */
			slot = position;
		}
		/* Postcondition			*/
		/* slot > 0				*/

                #ifdef DEBUG_MEMFS
                fprintf(stderr, "free slot at: %d\n", slot);
                #endif

		/* Postcondition			 				*/
		/* slot >= 0, otherwise an error occured 				*/
		/* filetable[slot].content == NULL, otherwise slot is already used 	*/
		assert(slot >= 0);
               	assert(filetable[slot].content == NULL);
		
		/* Precondition								*/
		/* filetable[slot].filename == NULL, otherwise file already used	*/
		assert(filetable[slot].filename == NULL);

		/* create the file */
                filetable[slot].filename = (char *)malloc((strlen(name)+1) * sizeof(char));
                strncpy(filetable[slot].filename, name, strlen(name));
                filetable[slot].filename[strlen(name)] = '\0';

		/* Postcondition							*/
		/* filetable[slot].filename != NULL, otherwise malloc did not work      */
		assert(filetable[slot].filename != NULL);

                filetable[slot].status = accessmode;
                filetable[slot].open_count += 1;
                if(blocksize != 0)
                   filetable[slot].dataindex_size = blocksize;
                else
                   filetable[slot].dataindex_size = DATAINDEX_SIZE;

                #ifdef DEBUG_MEMFS
                fprintf(stderr, "set filetable[slot].dataindex_size to: %lld\n", filetable[slot].dataindex_size);
                #endif
		
                numfiles++;

                #ifdef DEBUG_MEMFS
                fprintf(stderr, "created file %d : %s\n", slot, filetable[slot].filename);
                printfiletable(filetable, numfiles);
                #endif
            } else {
                /* filetable is full and has to be resized */
                filetable = resize(filetable);

                #ifdef DEBUG_MEMFS
                printfiletable(filetable, numfiles);
                #endif
                /* call memfs_open again, now with resized filetable */
                return memfs_open(name, accessmode, blocksize, position);
            } 
        } else {
            #ifdef DEBUG_MEMFS
            fprintf(stderr, "File %s does not exist and MPI_MODE_CREATE is not set, MPI_MODE: %d\n", name, accessmode);
            #endif
            return -1;
        }   
    } else { /* file already exists */
        /* TODO: Compare actual accessmode of file with new accessmode */
        filetable[pos].open_count++;
        slot = pos;
    }

#ifdef TIMEMEASURE
    t1 = gettime();
    measuredTime[OPEN] += t1 - t0;
#endif

    /* return slot as unique identificator of the file */
    return slot;
}


/*
    Allocate space for the file data
    Allocate the file up to filepointer + size  
*/
static int allocate_space(int fh, ADIO_Offset filepointer, int size) {
    int i, j, newblocks = 0;
    int endblock;
    int64_t allocated = 0;
    int before;
    /* maximum number of attempts to allocate space */
#ifdef TIMEMEASURE
    double t0, t1;
    t0 = gettime();
#endif

    before = filetable[fh].numDataIndices;

    assert(filetable[fh].dataindex_size > 0); 
    
    endblock = (filepointer + size) / filetable[fh].dataindex_size; 	// dataindex_size=1.000.000 Byte
    if(((filepointer + size) % filetable[fh].dataindex_size) > 0){
	endblock++;
    }	

#ifdef DEBUG_MEMFS
    fprintf(stderr, "allocate_space(): [%d] Endblock: %d, Size: %d, filepointer: %lld, dataindex_size: %lld \n", 
		comm_rank, endblock, size, filepointer, filetable[fh].dataindex_size);
#endif
 
    if(size < 0) return -1;

    for(i = 0; i < endblock; i++) {
        /* check if the dataindex array size has to be increased */
        if(filetable[fh].allocatedDataIndices <= filetable[fh].numDataIndices) {
            filetable[fh].content = (dataIndex *)realloc(filetable[fh].content,
                filetable[fh].allocatedDataIndices * 2 * sizeof(dataIndex));
            for(j = filetable[fh].allocatedDataIndices; j < filetable[fh].allocatedDataIndices * 2; j++) {
                filetable[fh].content[j].isAllocated = 0;
            }
            filetable[fh].allocatedDataIndices *= 2;
        }
        if(filetable[fh].content[i].isAllocated == 0) {
            filetable[fh].content[i].isAllocated = 1;
            //filetable[fh].content[i].locked = 0;
            /* calloc allocates space for an array of elements. The space is initialized with zero */
            filetable[fh].content[filetable[fh].numDataIndices].data = (char*)calloc(filetable[fh].dataindex_size, sizeof(char));
            allocated += filetable[fh].dataindex_size;

            if(filetable[fh].content[filetable[fh].numDataIndices].data == NULL) {
                fprintf(stderr, "allocate_space(): Tried to allocate %lld bytes! Insufficient Memory Available! Error at block %d\n", allocated, i);
                return -2;
            }
            filetable[fh].numDataIndices++;
            newblocks++;
        }
    }


    if(filetable[fh].numDataIndices < endblock) {
        fprintf(stderr, "WARNING: memfs_files [%d] allocate_space(): numDataIndices %d < endblock %d. numDataIndices before: %d\n", 
                comm_rank, filetable[fh].numDataIndices, endblock, before);
        fprintf(stderr, "WARNING: allocate_space(): [%d] Endblock: %d, Size: %d, filepointer: %lld, dataindex_size: %lld \n",
                comm_rank, endblock, size, filepointer, filetable[fh].dataindex_size);
        fprintf(stderr, "allocatedDataIndices: %d, content[%d].isAllocated: %d\n", 
                filetable[fh].allocatedDataIndices, endblock-1, filetable[fh].content[endblock-1].isAllocated);
    }
/*
    assert(filetable[fh].numDataIndices >= endblock);
*/



#ifdef DEBUG_MEMFS
    /* DEBUG INFORMATION: */
    fprintf(stderr, 
                "ALLOCATE_SPACE: Allocated %d new blocks for file %d. Needed space was: %d. File has now %d blocks\n"
                 ,newblocks, fh, size, filetable[fh].numDataIndices);
#endif

#ifdef TIMEMEASURE
    t1 = gettime();
    measuredTime[ALLOC] += t1 - t0;
#endif

    return 0;
}


/* 
    Write data into a file (Read data from a file)
    Write (Read) to file "fh", beginning at "filepointer", read (write) from "buf" and write (read) "size" byte
    mode: 0 means write, 1 means read
*/
static int FileIO(int fh, ADIO_Offset filepointer, char *buf, int size, int mode) {

    int i, towrite, written, block, blockpointer, blockspace;
    float carry, carry2;

    /* write in blocks of size DATAINDEX_SIZE */
    int numblocks;
  
#ifdef DEBUG_MEMFS
    fprintf(stderr, "Filepointer: %lld \n", filepointer);
#endif
 
#ifdef TIMEMEASURE
    double t0, t1;
    t0 = gettime();
#endif

    assert(buf != NULL);
    assert(filepointer >= 0);
    assert(fh >= 0);
    assert(size > 0);

    /* The data read/written is split in blocks of size: dataindex_size */
    numblocks = size / filetable[fh].dataindex_size;
    written = 0;
    towrite = size;
    /* avoid using ceil because therefore libmath is needed. 		
       in case of odd filesizes (i.e. 1.5MB) and a blocksize of 1MB	
       an extra block is added to contain the remainder (0.5MB)		*/
    if((size % filetable[fh].dataindex_size) > 0) {
        numblocks++;
    }

    /* if carry of (size / DATAINDEX_SIZE) + carry of (filepointer / DATAINDEX_SIZE) 
       > DATAINDEX_SIZE 1 more block is needed. 
       Example1: size=10.5MB, dataindex_size=1MB, filepointer=5.4MB
	=>	 carry=0.5MB, carry2=0.4MB
       Example2: size 10.5MB, dataindex_size=1MB, filepointer=5.6MB
	=>	 carry=0.5MB, carry2=0.6MB
									*/
    carry  = size % filetable[fh].dataindex_size;
    carry2 = filepointer % filetable[fh].dataindex_size;
    if(carry + carry2 > filetable[fh].dataindex_size) {
        numblocks++;
    }

#ifdef DEBUG_MEMFS
    fprintf(stderr, "size: %d, filetable[fh].dataindex_size: %lld, read / write %d blocks\n", size, filetable[fh].dataindex_size, numblocks);
#endif

    /* block = first block that is written with data 	*/

    for(i = 0; i < numblocks; i++) {
        block = (filepointer / filetable[fh].dataindex_size) + i;
        blockpointer = 0;
        blockspace = filetable[fh].dataindex_size;
        if(i == 0) {
            blockpointer = filepointer % filetable[fh].dataindex_size;
            blockspace = filetable[fh].dataindex_size - blockpointer;
        }

        
        /* check if not the full block needs to be written / read */
        if(towrite < blockspace) 
            blockspace = towrite;

#ifdef DEBUG_MEMFS
        fprintf(stderr, "read / write block %lld of file %d. numDataIndices: %d. blockspace: %d\n", 
        	filepointer / filetable[fh].dataindex_size + i, fh, filetable[fh].numDataIndices, blockspace);
#endif

	/* mode == 0 read 	*/
	/* mode == 1 write 	*/        
        assert(mode == 0 || mode == 1);

#ifdef DEBUG_MEMFS
        fprintf(stderr, "position: %lld numDataIndices: %d\n",
        	filepointer / DATAINDEX_SIZE +i, filetable[fh].numDataIndices);
#endif

        if(block >= filetable[fh].numDataIndices) {
            fprintf(stderr, "[%d] Error: block %d >= filetable[fh].numDataIndices %d, Mode %d\n", 
                   comm_rank, block, filetable[fh].numDataIndices, mode);
            fprintf(stderr, "[%d] filepointer: %lld, size: %d, numblocks: %d\n", comm_rank, filepointer, size, numblocks);
	    return -1;
        }

        /*
            arraycopy(dest, src, dest offset, src offset, number of elements)

            WRITE (inverted for READ):
              copy from "buf" to "filetable"
              at current "block"
              write at position "blockpointer"
              read from position "i * DATAINDEX_SIZE"
              write "blockspace" bytes

            mode == 0 means WRITE
            mode == 1 means READ
        */
        if(mode == 0)
            arrayCopy(filetable[fh].content[block].data, buf, blockpointer, written, blockspace);
        if(mode == 1)
            arrayCopy(buf, filetable[fh].content[block].data, written, blockpointer, blockspace);
        
        towrite -= blockspace;
        written += blockspace;
        if(towrite < 0) {
            fprintf(stderr, "FATAL ERROR AT FUNCTION FileIO\n");
            return -1;
        }               
    }


#ifdef TIMEMEASURE
        t1 = gettime();
        if(mode == 0)
            measuredTime[WRITEIO] += t1 - t0;
        else
            measuredTime[READIO] += t1 - t0;
#endif

    return 0;
}




/*
    Write to the file identified by "fh" at position "filepointer".
    Write from char array "buf" exactly "size" elements to the file.
*/
int memfs_write(int fh, ADIO_Offset filepointer, char *buf, int size) {
    /* allocate enough space to write to file */
    int neededspace = 0;
    int error, i;
    int before = filetable[fh].numDataIndices;

#ifdef TIMEMEASURE
    double t0, t1;
    t0 = gettime();
#endif


    assert(filetable[fh].status != FREE);    
    if(!(filepointer >= 0)) {
        fprintf(stderr, "memfs_write: [%d] This is strange! Filepointer is at %lld\n", comm_rank, filepointer);
        return 0;
    }
    assert(buf != NULL);
    
    if(!(filetable[fh].status & ADIO_RDWR || filetable[fh].status & ADIO_WRONLY)) {
        fprintf(stderr, "file %s is not opened for writing. Error!\n", filetable[fh].filename);
        return -1;
    }    
  
    if(size > 0) { 
 
        if (filetable[fh].content == NULL) {
            /* the file is empty right now */
            /* need to allocate "size" bytes */
            neededspace = size;
            /* allocate space for 10 dataIndices at once. Will be increased when needed */
            /* sizeof(dataIndex): 12 Byte */
            filetable[fh].content = (dataIndex *)malloc(N * sizeof(dataIndex));
            filetable[fh].allocatedDataIndices = N; 
            for(i = 0; i < N; i++) {   
                filetable[fh].content[i].isAllocated = 0;
            }
        } else {
            neededspace = size;
            /* There is already data in the file */        
        }    

#ifdef DEBUG_MEMFS
        fprintf(stderr, "[%d] Needed space for write is: %d\n", comm_rank, neededspace);
#endif

        /* now allocate the space */
#ifdef DEBUG_MEMFS
        fprintf(stderr, "memfs_write: [%d], File: %d, Neededspace: %d, filepointer: %lld, size: %d, mode: WRITE, numDataIndices: %d\n",
                comm_rank,fh,neededspace,filepointer,size,filetable[fh].numDataIndices);
#endif
        if(neededspace > 0) {
            error = allocate_space(fh, filepointer, neededspace);
        }

        if(error == -2) {
            fprintf(stderr, "Error in allocating space. Exiting\n");
            return -1;
        }
#ifdef DEBUG_MEMFS
        fprintf(stderr, "[%d]: file: %d numDataIndices before: %d, now: %d, filepointer: %lld, size: %d, dataindex_size: %lld\n", 
                comm_rank, fh, before, filetable[fh].numDataIndices, filepointer, size, filetable[fh].dataindex_size);
        assert(filetable[fh].numDataIndices >= (filepointer+size)/filetable[fh].dataindex_size);
#endif

        /* now there is enough allocated space. Write Data into File now */
#ifdef DEBUG_MEMFS
        fprintf(stderr, "[%d] calling FileIO for file %d, filepointer %lld, size %d, mode: WRITE\n", comm_rank, fh, filepointer, size);
#endif

        if(size > 0)
            FileIO(fh, filepointer, buf, size, 0);

    
        /* Update filesize */
        if(filepointer + size > filetable[fh].filesize)
            filetable[fh].filesize = filepointer + size;

#ifdef DEBUG_MEMFS
        fprintf(stderr, "[%d] memfs_write: filesize: %lld\n", comm_rank, filetable[fh].filesize);
#endif
        
        /* Update filepointer */
        filetable[fh].filepointer = filepointer + size;


#ifdef DEBUG_MEMFS
        fprintf(stderr, "[%d] memfs_write finished\n", comm_rank);
#endif

#ifdef TIMEMEASURE
        t1 = gettime();
        measuredTime[WRITE] += t1 - t0;
#endif

    } /* if (size > 0) */

    return 0;

}


/*
    Read from the file "fh". Start at position "filepointer".
    Write exactly "size" elements to the char array "buf".
*/
int memfs_read(int fh, ADIO_Offset filepointer, char *buf, int count) {

#ifdef TIMEMEASURE
    double t0, t1;
    t0 = gettime();
#endif

    if(!(filepointer >= 0)) {
        fprintf(stderr, "memfs_read: [%d] This is strange! Filepointer is at %lld\n", comm_rank, filepointer);
        return 0;
    }


    if(count > 0) {

        if(filetable[fh].content == NULL) {
            printfiletable(filetable, numfiles);
            fprintf(stderr, "memfs_read: The File %s is not initialized. filesize: %lld Error!\n", filetable[fh].filename, filetable[fh].filesize);
            return 0;
        }

        if(!(filetable[fh].status & ADIO_RDWR || filetable[fh].status & ADIO_RDONLY)) {
            fprintf(stderr, "READ MODE is not set for file %s. Error!\n",filetable[fh].filename);
            return -1;
        }


        if(filepointer + count > filetable[fh].filesize) {

#ifdef DEBUG_MEMFS
          fprintf(stderr, "[%d] %lld > %lld\n", comm_rank, filepointer + count, filetable[fh].filesize);
          fprintf(stderr, "[%d] Trying to read behind EOF. Error!\n", comm_rank);
          fprintf(stderr, "[%d] filepointer: %lld, count: %d\n", comm_rank, filepointer, count);
#endif
          count = filetable[fh].filesize - filepointer;       
        } 
#ifdef DEBUG_MEMFS
        else {

          fprintf(stderr, "[%d] %lld <= %lld\n", comm_rank, filepointer + count, filetable[fh].filesize);
        }
        fprintf(stderr, "memfs_read: [%d] calling FileIO for file %d, filepointer: %lld, reading %d bytes\n", comm_rank, fh, filepointer, count);
#endif
    
        /* bzero(buf, strlen(buf)); */
        if(count > 0) {
            FileIO(fh, filepointer, buf, count, 1);
            filetable[fh].filepointer = filepointer + count;
        }
   
 
#ifdef TIMEMEASURE
        t1 = gettime();
        measuredTime[READ] += t1 - t0;
#endif
    }

    return count;
}


/*
    Close the file, i.e. decrease the open_count by 1
*/
int memfs_close(int fh) {

#ifdef TIMEMEASURE
    double t0, t1;
    t0 = gettime();
#endif


    if(filetable[fh].status == FREE) {
        fprintf(stderr, "memfs_close: File does not exist. ERROR!\n");
        return ERROR_NO_FILE_EXISTS;
    }
    
    /*  decrease the open_count */
    filetable[fh].open_count--;
    
    /*  if all processes have closed the file, set the status to CLOSED */
    if(filetable[fh].open_count == 0) {
        if(filetable[fh].status & ADIO_DELETE_ON_CLOSE) {
          ;
#ifdef DEBUG_MEMFS
          fprintf(stderr, "deleting file %s due to ADIO_DELETE_ON_CLOSE\n", filetable[fh].filename);
#endif
          /* delete is called automatically if DELETE_ON_CLOSE specified
          memfs_del(filetable[fh].filename);
          */
        } else {
          filetable[fh].status = CLOSED;
        }
    }
#ifdef DEBUG_MEMFS
    printfiletable(filetable, 10);    
#endif

#ifdef TIMEMEASURE
        t1 = gettime();
        measuredTime[CLOSE] += t1 - t0;
        printTime();
#endif
    return 0;
}


/*
    Delete the file "filename". Free the allocated memory
*/
int memfs_del(char *filename) {
    int fh;
    int i;

    fh = 0;

#ifdef TIMEMEASURE
    double t0, t1;
    t0 = gettime();
#endif

#ifdef DEBUG_MEMFS
    fprintf(stderr, "[%d] memfs_del: deleting file %s\n", comm_rank, filename);
#endif

    /* Precondition                              */
    /* fh == 0                                   */
    assert(fh == 0);

    /* get the filehandle for the given filename */
    fh = searchFile(filename);

#ifdef DEBUG_MEMFS
    printfiletable(filetable, numfiles);  
    fprintf(stderr, "[%d] deleting fh %d\n", comm_rank, fh);
#endif

    if(fh == -1) {
#ifdef DEBUG_MEMFS
      fprintf(stderr, "[%d] File does not exist. Can't be deleted\n", comm_rank);
#endif
      /* there seems to be a bug with deleting files somewhere. */
      /* So long return 0, although the file wasn't deleted */
      fprintf(stderr, "[%d] ERROR! Delete called for unexisting file %s\n", comm_rank, filename);
      return 0;
      /*  return -1; */
    }
    
    filetable[fh].status = FREE;
    /* free the allocated memory of the file */
    for(i = 0; i < filetable[fh].numDataIndices; i++) {
        if(filetable[fh].content != NULL) {
            filetable[fh].content[i].isAllocated = 0;
            //filetable[fh].content[i].locked = 0;
            if(filetable[fh].content[i].data != NULL) {
                free(filetable[fh].content[i].data);
            }
        }
    }
    if(filetable[fh].content != NULL) {
        free(filetable[fh].content);
        filetable[fh].content = NULL;
    }

    /* Postcondition							*/
    /* filetable[fh].content = NULL, otherwise free() did not work	*/
    assert(filetable[fh].content == NULL);

    /* reset all file values to 0 */
    filetable[fh].numDataIndices = 0;
    filetable[fh].filepointer = 0;
    filetable[fh].filesize = 0; 
    if(filetable[fh].filename != NULL) {
        free(filetable[fh].filename);
        filetable[fh].filename = NULL;
    }
    if(filetable[fh].locked != NULL) {
        free(filetable[fh].locked);
        filetable[fh].locked = NULL;
    }


#ifdef DEBUG_MEMFS
      fprintf(stderr, "[%d] File deleted. returning now\n", comm_rank);
#endif

    
#ifdef TIMEMEASURE
        t1 = gettime();
        measuredTime[DELETE] += t1 - t0;
#endif

    return 0;
}


int memfs_resize(int fh, ADIO_Offset size) {

  char *tempbuf;
  int toappend, lastblock, i;

#ifdef DEBUG_MEMFS
  fprintf(stderr, "memfs_resize called. Resizing file to %lld bytes\n", size);
#endif

  if(filetable[fh].filename == NULL) {
    fprintf(stderr, "File does not exist. ERROR!\n");
    return -1;
  }

  if(size < filetable[fh].filesize) {
    /* this should be the most common case. Reduce the filesize
       Just cut the file off after size bytes */

    lastblock = size / filetable[fh].dataindex_size;
    if(size % filetable[fh].dataindex_size > 0)
      lastblock++;

    for(i = filetable[fh].numDataIndices; i >= lastblock; i--) {
      free(filetable[fh].content[i].data);
      assert(filetable[fh].content[i].data==NULL);
      filetable[fh].content[i].isAllocated = 0;
    }
    filetable[fh].numDataIndices = lastblock;
    filetable[fh].filesize = size;

  } else if(size > filetable[fh].filesize) {
    /* special case: Increase the filesize */
    toappend = size - filetable[fh].filesize;
    tempbuf = (char *)malloc(toappend);
    bzero(tempbuf, toappend);
    /* just write zeroes in the new space to fill the file */
    memfs_write(fh, filetable[fh].filepointer, tempbuf, toappend);
    free(tempbuf);

    assert(tempbuf == NULL);
  }

  /* if size == filetable[fh].filesize nothing has to be done */

  return 0;

}

int memfs_getfilesize(int fh) {


  if(filetable[fh].filename == NULL) {
    fprintf(stderr, "memfs_getfilesize called for invalid file. File does not exist. ERROR!\n");
    return -1;
  }

  return filetable[fh].filesize;

}


int memfs_lock_file(int fh, int *blocks, int size, int num_server)
{
  int i,j,k,comm_rank,start_block;
  int newArraySize;
  int oldArraySize;
  num_server = num_server + 1;	/* negative server numbers are used trying to lock a file
				 * and positiv server numbers are used in case it was
				 * possible to lock all blocks
				 */ 

  MPI_Comm_rank(MPI_COMM_META_REDUCED, &comm_rank);

  /* 
   * Try to lock each block from the file fh
   *  -filetable[fh].content[blocks[j]].locked = negative server number
   * If it does not work return -1
   * If it does work assign the positiv server number to those blocks
   */

  newArraySize = blocks[size-1] + 1;
  oldArraySize = filetable[fh].totalLocks;

  if(filetable[fh].locked == NULL){
        filetable[fh].locked = (int*)calloc(newArraySize, sizeof(int));		
        filetable[fh].totalLocks = newArraySize;
  }

  // Calculate max size of the lock array
  if( filetable[fh].totalLocks < newArraySize ){
        // Realloc more space
	//fprintf(stderr,"service[%d] Lock Array has to be resized. totalLocks=%d, newArraySize=%d \n",comm_rank,filetable[fh].totalLocks,newArraySize);
 	filetable[fh].locked = (int*)realloc(filetable[fh].locked, newArraySize * sizeof(int));
	
	for(i=oldArraySize; i < newArraySize; i++){
                filetable[fh].locked[i] = 0;
		//fprintf(stderr,"[%d]",filetable[fh].locked[i]);
        }
	//fprintf(stderr,"\n");
	filetable[fh].totalLocks = newArraySize;
  }

#ifdef DEBUG_LOCKS 
  fprintf(stderr," \n");
  for(i=0; i < filetable[fh].totalLocks; i++){
    fprintf(stderr,"[%d]",filetable[fh].locked[i]);
  }
  fprintf(stderr,"ENDE \n");
#endif


  for(i = 0; i < size; i++){
	if(filetable[fh].locked[blocks[i]] == 0){
		filetable[fh].locked[blocks[i]] = (num_server * -1);
		//fprintf(stderr,"service[%d] i: blocks[i]=%d , content=%d \n",comm_rank,blocks[i],filetable[fh].locked[blocks[i]]);
	}
	else {
#ifdef DEBUG_LOCKS
		fprintf(stderr,"BEFORE RESET \n");
                for(i=0; i < filetable[fh].totalLocks; i++){
                  fprintf(stderr,"[%d]",filetable[fh].locked[i]);
                }
                fprintf(stderr," \n");		
#endif
	
		// reset the changed values
		for(k = 0; k < size; k++){
			if( filetable[fh].locked[blocks[k]] == (num_server * -1) ){
				filetable[fh].locked[blocks[k]] = 0;
			}
		}

#ifdef DEBUG_LOCKS
		fprintf(stderr,"service[%d] locking did not work. last i=%d, blocks[%d] to [%d] \n", comm_rank,i,blocks[0],blocks[size-1]);
  		fprintf(stderr,"AFTER RESET \n");
  		for(i=0; i < filetable[fh].totalLocks; i++){
  		  fprintf(stderr,"[%d]",filetable[fh].locked[i]);
  		}
		fprintf(stderr,"ENDE \n");
#endif

		return -1;
	}
  }

  // Locking worked, so assign a positiv server number to all blocks
  for(j = 0; j < size; j++){
        filetable[fh].locked[blocks[j]] = num_server;
	//fprintf(stderr,"service[%d] j: blocks[j] %d = %d \n",comm_rank,blocks[j],filetable[fh].locked[blocks[j]]); 
  }
 
  return 1; 	// 1=Ok, locking of the blocks worked 
}


int memfs_unlock_file(int fh, int *blocks, int size, int num_server)
{
  int i,j;
  num_server = num_server + 1;  /* negative server numbers are used trying to unlock a file
                                 * and positiv server numbers are used in case it was
                                 * possible to unlock all blocks
                                 */
#ifdef DEBUG_LOCKS
  fprintf(stderr,"service[%d] totalLocks=%d \n",comm_rank, filetable[fh].totalLocks);
  for(j=0; j < filetable[fh].totalLocks; j++){
    fprintf(stderr, "[%d]", filetable[fh].locked[j] );
  }
  fprintf(stderr,"\n");
#endif

  /*
   * Try to unlock each block from the file fh
   *  -filetable[fh].content[blocks[j]].locked = negative server number
   * If it does not work return -1
   * If it does work assign a 0 to those blocks
   */
  for(i = 0; i < size; i++){
        if(filetable[fh].locked[blocks[i]] == num_server){
                filetable[fh].locked[blocks[i]] = num_server * -1;
        }
        else {
		//fprintf(stderr,"service[%d] unlocking did not work. last i=%d, blocks[%d] to [%d] \n", comm_rank,i,blocks[0],blocks[size-1]);
                return -1;	// The block is locked by a different server, fatal error, should not happen
        }
  }

  // Unlocking worked, so assign a 0 to all blocks
  for(j = 0; j < size; j++){
        filetable[fh].locked[blocks[j]] = 0;
  }

#ifdef DEBUG_LOCKS
  fprintf(stderr,"\n");
  fprintf(stderr,"# service[%d] unlocking worked, totalLocks=%d \n \n",comm_rank,filetable[fh].totalLocks);

  for(j=0; j < filetable[fh].totalLocks; j++){
    fprintf(stderr, "[%d]", filetable[fh].locked[j] );
  }
  fprintf(stderr,"\n");
#endif

  return 1;     // 1=Ok, unlocking of the blocks worked
}


int memfs_get_exclusive(int fh){
  if(filetable[fh].filename == NULL)
	return -1;

  return filetable[fh].exclusive;	
}

int memfs_set_exclusive(int fh, int exclusive){
  filetable[fh].exclusive = exclusive;
  return filetable[fh].exclusive;
}

int64_t memfs_get_blocksize(int fh){
  return filetable[fh].dataindex_size; 
}

