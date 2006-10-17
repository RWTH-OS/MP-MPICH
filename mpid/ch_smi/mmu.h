#ifndef _MPID_SMI_MMU_H
#define _MPID_SMI_MMU_H

#include "adi3types.h"
#include "smidef.h"

#define MPID_SMI_SHREG_TABLE_SIZE	(128*1024*1024)
#define MPID_SMI_SHREG_PAGE_SIZE	(128*1024)

#define MPI_INFO_KEY_TYPE             "type"
#define MPI_INFO_VALUE_TYPE_PRIVATE   "private"
#define MPI_INFO_VALUE_TYPE_SHARED    "shared"
#define MPI_INFO_VALUE_TYPE_DEFAULT   "default"
#define MPI_INFO_KEY_ALIGN            "alignment"

#define MAY_BE_SHARED     0
#define MUST_BE_SHARED    1
#define MUST_BE_PRIVATE  -1

#define NO_ALIGNMENT      -1
#define AUTO_ALIGNMENT    0

typedef struct _MPID_SMI_shreg {
	int     				id;		/* shreg id */
	int						shid;	/* exchangable id */
	volatile int			refcount;
	int						owner;
	void *  				base;	/* local base address */
	size_t  				size;	/* size of shreg */
	int						islocal;
	int						ispool;	/* is it a pool or a static shared reg */
	struct _MPID_SMI_shreg	* next,
							* prev,
							* next_pool,
							* prev_pool,
							* next_id,
							* prev_id;
	void 					* list;
} MPID_SMI_shreg;


#define MPID_SMI_ISADDR_SHARED(ptr) \
					(MPID_SMI_Get_shreg (ptr) ? 1 : 0)



void * MPID_SMI_Alloc_mem (size_t size, MPID_Info * info);
void * MPID_SMI_Alloc_mem_internal (size_t size, int shared, int align);
int MPID_SMI_Free_mem (void *ptr);

int MPID_SMI_MMU_init (void);
void MPID_SMI_MMU_shutdown (void);

int MPID_SMI_Shreg_connect (int rank, int id);
int MPID_SMI_Shreg_tryconnect (int rank, int id);
int MPID_SMI_Shreg_disconnect (MPID_SMI_shreg * shreg);

MPID_SMI_shreg * MPID_SMI_Get_shreg (void *ptr);
MPID_SMI_shreg * MPID_SMI_Get_shreg_by_id (int rank, int id);

int MPID_SMI_Addr_to_offset (void *ptr, int *id, size_t *offset);
void * MPID_SMI_Offset_to_addr (int rank, int id, size_t offset);


/* are the following functions really needed?? */
MPID_SMI_shreg * MPID_SMI_Create_shreg (size_t size);
int MPID_SMI_Destroy_shreg (MPID_SMI_shreg * shreg);




















#endif	/* _MPID_SMI_MMU_H */


/*
 * Overrides for XEmacs and vim so that we get a uniform tabbing style.
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
