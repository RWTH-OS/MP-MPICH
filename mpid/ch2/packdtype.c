#include <stdlib.h>
#include "mpimem.h"
#include "mpiimpl.h"
#include "adi3types.h"
#include "packdtype.h"
#include "lock.h"

#ifdef WIN32
typedef _int32 int32_t;
#endif

#define MAGIC 997817411		/* magic number */
#define VERSION 1	

static int ** table = NULL;
static int * table_size = NULL;
#ifdef MPID_USE_DEVTHREADS
#  ifndef WIN32
		static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#  endif
#endif

#define GET_TYPE_FROM_ID(rank,id) (((id) + 1) > table_size[rank] ? 0 : \
									table[rank][id])

static int pack_dtype (MPID_Datatype *, int, void **, int *, int *, int *);
static MPI_Datatype unpack_dtype (void *, int);
static int init_table ();


/*
 * MPID_Pack_dtype	- packs a datatype description, so that it can
 *						be sent to another host, and there recreated
 *
 * input parameters:
 *	dtype	- the datatype to pack
 *	rank	- the rank, for whom it should be packed
 *
 * output parameters:
 *	buf		- the packed datatype
 *	size	- size of the packed datatype in byte
 *
 * return value:
 *	0: on error
 *	1: on success
 */

int
MPID_Pack_dtype (dtype, rank, buf, size)
	MPID_Datatype	* dtype;
	int				rank;
	void			** buf;
	size_t			* size;
{
	void 	* nbuf;
	int		asize;
	int		psize;
	int		numtypes;
	int32_t	* tarray;

	if (!buf || !size) return 0;

	psize = 0;
	asize = 1024;
	numtypes = 0;
	nbuf = malloc (asize);
	if (!nbuf) return 0;
	tarray = (int32_t *) nbuf;
	tarray[0] = MAGIC;
	tarray[1] = VERSION;
	psize = 4*4;
	if (dtype->basic || dtype->known && dtype->known[rank]) {
		tarray[4] = 12;
		tarray[5] = MPID_DTYPE_KNOWN;
		tarray[6] = dtype->self;
		psize += 12;
	} else if (!pack_dtype (dtype, rank, &nbuf, &asize, &psize, &numtypes)) {
		FREE (nbuf);
		return 0;
	}
	tarray[2] = psize;
	tarray[3] = numtypes;
	*buf = nbuf;
	*size = psize;
	
	return 1;
}


/* 
 * MPID_Unpack_dtype	- recreates a datatype packed with
 *							MPID_Pack_dtype
 *
 * input parameters:
 *	buf		- the packed datatype
 *	rank	- from whom is the datatype
 *
 * return value:
 *	a pointer to the recreated datatype or NULL on error
 */

MPID_Datatype *
MPID_Unpack_dtype (buf, rank)
	void	* buf;
	int		rank;
{
	int				i;
	MPID_Datatype	* dtype;
	MPI_Datatype	mtype;
	int				size, psize, numtypes;
	int32_t			* tarray;
	void 			* ptr;

	if (!buf) return NULL;
	tarray = (int32_t *) buf;
	if (tarray[0] != MAGIC || tarray[1] != VERSION) return NULL;
	if (!table && !init_table()) return NULL;
	psize = tarray[2];
	numtypes = tarray[3];
	size = 4*4;
	for (i=0; i<numtypes; i++) {
		ptr = (char *)buf + size;
		size += *(int32_t *)ptr;
		if (size < psize) return NULL;
		mtype = unpack_dtype (ptr, rank);
		if (!mtype) return NULL;
	}
	dtype = MPID_GET_DTYPE_PTR(mtype);
	if (MPID_TEST_DTYPE_NOTOK (mtype, dtype)) return NULL;

	return dtype;
}


/*
 * MPID_Get_known_dtype		- returns an already known datatype
 *
 * input parameters:
 *	id		- known datatype id
 *	rank	- rank of proc from whom the datatype is
 *
 * return value:
 *	datatype or NULL on error
 */

MPID_Datatype *
MPID_Get_known_dtype (id, rank)
	int	id, rank;
{
	MPI_Datatype	mtype;
	MPID_Datatype	* dtype;

	if (!table && !init_table()) return NULL;

	mtype = (MPI_Datatype) (id >=0 && id < table_size[rank] ? 
							GET_TYPE_FROM_ID (rank, id) : 0);
	dtype = mtype ? MPID_GET_DTYPE_PTR (mtype) : NULL;
	if (MPID_TEST_DTYPE_NOTOK (mtype, dtype)) dtype = NULL;

	return dtype;
}



/* internal functions */

static
int
pack_dtype (dtype, rank, buf, asize, psize, numtypes)
	MPID_Datatype	* dtype;
	int				rank;
	void			** buf;
	int				* asize;
	int				* psize;
	int				* numtypes;
{
	int		i;
	int32_t	* tarray;
	int		size, vector=0, indexed=0;
	int		numproc;

	if (dtype->basic || dtype->known && dtype->known[rank])
		return 1;

	if (dtype->dte_type != MPIR_STRUCT) {
		if (!pack_dtype (dtype->old_type, rank, buf, asize, psize, numtypes)) 
			return 0;
	} else if (dtype->old_types) {
		for (i=0; i<dtype->count; i++) 
			if (!pack_dtype (dtype->old_types[i], rank, buf, asize, psize, 
											numtypes))
				return 0;
	}

	switch (dtype->dte_type) {
	case MPIR_CONTIG:
		size	= 4 /* size */
				+ 4 /* kind */
				+ 4 /* this id */
				+ 4 /* count */
				+ 4 /* oldtype */
				;
		if (*asize < *psize + size) {
			*asize += 1024;
			*buf = realloc (*buf, *asize);
			if (!*buf) return 0;
		}
		tarray = (int32_t *)((char *)*buf + *psize);
		tarray[0] = size;
		tarray[1] = MPIR_CONTIG;
		tarray[2] = dtype->self;
		tarray[3] = dtype->count;
		tarray[4] = dtype->old_type->self;
		*psize += size;
		break;
	case MPIR_VECTOR:
	case MPIR_HVECTOR:
		size 	= 4 /* size */ 
				+ 4 /* kind */ 
				+ 4 /* this id */
				+ 4 /* count */
				+ 4 /* blocklen */
				+ 4 /* stride */
				+ 4 /* oldtype */
				;
		if (*asize < *psize + size) {
			*asize += 1024;
			*buf = realloc (*buf, *asize);
			if (!*buf) return 0;
		}
		tarray = (int32_t *)((char *)*buf + *psize);
		tarray[0] = size;
		tarray[1] = MPIR_HVECTOR;
		tarray[2] = dtype->self;
		tarray[3] = dtype->count;
		tarray[4] = dtype->blocklen;
		tarray[5] = dtype->stride;
		tarray[6] = dtype->old_type->self;
		*psize += size;
		break;
	case MPIR_INDEXED:
	case MPIR_HINDEXED:
		size	= 4 /* size */
				+ 4 /* kind */
				+ 4 /* this id */
				+ 4 /* count */
				+ 4 * dtype->count /* blocklens */
				+ 4 * dtype->count /* indexes */
				+ 4 /* oldtype */
				;
		if (*asize < *psize + size) {
			*asize += (size / 1024 + 1) * 1024;
			*buf = realloc (*buf, *asize);
			if (!*buf) return 0;
		}
		tarray = (int32_t *)((char *)*buf + *psize);
		tarray[0] = size;
		tarray[1] = MPIR_HINDEXED;
		tarray[2] = dtype->self;
		tarray[3] = dtype->count;
		for (i=0; i<dtype->count; i++)
			tarray[4+i] = dtype->blocklens[i];
		for (i=0; i<dtype->count; i++)
			tarray[4+dtype->count+i] = dtype->indices[i];
		tarray[4+2*dtype->count] = dtype->old_type->self;
		*psize += size;
		break;
	case MPIR_STRUCT:
		size	= 4 /* size */
				+ 4 /* kind */
				+ 4 /* this id */
				+ 4 /* count */
				+ 4 * dtype->count /* blocklens */
				+ 4 * dtype->count /* indexes */
				+ 4 * dtype->count /* oldtypes */
				;
		if (*asize < *psize + size) {
			*asize += (size / 1024 + 1) * 1024;
			*buf = realloc (*buf, *asize);
			if (!*buf) return 0;
		}
		tarray = (int32_t *)((char *)*buf + *psize);
		tarray[0] = size;
		tarray[1] = MPIR_STRUCT;
		tarray[2] = dtype->self;
		tarray[3] = dtype->count;
		for (i=0; i<dtype->count; i++)
			tarray[4+i] = dtype->blocklens[i];
		for (i=0; i<dtype->count; i++)
			tarray[4+dtype->count+i] = dtype->indices[i];
		for (i=0; i<dtype->count; i++)
			tarray[4+2*dtype->count+i] = dtype->old_types[i]->self;
		*psize += size;
		break;
	default:
		/* basic types are known */
		return 0;
	}
	/* this datatype is now known */
	if (!dtype->known) {
		MPI_Comm_size (MPI_COMM_WORLD, &numproc);
		dtype->known = MALLOC (sizeof (int) * numproc);
		if (dtype->known) 
			memset (dtype->known, 0, sizeof (int) * numproc);
	}
	if (dtype->known)
		dtype->known[rank] = 1;
	(*numtypes)++;
	return 1;
}






static
MPI_Datatype
unpack_dtype (buf, rank)
	void	* buf;
	int		rank;
{
	int32_t			* tarray;
	int				kind;
	int				count;
	int				blocklen;
	int				* blocklens;
	MPI_Aint		* indices;
	MPI_Datatype	* oldtypes,
					oldtype;
	int				typeid;
	MPI_Datatype	newtype;
	int				id;
	int				stride;
	int				j, retval;
	int				* newtable;
	int				newsize;

	tarray = (int32_t *) buf;
	kind = tarray[1];
	id = tarray[2];
	if (kind == MPID_DTYPE_KNOWN) {
		newtype = GET_TYPE_FROM_ID (rank, id);
		return newtype;
	}
	if (oldtype = GET_TYPE_FROM_ID (rank, id)) 
		MPI_Type_free (&oldtype);
	switch (kind) {
	case MPIR_CONTIG:
		count = tarray[3];
		typeid = tarray[4];
		if (!(oldtype = GET_TYPE_FROM_ID (rank, typeid))) return 0;
		if (MPI_Type_contiguous (count, oldtype, &newtype) != MPI_SUCCESS)
			return 0;
		break;
	case MPIR_VECTOR:
	case MPIR_HVECTOR:
		count = tarray[3];
		blocklen = tarray[4];
		stride = tarray[5];
		typeid = tarray[6];
		if (!(oldtype = GET_TYPE_FROM_ID (rank, typeid))) return 0;
		if (MPI_Type_hvector (count, blocklen, stride, oldtype, &newtype)
					!= MPI_SUCCESS) 
			return 0;
		break;
	case MPIR_INDEXED:
	case MPIR_HINDEXED:
		count = tarray[3];
		typeid = tarray[4+2*count];
		if (!(oldtype = GET_TYPE_FROM_ID (rank, typeid))) return 0;
		blocklens = MALLOC (sizeof (int) * count);
		if (!blocklens) return 0;
		indices = MALLOC (sizeof (MPI_Aint) * count);
		if (!indices) {
			FREE (blocklens);
			return 0;
		}
		for (j=0; j<count; j++) {
			blocklens[j] = tarray[4+j];
			indices[j] = tarray[4+count+j];
		}
		retval = MPI_Type_hindexed (count, blocklens, indices, oldtype, 
									&newtype) == MPI_SUCCESS;
		FREE (blocklens);
		FREE (indices);
		if (!retval) return 0;
		break;
	case MPIR_STRUCT:
		count = tarray[3];
		blocklens = MALLOC (sizeof (int) * count);
		indices = MALLOC (sizeof (MPI_Aint) * count);
		oldtypes = MALLOC (sizeof (MPI_Datatype) * count);
		if (!blocklens || !indices || !oldtypes) {
			if (blocklens) FREE (blocklens);
			if (indices) FREE (indices);
			if (oldtypes) FREE (oldtypes);
			return 0;
		}
		for (j=0; j<count; j++) {
			blocklens[j] = tarray[4+j];
			indices[j] = tarray[4+count+j];
			typeid = tarray[4+2*count+j];
			if (!(oldtypes[j] = GET_TYPE_FROM_ID (rank, typeid))) {
				FREE (oldtypes);
				FREE (indices);
				FREE (blocklens);
				return 0;
			}
		}
		retval = MPI_Type_struct (	count, blocklens, indices, oldtypes,
									&newtype) == MPI_SUCCESS;
		FREE (blocklens);
		FREE (indices);
		FREE (oldtypes);
		if (!retval) return 0;
		break;
	default:
		return 0;
	}

	/* commmit datatype */
	MPI_Type_commit (&newtype);

	/* insert new type into table */
	MPID_LOCK (&mutex);
	if (id >= table_size[rank]) {
		newsize = id + 1;
		newtable = realloc (table[rank], newsize * sizeof (int));
		if (!newtable) {
			MPID_UNLOCK (&mutex);
			return 0;
		}
		table_size[rank] = newsize;
		table[rank] = newtable;
	}
	table[rank][id] = newtype;
	MPID_UNLOCK (&mutex);

	return newtype;
}



static
int
init_table ()
{
	int	numproc;
	int	i,j;
	
	MPID_LOCK (&mutex);
	MPI_Comm_size (MPI_COMM_WORLD, &numproc);
	table = MALLOC (sizeof (int *) * numproc);
	table_size = MALLOC (sizeof (int) * numproc);
	if (!table_size || !table) {
		MPID_UNLOCK (&mutex);
		return 0;
	}
	for (i=0; i<numproc; i++) {
		table_size[i] = 256;
		table[i] = MALLOC (sizeof (int) * 256);
		if (!table[i]) {
			MPID_UNLOCK (&mutex);
			return 0;
		}
		memset (table[i], 0, sizeof (int) * 256);
		for (j=1; j<=16; j++) table[i][j] = j;
	}
	MPID_UNLOCK (&mutex);
	return 1;
}











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
