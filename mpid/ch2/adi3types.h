/* $Id$ */

#ifndef ADI3_TYPES_H
#define ADI3_TYPES_H

#include <stdlib.h>
#include <stdio.h>
#include "mpiimpl.h"
#ifdef MPID_USE_DEVTHREADS
#include <pthread.h>
#endif
#ifndef WIN32
#include <sys/uio.h>
#else
	struct iovec {
		void 	*iov_base; 	/* Pointer to data.  */
		size_t 	iov_len; 	/* Length of data.  */
	};
	typedef short int16_t;
#endif


#define MPI_MAX_OBJECT_NAME 32



typedef struct MPIR_Info 			MPID_Info;
typedef struct MPIR_GROUP 			MPID_Group;
typedef struct MPIR_COMMUNICATOR	MPID_Comm;
typedef struct MPIR_DATATYPE	 	MPID_Datatype;
typedef union MPIR_HANDLE 			MPID_Request;


/* This structure is used, but not defined in the ADI-3 standard,
	and therefore might change later */
typedef struct _MPID_List {
	int 				idx;
	void 				* ptr;
	struct _MPID_List 	* next;
} * MPID_List;

	
/* include the device specific structures */
#ifdef MPI_SMI
#  include "../ch_smi/smiostypes.h"
#endif


#define MPID_WIN_COOKIE	0x14102001
typedef struct {
	MPIR_COOKIE						/* sanity check */
	int 			id;
	volatile int	ref_count;
	
	/* communicator related to window group */
	MPID_Comm		*comm;				
	
	/* address and length of *local* window */
	void 			*start_address;	
	int				length;

	MPID_List		attributes;
	MPI_Errhandler	errhandler;
	char 			name [MPI_MAX_OBJECT_NAME];

	/* the following field is missing in the ADI3 specification  */
	int 			disp_unit;			/* displacement unit */

#ifdef MPID_USE_DEVTHREADS
	pthread_mutex_t	mutex;
#endif
	/* other, device specific information */
	/* XXX not multi-device-safe ! Better use the include-file technique as it
	   is done for the rndv_handle in the recv_handle. */
	union {
#ifdef MPI_SMI
		MPID_SMI_Win	w_smi;
#else
		int				w_dummy;
#endif
	}				devinfo;
} MPID_Win;


/* remote handlers */

typedef enum {
	/* Core remote handlers */
	MPID_Hid_Request_to_send = 1,
	MPID_Hid_Cancel = 27,
	MPID_Hid_Define_Datatype,
	MPID_Hid_Lock_Request,
	MPID_Hid_Lock_Grant,
	MPID_Hid_Unlock,
	MPID_Hid_Put_emulation,
	/* other remote handlers - added by Frank Reker */
	MPID_Hid_Get_emulation = 600,		/* the number might change */
	MPID_Hid_Accumulate
} MPID_Handler_id;


typedef struct {
	int16_t	request_id;
} MPID_Hid_Cancel_t;

typedef struct {
	MPI_Aint	target_offset;
	int			target_count;
	int			origin_count;
	int			kind_dtype;
	int			dtypes_equal;
} MPID_Hid_Put_emulation_t;

typedef struct {
	MPI_Aint	target_offset;
	int			target_count;
	int			origin_count;
	int			kind_dtype;
	int			dtypes_equal;
	int			do_remote_put;
} MPID_Hid_Get_emulation_t;

typedef struct {
	MPI_Aint	target_offset;
	int			target_count;
	int			origin_count;
	int			kind_dtype;
	int			dtypes_equal;
	MPI_Op		op;
} MPID_Hid_Accumulate_t;




/* other datatypes defined by ADI-3 */

/* ADI-3 puts several structures from the MPI level to the device 
	level. Also most structures change a lot.
	We make now a mapping of the old structures to the new names.
 */










/* some macros for receiving pointer from handle */
#define MPID_GET_WIN_PTR(handle) \
			(MPID_Win *) MPIR_ToPointer (handle)
#define MPID_TEST_WIN_NOTOK(handle,ptr) \
			(!(ptr) || ((ptr)->cookie != MPID_WIN_COOKIE))

#define MPID_GET_GROUP_PTR(handle) \
			(MPID_Group *) MPIR_ToPointer (handle)
#define MPID_TEST_GROUP_NOTOK(handle,ptr) \
			(!(ptr) || ((ptr)->cookie != MPIR_GROUP_COOKIE))

#define MPID_GET_COMM_PTR(handle) \
			(MPID_Comm *) MPIR_ToPointer (handle)
#define MPID_TEST_COMM_NOTOK(handle,ptr) \
			(!(ptr) || ((ptr)->cookie != MPIR_COMM_COOKIE))

#define MPID_GET_ERRHANDLER_PTR(handle) \
			MPIR_GET_ERRHANDLER_PTR (handle)
#define MPID_TEST_ERRHANDLER_NOTOK(handle,ptr) \
			MPIR_TEST_ERRHANDLER_NOTOK (handle, ptr)

#define MPID_GET_INFO_PTR(handle) \
			((MPID_Info *) (handle))
#define MPID_TEST_INFO_NOTOK(handle,ptr) \
			((ptr) && ((ptr)->cookie != MPIR_INFO_COOKIE))

#define MPID_GET_DTYPE_PTR(handle) \
			(MPID_Datatype *)MPIR_ToPointer (handle)
#define MPID_TEST_DTYPE_NOTOK(handle,ptr) \
			(!(ptr) || ((ptr)->cookie != MPIR_DATATYPE_COOKIE))

#define MPID_COMM_GET_HANDLE_FROM_PTR(ptr) \
			(!(ptr) ? (MPI_Comm) 0 : (MPI_Comm) (ptr)->self)
#define MPID_GROUP_GET_HANDLE_FROM_PTR(ptr) \
			(!(ptr) ? (MPI_Group) 0 : (MPI_Group) (ptr)->self)
#define MPID_WIN_GET_HANDLE_FROM_PTR(ptr) \
			(!(ptr) ? (MPI_Win) 0 : (MPI_Win) (ptr)->id)
#define MPID_DTYPE_GET_HANDLE_FROM_PTR(ptr) \
			(!(ptr) ? (MPI_Datatype) 0 : (MPI_Datatype) (ptr)->self)
#define MPID_INFO_GET_HANDLE_FROM_PTR(ptr) \
			(!(ptr) ? (MPI_Info) 0 : (MPI_Info) (ptr)->self)





#define MPID_WIN_SYNC_FENCE		1
#define MPID_WIN_SYNC_POST		2
#define MPID_WIN_SYNC_START		3
#define MPID_WIN_SYNC_COMPLETE	4
#define MPID_WIN_SYNC_WAIT		5
#define MPID_WIN_SYNC_TEST		6













#endif	/* ADI3_TYPES_H */














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
