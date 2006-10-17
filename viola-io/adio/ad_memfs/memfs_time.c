#include "memfs_time.h"

static int timers = 22;
static double timeMeasured[22];
static int functionCount[22];
static int timeInit = 0;

/*
    Function to measure time usage
*/
double gettime(void) {
  struct timeval t;

  (void)gettimeofday(&t, NULL);
  return t.tv_sec + t.tv_usec*1e-6;
}

void settime(int function, double value) {
    timeMeasured[function] += value;
    functionCount[function] += 1;
    
}

int timeInitialized() {
    return timeInit;
}

/*
     Init variables for time measurement
*/
void initTimeMeasurement() {
    int i;
    timeInit = 1;
    for(i = 0; i < timers; i++) {
        timeMeasured[i] = 0;
        functionCount[i] = 0;
    }
}


/*
    Print results of time measurement
*/
void printTimeMeasurement(int rank) {
    int comm_rank, comm_size, i;
    MPI_Status msg_status;
    MPI_Comm_rank(MPI_COMM_META_REDUCED, &comm_rank);
    MPI_Comm_size(MPI_COMM_META_REDUCED, &comm_size);
    int output_rank = rank;

    if(comm_rank == 0) {
    assert(timeInit);

        for(i = 0; i < comm_size; i++) {
            if(i != 0) {
                ptMPI_Recv(&timeMeasured, timers, MPI_DOUBLE, i, MEMFS_TIME_DATA, MPI_COMM_META_REDUCED, &msg_status);
                ptMPI_Recv(&functionCount, timers, MPI_DOUBLE, i, MEMFS_TIME_DATA, MPI_COMM_META_REDUCED, &msg_status);
                output_rank = i; 
            }
            fprintf(stderr, "\nResults of Time Measurement for server %d:\n", output_rank);
            fprintf(stderr, "Total Time in MEMFS      : %12.9f s  calls: %8d\n", timeMeasured[TOTAL_TIME], functionCount[TOTAL_TIME]);
            fprintf(stderr, "Time in READ             : %12.9f s  calls: %8d\n", timeMeasured[READ_TIME], functionCount[READ_TIME]);
            fprintf(stderr, "Time in READ_MAIN        : %12.9f s  calls: %8d\n", timeMeasured[READ_MAIN], functionCount[READ_MAIN]);    
            fprintf(stderr, "Time in READ_SETUP       : %12.9f s  calls: %8d\n", timeMeasured[READ_SETUP], functionCount[READ_SETUP]);
            fprintf(stderr, "Time in LOCAL_READ       : %12.9f s  calls: %8d\n", timeMeasured[LOCAL_READ], functionCount[LOCAL_READ]);
            fprintf(stderr, "Time in REMOTE_READ      : %12.9f s  calls: %8d\n", timeMeasured[REMOTE_READ], functionCount[REMOTE_READ]);
            fprintf(stderr, "Time in READ_SETLOCK     : %12.9f s  calls: %8d\n", timeMeasured[READ_SETLOCK], functionCount[READ_SETLOCK]);
            fprintf(stderr, "Time in READ_REMOVELOCK  : %12.9f s  calls: %8d\n", timeMeasured[READ_REMOVELOCK], functionCount[READ_REMOVELOCK]);
            fprintf(stderr, "Time in READ_SERVICE     : %12.9f s  calls: %8d\n", timeMeasured[READ_SERVICE], functionCount[READ_SERVICE]);
            fprintf(stderr, "---------------------------------------------------------\n");
            fprintf(stderr, "Time in WRITE            : %12.9f s  calls: %8d\n", timeMeasured[WRITE_TIME], functionCount[WRITE_TIME]);
            fprintf(stderr, "Time in WRITE_MAIN       : %12.9f s  calls: %8d\n", timeMeasured[WRITE_MAIN], functionCount[WRITE_MAIN]);
            fprintf(stderr, "Time in WRITE_SETUP      : %12.9f s  calls: %8d\n", timeMeasured[WRITE_SETUP], functionCount[WRITE_SETUP]);
            fprintf(stderr, "Time in LOCAL_WRITE      : %12.9f s  calls: %8d\n", timeMeasured[FILESYSTEM_WRITE], functionCount[FILESYSTEM_WRITE]);
            fprintf(stderr, "Time in REMOTE_WRITE     : %12.9f s  calls: %8d\n", timeMeasured[REMOTE_WRITE], functionCount[REMOTE_WRITE]);
            fprintf(stderr, "Time in WRITE_GETERROR   : %12.9f s  calls: %8d\n", timeMeasured[WRITE_GETERROR], functionCount[WRITE_GETERROR]);
            fprintf(stderr, "Time in WRITE_SETLOCK    : %12.9f s  calls: %8d\n", timeMeasured[WRITE_SETLOCK], functionCount[WRITE_SETLOCK]);
            fprintf(stderr, "Time in WRITE_REMOVELOCK : %12.9f s  calls: %8d\n", timeMeasured[WRITE_REMOVELOCK], functionCount[WRITE_REMOVELOCK]);
            fprintf(stderr, "Time in WRITE_SERVICE    : %12.9f s  calls: %8d\n", timeMeasured[WRITE_SERVICE], functionCount[WRITE_SERVICE]);
            fprintf(stderr, "---------------------------------------------------------\n");
            fprintf(stderr, "Time in OPEN             : %12.9f s  calls: %8d\n", timeMeasured[OPEN_TIME], functionCount[OPEN_TIME]);
            fprintf(stderr, "Time in CLOSE            : %12.9f s  calls: %8d\n", timeMeasured[CLOSE_TIME], functionCount[CLOSE_TIME]);
            fprintf(stderr, "Time in FILESYSTEM_LOCK  : %12.9f s  calls: %8d\n", timeMeasured[FS_LOCK], functionCount[FS_LOCK]);
            fprintf(stderr, "Time measurement         : %12.9f s  calls: %8d\n", timeMeasured[REFERENCE], functionCount[REFERENCE]);
        }
    } else {
        ptMPI_Send(timeMeasured, timers, MPI_DOUBLE, 0, MEMFS_TIME_DATA, MPI_COMM_META_REDUCED);
        ptMPI_Send(functionCount, timers, MPI_INT, 0, MEMFS_TIME_DATA, MPI_COMM_META_REDUCED);
    }

}

