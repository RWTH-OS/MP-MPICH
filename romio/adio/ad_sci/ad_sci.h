/* $Id$    
 */

#ifndef AD_SCI_INCLUDE
#define AD_ACI_INCLUDE

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#include "adio.h"

#ifndef NO_AIO
#ifdef AIO_SUN
#include <sys/asynch.h>
#else
#include <aio.h>
#endif
#endif



/*
  defines 
*/
#define ADSCI_SEGMENT_SIZE (1024*1024)   /* default-segment-size => should be equal to page-size */
#define ADSCI_PATH ("/pio")              /* default-path, every file is locally saved to */
#define ADSCI_MAX_NAME_SIZE  256         /* maximum (file-)name-size */
#define ADSCI_MAX_NODE       1024        /* maximum number of nodes */
#define ADSCI_FD_OFFSET      3           /* offset for filedescriptors (next to stdout, stderr, etc.) */

#define ADSCI_FRGMT_SIZE     64          /* size of a fragment descriptor in SCI memory */
#define ADSCI_FRGMT_KEY_T    ADIO_Offset;
#define ADSCI_HASH_VALUE     1715        /* prelimitary: fixed hash value */        
/*
 * external C interface
 */

#ifdef __cplusplus
extern "C" {
#endif
    void ADIOI_SCI_Open(ADIO_File fd, int *error_code);
    void ADIOI_SCI_Close(ADIO_File fd, int *error_code);
    void ADIOI_SCI_ReadContig(ADIO_File fd, void *buf, int count, 
			      MPI_Datatype datatype, int file_ptr_type,
			      ADIO_Offset offset, ADIO_Status *status, int
			      *error_code);
    void ADIOI_SCI_WriteContig(ADIO_File fd, void *buf, int count, 
			       MPI_Datatype datatype, int file_ptr_type,
			       ADIO_Offset offset, ADIO_Status *status, int
			       *error_code);   
    void ADIOI_SCI_IwriteContig(ADIO_File fd, void *buf, int count, 
				MPI_Datatype datatype, int file_ptr_type,
				ADIO_Offset offset, ADIO_Request *request, int
				*error_code);   
    void ADIOI_SCI_IreadContig(ADIO_File fd, void *buf, int count, 
			       MPI_Datatype datatype, int file_ptr_type,
			       ADIO_Offset offset, ADIO_Request *request, int
			       *error_code);   
    int ADIOI_SCI_ReadDone(ADIO_Request *request, ADIO_Status *status, int
			   *error_code);
    int ADIOI_SCI_WriteDone(ADIO_Request *request, ADIO_Status *status, int
			    *error_code);
    void ADIOI_SCI_ReadComplete(ADIO_Request *request, ADIO_Status *status, int
				*error_code); 
    void ADIOI_SCI_WriteComplete(ADIO_Request *request, ADIO_Status *status,
				 int *error_code); 
    void ADIOI_SCI_Fcntl(ADIO_File fd, int flag, ADIO_Fcntl_t *fcntl_struct, int
			 *error_code); 
    void ADIOI_SCI_WriteStrided(ADIO_File fd, void *buf, int count,
				MPI_Datatype datatype, int file_ptr_type,
				ADIO_Offset offset, ADIO_Status *status, int
				*error_code);
    void ADIOI_SCI_ReadStrided(ADIO_File fd, void *buf, int count,
			       MPI_Datatype datatype, int file_ptr_type,
			       ADIO_Offset offset, ADIO_Status *status, int
			       *error_code);
    void ADIOI_SCI_WriteStridedColl(ADIO_File fd, void *buf, int count,
				    MPI_Datatype datatype, int file_ptr_type,
				    ADIO_Offset offset, ADIO_Status *status, int
				    *error_code);
    void ADIOI_SCI_ReadStridedColl(ADIO_File fd, void *buf, int count,
				   MPI_Datatype datatype, int file_ptr_type,
				   ADIO_Offset offset, ADIO_Status *status, int
				   *error_code);
    void ADIOI_SCI_IreadStrided(ADIO_File fd, void *buf, int count,
				MPI_Datatype datatype, int file_ptr_type,
				ADIO_Offset offset, ADIO_Request *request, int
				*error_code);
    void ADIOI_SCI_IwriteStrided(ADIO_File fd, void *buf, int count,
				 MPI_Datatype datatype, int file_ptr_type,
				 ADIO_Offset offset, ADIO_Request *request, int
				 *error_code);
    void ADIOI_SCI_Flush(ADIO_File fd, int *error_code);
    void ADIOI_SCI_Resize(ADIO_File fd, ADIO_Offset size, int *error_code);
    ADIO_Offset ADIOI_SCI_SeekIndividual(ADIO_File fd, ADIO_Offset offset, 
					 int whence, int *error_code);
    void ADIOI_SCI_SetInfo(ADIO_File fd, MPI_Info users_info, int *error_code);
    void ADIOI_SCI_Get_shared_fp(ADIO_File fd, int size, ADIO_Offset *shared_fp, 
				 int *error_code);
    void ADIOI_SCI_Set_shared_fp(ADIO_File fd, ADIO_Offset offset, int *error_code);
    int ADIOI_SCI_aio(ADIO_File fd, void *buf, int len, ADIO_Offset offset,
		      int wr, void *handle);
#ifdef __cplusplus
}
#endif

/*
  type-definitions 
*/
typedef unsigned long ADSCI_OFFSET;   /* for 32bit systems, we can only use this offset type */

typedef enum {
    DATA, STATE, LOCK, INTRPT
} ADISCI_scidata_idx_t;	
#define NBR_SCIDATA_IDX 4

/* persistent file-specific info ( saved to "<DEFAULT_PATH>.<filename>.adio_file_info") */ 
struct _ADSCI_file_info {
  char         node[ADSCI_MAX_NAME_SIZE];/* node the segment is locally saved to */
  int          sgmt_nbr;                   /* node's <seg_nr>th segment */
  ADIO_Offset  size;                       /* segment-size in bytes */
  int          nbr_sgmts;                  /* number of the file's segments */
  int          sgmts_per_node;              /* number of segments per node */
  ADIO_Offset  bytes_used;                 /* bytes really used in segment <seg_nr> */
  int          nbr_reqs;                   /* number of requests for new segments */
  ADIO_Offset  avg_req_size;               /* avarage request-size */
};
typedef struct _ADSCI_file_info ADSCI_file_info_t;

/* list of segments belonging to opened files (held in memory) */
struct _ADSCI_sgmt {
  int          fd;				  /* segment's fd */
  void         *data_ptr;                         /* pointer to data */
  ADIO_Offset  size;                              /* segment-size in bytes */
  char         node_name[MPI_MAX_PROCESSOR_NAME]; /* name of node the segment is persistently */
                                          	  /* lying on */  
  struct _ADSCI_sgmt *self,             /* pointer to itself */
      *prev_filesgmt,    /* pointer to the file's previous segment */
      *next_filesgmt,    /* pointer to the file's next segment */
      *prev_sgmt,        /* pointer to the previous segment */
      *next_sgmt;        /* pointer to the next segment */
};
typedef struct _ADSCI_sgmt ADSCI_sgmt_t;

/* list of opened files (held in memory) */
struct _ADSCI_file {
    int          fd;  			   
    char         name[ADSCI_MAX_NAME_SIZE]; /* filename */
    ADIO_Offset  size;                       /* file-size in bytes */
    int          nbr_nodes;                  /* number of nodes the file is distributed to */
    int          sgmts_per_node;             /* number of segments per node */
    int          nbr_reqs;                   /* number of requests for new segments */
    ADIO_Offset  avg_req_size;               /* avarage request-size */
    struct _ADSCI_file *self, *prev, *next;

    /* SCI related data */
    sci_desc_t *sci_desc[NBR_SCIDATA_IDX];   /* sgmnt descriptors towards each proc */
    sci_local_segment_t sci_locsgmt[NBR_SCIDATA_IDX];   /* local segments */
    sci_remote_segment_t *sci_rmtsgmt[NBR_SCIDATA_IDX]; /* remote segments */
    unsigned int *sci_sgmtid[NBR_SCIDATA_IDX];          /* segment ids */
    sci_map_t *sci_map[NBR_SCIDATA_IDX];                /* segment maps */
};
typedef struct _ADSCI_file ADSCI_file_t;

/* nodes participating the actual MPI-session 
   (held in working memory), needed for ad_sci_open() */
struct _ADSCI_nodes {
  char name[MPI_MAX_PROCESSOR_NAME]; /* node-name - gethostname() */
  int  rank;			     /* node's rank */
};
typedef struct _ADSCI_node ADSCI_node_t;

/* thread created for asynchronous I/O */
struct _ADSCI_thread {
   ADIO_Request         request;
   pthread_t            tid;
   int                  done;   
};
typedef struct _ADSCI_thread ADSCI_thread_t;

#endif




// 
// Overrides for XEmacs and vim so that we get a uniform tabbing style.
// XEmacs/vim will notice this stuff at the end of the file and automatically
// adjust the settings for this buffer only.  This must remain at the end
// of the file.
// ---------------------------------------------------------------------------
// Local variables:
// c-indent-level: 3
// c-basic-offset: 3
// tab-width: 3
// End:
// vim:tw=0:ts=3:wm=0:
// 
