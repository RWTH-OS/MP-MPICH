#ifndef _MPPTEST
#define _MPPTEST

/* for MPI implementations without MPI_Alloc_mem(), set this to 0 */
#define HAVE_MPI_ALLOC_MEM 1
/* to fully disable verification overhead (even if the verification itself
   must be enabled via -verify option, set this to 0 */
#define ENABLE_VERIFY      1

/* Definitions for pair-wise communication testing */
typedef struct {
    int  proc1, proc2;
    int  source, destination,    /* Source and destination.  May be the
				    same as partner (for pair) or different
				    (for ring) */
         partner;                /* = source = destination if same */
    int  is_master, is_slave;
    } PairData;

/* Structure for the collective communication testing */
typedef struct {
    MPI_Comm pset;       /* Procset to test over */
    int     src;         /* Source (for scatter) */
    } GOPctx;

#define NO_NBR -1

/* size of the job and my rank in MPI_COMM_WORLD */
extern int __NUMNODES, __MYPROCID;
extern int do_verify;
extern int use_mpi_alloc;

/* Function prototypes */
void *PairInit( int, int );
void *BisectInit( int );
void PairChange( int, PairData * );
void BisectChange( int, PairData * );
void *GOPInit( int *, char ** );
void RunATest( int, int*, int*, double *, double *, int *, double (*)(), 
	       int, int, int, int, void *, void *);
void CheckTimeLimit( void );
void *OverlapInit();

void init_rbuffer (void *buf, size_t len);
void init_sbuffer (void *buf, size_t len);
int check_rbuffer (void *buf, size_t len);

void *mpp_alloc (size_t len);
void mpp_free (void *buf);

double (*GetPairFunction())(int, int, PairData *);
double (*GetGOPFunction( int*, char **, char *, char *))( int, int, GOPctx *);
double memcpy_rate( int, int, PairData *);

/* Overlap testing */
double round_trip_nb_overlap();
double round_trip_b_overlap();

/* Routine to generate graphics context */
void *SetupGraph( int *, char *[] );

/* Global operations */
void PrintGOPHelp( void );

/* Patterns */
void PrintPatternHelp( void );

/* Prototypes */
double RunSingleTest();
void time_function();
void ClearTimes(void);


#endif
