

#include "mpiimpl.h"
#include "mpimem.h"
/* pt2pt for MPIR_Type_get_limits */
#include "mpipt2pt.h"
#include "mpicoll.h"
#include "mpiops.h"

extern int MPIR_Op_errno;
#define MPIR_REDUCE_TAG   11


static int WSOCK_Reduce (void *, void *,int,struct MPIR_DATATYPE *, MPI_Op, int, struct MPIR_COMMUNICATOR *);
static int MPID_SMI_Reduce (void *, void *,int,struct MPIR_DATATYPE *, MPI_Op, int, struct MPIR_COMMUNICATOR *);

int MPID_WSOCK_Collops_init(struct MPIR_COMMUNICATOR *comm, MPIR_COMM_TYPE comm_type)
{
    /*if(comm_type == MPIR_INTRA) {
	comm->collops->Reduce = MPID_SMI_Reduce;//WSOCK_Reduce;
    }
    */
    return MPI_SUCCESS;
}


#define CALC_RED {\
    if (op_ptr->commute) {\
	if(op_ptr->stdcall) op_ptr->op_s(ib, ob, &cnt, &datatype->self);\
	else (*uop)(ib, ob, &cnt, &datatype->self);\
    } else {\
	if(op_ptr->stdcall) op_ptr->op_s(ob, ib, &cnt, &datatype->self);\
	else (*uop)(ob, ib, &cnt, &datatype->self);\
	if(RecvUp) memcpy( ob, ib, m_extent*cnt );\
    }\
}

static int WSOCK_Reduce ( 
	void *sendbuf, 
	void *recvbuf, 
	int count, 
	struct MPIR_DATATYPE *datatype, 
	MPI_Op op, 
	int root, 
	struct MPIR_COMMUNICATOR *comm )
{
    MPI_Status status;
    MPI_Request RecvUpRequest,SendUpRequest,RecvDownRequest,SendDownRequest;
    int        size, rank;
    int        mask, relrank,mrelrank, source, lroot;
    int        mpi_errno = MPI_SUCCESS;
    int UpCount,LowCount,RecvUp,RecvLow,SendUp,SendLow,UpFinished=0,DownFinished=0;
    MPI_User_function *uop;
    MPI_Aint   lb, ub, m_extent;  /* Extent in memory */
    void       *buffer;
    struct MPIR_OP *op_ptr;
    static char myname[] = "WSOCK_REDUCE";
    MPIR_ERROR_DECL;
    mpi_comm_err_ret = 0;
    
    /* Is root within the communicator? */
    MPIR_Comm_size ( comm, &size );
#ifndef MPIR_NO_ERROR_CHECKING
    if ( root >= size )
	mpi_errno = MPIR_Err_setmsg(MPI_ERR_ROOT, MPIR_ERR_ROOT_TOOBIG,
				    myname,(char *)0,(char *)0,root,size);
    if (root < 0) 
	mpi_errno = MPIR_Err_setmsg(MPI_ERR_ROOT,MPIR_ERR_DEFAULT,myname,
				    (char *)0,(char *)0,root);
    if (mpi_errno)
	return MPIR_ERROR(comm, mpi_errno, myname );
#endif
    
    /* See the overview in Collection Operations for why this is ok */
    if (count == 0) return MPI_SUCCESS;
    
    /* If the operation is predefined, we could check that the datatype's
    type signature is compatible with the operation.  
    */
#ifdef MPID_Reduce
    /* Eventually, this could apply the MPID_Reduce routine in a loop for
    counts > 1 */
    if (comm->ADIReduce && count == 1) {
    /* Call a routine to sort through the datatypes and operations ...
    This allows us to provide partial support (e.g., only SUM_DOUBLE)
	*/
	if (MPIR_ADIReduce( comm->ADIctx, comm, sendbuf, recvbuf, count, 
	    datatype->self, op, root ) == MPI_SUCCESS)
	    return MPI_SUCCESS;
    }
#endif
    
    /* Get my rank and switch communicators to the hidden collective */
    MPIR_Comm_rank ( comm, &rank );
    comm = comm->comm_coll;
    op_ptr = MPIR_GET_OP_PTR(op);
    MPIR_TEST_MPI_OP(op,op_ptr,comm,myname);
    uop  = op_ptr->op;
    
    
    /* Here's the algorithm.  Relative to the root, look at the bit pattern in 
    my rank.  Starting from the right (lsb), if the bit is 1, send to 
    the node with that bit zero and exit; if the bit is 0, receive from the
    node with that bit set and combine (as long as that node is within the
    group)
    
      Note that by receiving with source selection, we guarentee that we get
      the same bits with the same input.  If we allowed the parent to receive 
      the children in any order, then timing differences could cause different
      results (roundoff error, over/underflows in some cases, etc).
      
	Because of the way these are ordered, if root is 0, then this is correct
	for both commutative and non-commutitive operations.  If root is not
	0, then for non-commutitive, we use a root of zero and then send
	the result to the root.  To see this, note that the ordering is
	mask = 1: (ab)(cd)(ef)(gh)            (odds send to evens)
	mask = 2: ((ab)(cd))((ef)(gh))        (3,6 send to 0,4)
	mask = 4: (((ab)(cd))((ef)(gh)))      (4 sends to 0)
	
	  Comments on buffering.  
	  If the datatype is not contiguous, we still need to pass contiguous 
	  data to the user routine.  
	  In this case, we should make a copy of the data in some format, 
	  and send/operate on that.
	  
	    In general, we can't use MPI_PACK, because the alignment of that
	    is rather vague, and the data may not be re-usable.  What we actually
	    need is a "squeeze" operation that removes the skips.
    */
    /* Make a temporary buffer */
    MPIR_Type_get_limits( datatype, &lb, &ub );
    m_extent = ub - lb;
    /* MPI_Type_extent ( datatype, &extent ); */
    MPIR_ALLOC(buffer,(void *)MALLOC(m_extent * count),comm, MPI_ERR_EXHAUSTED, 
	"MPI_REDUCE" );
    buffer = (void *)((char*)buffer - lb);
    
    /* If I'm not the root, then my recvbuf may not be valid, therefore
    I have to allocate a temporary one */
    if (rank != root) {
	MPIR_ALLOC(recvbuf,(void *)MALLOC(m_extent * count),
	    comm, MPI_ERR_EXHAUSTED, "MPI_REDUCE" );
	recvbuf = (void *)((char*)recvbuf - lb);
    }
    
    /* This code isn't correct if the source is a more complex datatype */
    memcpy( recvbuf, sendbuf, m_extent*count );
    mask    = 0x1;
    if (op_ptr->commute) lroot   = root;
    else                 lroot   = 0;
    relrank = (rank - lroot + size) % size;
    mrelrank = size - 1 - relrank; /* The mirrored rank */

    /* Lock for collective operation */
    MPID_THREAD_LOCK(comm->ADIctx,comm);
    
    MPIR_Op_errno = MPI_SUCCESS;
    
    LowCount = count / 2;
    UpCount = count -LowCount;
    while (/*(mask & relrank) == 0 && */mask < size) {
	RecvUp = RecvLow=SendUp=SendLow = 0;
	/* Receive */ 
	if(!UpFinished) {
	    if ((mask & relrank) == 0) {
		source = (relrank | mask);
		if (source < size) {		
		    RecvUp = 1;
		    source = (source + lroot) % size;
		    //printf("%d receiving from %d mask %d\n",rank,source,mask);
		    mpi_errno = MPI_Irecv (buffer, UpCount, datatype->self, source, 
			MPIR_REDUCE_TAG, comm->self, &RecvUpRequest);
		    if (mpi_errno) return MPIR_ERROR( comm, mpi_errno, myname );
		}
	    } else {
		SendUp = UpFinished = 1;
		source = ((relrank & (~ mask)) + lroot) % size;
		//printf("%d sending to %d mask %d\n",rank,source,mask);
		mpi_errno  = MPI_Isend( recvbuf, UpCount, datatype->self, 
		    source,MPIR_REDUCE_TAG, comm->self,&SendUpRequest );
		if (mpi_errno) return MPIR_ERROR( comm, mpi_errno, myname );
	    }
	}

	if(!DownFinished) {
	    if((mask & mrelrank) == 0 ) {
		source = mrelrank|mask;
		if(source < size) {		
		    RecvLow = 1;
		    source = (size - 1 - source + lroot) % size;
		    //printf("%d receiving from %d mask %d\n",rank,source,mask);
		    mpi_errno = MPI_Irecv (((char*)buffer)+m_extent*UpCount, LowCount, datatype->self, source, 
			MPIR_REDUCE_TAG, comm->self, &RecvDownRequest);
		    if (mpi_errno) return MPIR_ERROR( comm, mpi_errno, myname );
		}
		
	    } else {
		source = ((mrelrank & (~ mask)) + lroot) % size;
		source = size - 1 - source;
		//printf("%d sending to %d mask %d\n",rank,source,mask);
		SendLow = DownFinished = 1;
		mpi_errno  = MPI_Isend( ((char*)recvbuf)+m_extent*UpCount, LowCount, datatype->self, 
		    source,MPIR_REDUCE_TAG, comm->self,&SendDownRequest );
		if (mpi_errno) return MPIR_ERROR( comm, mpi_errno, myname );
	    }
	}
		
	if(RecvUp || RecvLow) {
	    int *ib,*ob,cnt;
	    
	    if(RecvUp) {		
		ib = buffer;
		ob=recvbuf;
		cnt = UpCount;
		mpi_errno = MPI_Wait(&RecvUpRequest,&status);
		if (mpi_errno) return MPIR_ERROR( comm, mpi_errno, myname );
		CALC_RED
	    } 
	    if(RecvLow) {
		ob = (int*)(((char*)buffer)+m_extent*UpCount);
		ib=(int*)(((char*)recvbuf)+m_extent*UpCount);
		cnt = LowCount;
		mpi_errno = MPI_Wait(&RecvDownRequest,&status);
		if (mpi_errno) return MPIR_ERROR( comm, mpi_errno, myname );
		CALC_RED
	    }	    
	}
    
	if(SendUp) {
	    mpi_errno = MPI_Wait(&SendUpRequest,&status);
	    if (mpi_errno) return MPIR_ERROR( comm, mpi_errno, myname );
	}

	if(SendLow) {
	    mpi_errno = MPI_Wait(&SendDownRequest,&status);
	    if (mpi_errno) return MPIR_ERROR( comm, mpi_errno, myname );
	}
	
	if(UpFinished && DownFinished) break;		
	mask <<= 1;
    }
    
    if(relrank == size-1) {
	/* Send lower half to root */
	//printf("%d sending lower result to  %d\n",rank,root);
	mpi_errno  = MPI_Send( ((char*)recvbuf)+m_extent*UpCount, LowCount, datatype->self, 
		root,MPIR_REDUCE_TAG, comm->self );
	if (mpi_errno) return MPIR_ERROR( comm, mpi_errno, myname );
    } else if(rank == root) {
	/*Receive lower half */	
	source = (size-1+lroot)%size;
	//printf("%d receiving lower result from %d\n",root,source);
	mpi_errno  = MPI_Recv( ((char*)recvbuf)+m_extent*UpCount, LowCount, datatype->self, 
		source,MPIR_REDUCE_TAG, comm->self,&status );
	if (mpi_errno) return MPIR_ERROR( comm, mpi_errno, myname );
    }
    FREE( (char *)buffer + lb );
    if (!op_ptr->commute && root != 0) {
	if (rank == 0) {
	    mpi_errno  = MPI_Send( recvbuf, count, datatype->self, root, 
		MPIR_REDUCE_TAG, comm->self );
	}
	else if (rank == root) {
	    mpi_errno = MPI_Recv ( recvbuf, count, datatype->self, 0, /*size-1, */
		MPIR_REDUCE_TAG, comm->self, &status);
	}
    }
    //printf("********\n");
    //fflush(stdout);
    /* Free the temporarily allocated recvbuf */
    if (rank != root)
	FREE( (char *)recvbuf + lb );
    
    /* If the predefined operation detected an error, report it here */
    /* Note that only the root gets this result, so this can cause
    programs to hang, particularly if this is used to implement 
    MPI_Allreduce.  Use care with this.
    */
    if (mpi_errno == MPI_SUCCESS && MPIR_Op_errno) {
	/* PRINTF( "Error in performing MPI_Op in reduce\n" ); */
	mpi_errno = MPIR_Op_errno;
    }
    
    /* Unlock for collective operation */
    MPID_THREAD_UNLOCK(comm->ADIctx,comm);
    
    return (mpi_errno);
}


/* This is the original MPICH-Reduce for the cases in which the
   Rabenseifner-Reduce is less efficient. See src/coll/intra_fns.c for
   fully commented version. */
int MPID_SMI_intraReduce ( void *sendbuf, void *recvbuf, int count, 
			   struct MPIR_DATATYPE *datatype, MPI_Op op, 
			   int root, struct MPIR_COMMUNICATOR *comm )
{
  MPI_Status status;
  int        size, rank;
  int        mask, relrank, source, lroot;
  int        mpi_errno = MPI_SUCCESS;
  MPI_User_function *uop;
  MPI_Aint   lb, ub, m_extent;  /* Extent in memory */
  void       *buffer,*buf,*rbuf;
  struct MPIR_OP *op_ptr;
  static char myname[] = "MPID_SMI_intraReduce";
  MPIR_ERROR_DECL;
  mpi_comm_err_ret = 0;

  size = comm->local_group->np;
  /*  MPI_Comm_size ( comm->self, &size ); */
  /* See the overview in Collection Operations for why this is ok */
  if (count == 0) return MPI_SUCCESS;

  /* If the operation is predefined, we could check that the datatype's
     type signature is compatible with the operation.  
   */

  /* Get my rank and switch communicators to the hidden collective */
  rank = comm->local_group->local_rank;
  /*  MPI_Comm_rank ( comm->self, &rank ); */
  comm = comm->comm_coll;
  op_ptr = MPIR_GET_OP_PTR(op);
  MPIR_TEST_MPI_OP(op,op_ptr,comm,myname);
  uop  = op_ptr->op;

  /* Make a temporary buffer */
  MPIR_Type_get_limits( datatype, &lb, &ub );
  m_extent = ub - lb;
  /* MPI_Type_extent ( datatype, &extent ); */
  MPIR_ALLOC(buf,(void *)MALLOC(m_extent * count+32),comm, MPI_ERR_EXHAUSTED, 
	     "MPI_REDUCE" );
  /* Align memory to 32 bytes */
  buffer = (void*)(((((size_t)buf)/32)+1)*32);
  buffer = (void *)((char*)buffer - lb);

  /* If I'm not the root, then my recvbuf may not be valid, therefore
     I have to allocate a temporary one */
  if (rank != root) {
      MPIR_ALLOC(rbuf,(void *)MALLOC(m_extent * count+32),
		 comm, MPI_ERR_EXHAUSTED, "MPI_REDUCE" );
      recvbuf = (int*)(((((size_t)rbuf)/32)+1)*32);
      recvbuf = (void *)((char*)recvbuf - lb);
  }

  /* This code isn't correct if the source is a more complex datatype */
  memcpy( recvbuf, sendbuf, m_extent*count );
  mask    = 0x1;
  if (op_ptr->commute) lroot   = root;
  else                 lroot   = 0;
  relrank = (rank - lroot + size) % size;

  MPIR_Op_errno = MPI_SUCCESS;
  while (mask < size) {
	/* Receive */
	if ((mask & relrank) == 0) {
	    source = (relrank | mask);
	    if (source < size) {
		source = (source + lroot) % size;
		mpi_errno = MPI_Recv (buffer, count, datatype->self, source, 
				      MPIR_REDUCE_TAG, comm->self, &status);
		if (mpi_errno) return MPIR_ERROR( comm, mpi_errno, myname ); 
		if (op_ptr->commute)
#ifdef WIN32
		    if (op_ptr->stdcall) 
			op_ptr->op_s(buffer, recvbuf, &count, &datatype->self);
		    else
#endif
			(*uop)(buffer, recvbuf, &count, &datatype->self);
		else {
#ifdef WIN32
		    if(op_ptr->stdcall) 
			op_ptr->op_s(recvbuf, buffer, &count, &datatype->self);
		    else
#endif
			(*uop)(recvbuf, buffer, &count, &datatype->self);
		    /* short term hack to keep recvbuf up-to-date */
		    memcpy( recvbuf, buffer, m_extent*count );
		    }
		/* MPIR_ERROR_PUSH(comm); */
		}
	    }
	else {
	    /* I've received all that I'm going to.  Send my result to 
	       my parent */
	    source = ((relrank & (~ mask)) + lroot) % size;
	    mpi_errno  = MPI_Send( recvbuf, count, datatype->self, 
				   source, MPIR_REDUCE_TAG, comm->self );
	    if (mpi_errno) return MPIR_ERROR( comm, mpi_errno, myname );
	    break;
	    }
	mask <<= 1;
	}
  FREE( buf);
  if (!op_ptr->commute && root != 0) {
      if (rank == 0) {
	  mpi_errno  = MPI_Send( recvbuf, count, datatype->self, root, 
				MPIR_REDUCE_TAG, comm->self );
	  }
      else if (rank == root) {
	  mpi_errno = MPI_Recv ( recvbuf, count, datatype->self, 0, /*size-1, */
				MPIR_REDUCE_TAG, comm->self, &status);
	  }
      }

  /* Free the temporarily allocated recvbuf */
  if (rank != root)
    FREE( rbuf );

  if (mpi_errno == MPI_SUCCESS && MPIR_Op_errno) {
      /* PRINTF( "Error in performing MPI_Op in reduce\n" ); */
      mpi_errno = MPIR_Op_errno;
  }

  return (mpi_errno);
}

/* copyright for the code in MPID_SMI_Anyreduce: */
/*  Rolf Rabenseifner, 1997
 *  Computing Center University of Stuttgart
 *  rabenseifner@rus.uni-stuttgart.de 
 *
 */ 

 /* required for Rabenseifner-Allreduce() (see below) */
/*  routine =    reduce                allreduce             */
/*  size    =       2,   3,2**n,other     2,   3,2**n,other  */
 static int Lsh[2][4]={{   1,   1,   1,   1},{   1,   1,   1,   1}};
 static int Lin[2][4]={{   1,   1,   1,   1},{   1,   1,   1,   1}};
 static int Llg[2][4]={{   1,   1,   1,   1},{   1,   1,   1,   1}};
 static int Lfp[2][4]={{   1024,1,1024,   1},{   1,   1,   1,   1}};
 static int Ldb[2][4]={{   1,   1,   1,   1},{   1,   1,   1,   1}};
 static int Lby[2][4]={{   1,   1,   1,   1},{   1,   1,   1,   1}};

int MPID_SMI_Anyreduce (void *Sendbuf, void *Recvbuf, int count, struct MPIR_DATATYPE *datatype,
			MPI_Op op, int root, struct MPIR_COMMUNICATOR *comm, int is_all)
{
  char *scrbas,*scrbuf, *scr2buf, *xxx, *sendbuf, *recvbuf;
  int myrank, size, x_base, x_size, idx, computed = 0;
  int x_start, x_count, r, n, mynewrank, newroot, partner;
  int start_even[20], start_odd[20], count_even[20], count_odd[20];
  int count_lowerhalf, count_upperhalf;
  MPI_Aint typelng;
  int mpi_errno = MPI_SUCCESS, rc;
  MPI_Status status;
  size_t scrlng;
  int new_prot;
  struct MPIR_OP *op_ptr;
  MPI_User_function *uop;

  new_prot = 0;
  size = comm->local_group->np;
  /*  MPI_Comm_size(comm->self, &size); */
  if (size > 1) {   /*otherwise no balancing_protocol*/
      register int ss;

      if (size==2)
	  ss=0;
      else if (size==3)
	  ss=1;
      else {
	  register int s = size;

	  while (!(s & 1)) s = s >> 1;
	  if (s==1) /* size == power of 2 */
	      ss = 2;
           else /* size != power of 2 */
	       ss = 3;
      }
      switch(op) {
      case MPI_MAX:   case MPI_MIN: case MPI_SUM:  case MPI_PROD:
      case MPI_LAND:  case MPI_LOR: case MPI_LXOR:
      case MPI_BAND:  case MPI_BOR: case MPI_BXOR:
	  switch(datatype->dte_type) {
	  case MPIR_SHORT:  case MPIR_USHORT:
	      new_prot = count >= Lsh[is_all][ss]; break;
	  case MPIR_INT:    case MPIR_UINT:
	      new_prot = count >= Lin[is_all][ss]; break;
	  case MPIR_LONG:   case MPIR_ULONG:
	      new_prot = count >= Llg[is_all][ss]; break;
	  }
      }
      switch(op) {
      case MPI_MAX:  case MPI_MIN: case MPI_SUM: case MPI_PROD:
	  switch(datatype->dte_type) {
	  case MPIR_FLOAT:
	      new_prot = count >= Lfp[is_all][ss]; break;
	  case MPIR_DOUBLE:
	      new_prot = count >= Ldb[is_all][ss]; break;
	  }
      }
      switch(op) {
      case MPI_BAND:  case MPI_BOR: case MPI_BXOR:
	  switch(datatype->dte_type) {
	  case MPIR_BYTE:
	      new_prot = count >= Lby[is_all][ss]; break;
	  }
      }
  }

  if (new_prot) {

      myrank = comm->local_group->local_rank;
      /* get pointer to operation */
      /* XXXX switch to the hidden collective first? see intraReduce XXXX */
      op_ptr = MPIR_GET_OP_PTR(op);
      /* XXXX test operation here? see intraReduce XXXX */
      uop = op_ptr->op;
      count_lowerhalf = count/2;
      count_upperhalf = count - count_lowerhalf;

      /*      MPI_Comm_rank(comm->self, &myrank); */
      typelng = datatype->extent;
      scrlng  = typelng * count;

      sendbuf = (char*) Sendbuf;
      recvbuf = (char*) Recvbuf;
      if (is_all)
	  root = myrank; /* for correct recvbuf handling */
      if( root == myrank ) { /* recvbuf is valid and can be used */
	  MPIR_ALLOC(scrbas,(char *)MALLOC(scrlng),comm, MPI_ERR_EXHAUSTED, 
	     "MPI_REDUCE" );
	  //ALLOCATE( scrbas, char *, scrlng );
	  
	  scr2buf = scrbas;
	  scrbuf = recvbuf;
      }
      else { /* we need one more temporary buffer */
	  //ALLOCATE( scrbas, char *, 2*scrlng );
	  MPIR_ALLOC(scrbas,(char *)MALLOC(2*scrlng),comm, MPI_ERR_EXHAUSTED,"MPI_REDUCE");
	  
	  scr2buf = scrbas;
	  scrbuf = scr2buf + scrlng;
      }
    
      /*...step 1 */

      n = 0; x_size = 1;
      while (2*x_size <= size) {
	  n++; x_size = x_size * 2;
      }
      /* x_sixe == 2**n */
      r = size - x_size;

      /*...step 2 */

      if (myrank < 2*r) {
	  if ((myrank % 2) == 0) { /* even */

	      MPI_Sendrecv(sendbuf + (count/2)*typelng,count - count/2, datatype->self, myrank+1, 1220,
		           scrbuf,count/2, datatype->self, myrank+1, 1221, comm->self, &status);
	      /*MPICH_Send(sendbuf + (count/2)*typelng,
		//	 count - count/2, datatype->self, myrank+1, 1220, comm->self);
	      //MPICH_Recv(scrbuf,
		//	 count/2, datatype->self, myrank+1, 1221, comm->self, &status);
		*/
#ifdef WIN32
	      if( op_ptr->stdcall )
		  op_ptr->op_s( sendbuf, scrbuf, &count_lowerhalf, &datatype->self );
	      else
#endif /* WIN32 */
		  (*uop)( sendbuf, scrbuf, &count_lowerhalf, &datatype->self );

	      MPI_Recv(scrbuf + (count/2)*typelng, count - count/2,
			      datatype->self, myrank+1, 1223, comm->self, &status);
	      computed = 1;
	  }
	  else { /* odd */

	      MPI_Sendrecv(sendbuf,count/2, datatype->self, myrank-1, 1221,
		  scrbuf + (count/2)*typelng,count - (count/2), datatype->self, myrank-1, 1220, 
		  comm->self,&status);
	      /*MPICH_Recv(scrbuf + (count/2)*typelng,
			 count - (count/2), datatype->self, myrank-1, 1220, comm->self, &status);
	      MPICH_Send(sendbuf,
			 count/2, datatype->self, myrank-1, 1221, comm->self);
		*/

#ifdef WIN32
	      if( op_ptr->stdcall )
		  op_ptr->op_s( sendbuf + count_lowerhalf*typelng,
				scrbuf + count_lowerhalf*typelng,
				&count_upperhalf, &datatype->self );
	      else
#endif /* WIN32 */
		  (*uop)( sendbuf + count_lowerhalf*typelng,
			  scrbuf + count_lowerhalf*typelng,
			  &count_upperhalf, &datatype->self );
	      MPI_Send(scrbuf + (count/2)*typelng, count - count/2,
			      datatype->self, myrank-1, 1223, comm->self);
	  }
      }

      /*...step 3+4 */

      if ((myrank >= 2*r) || ((myrank%2 == 0)  &&  (myrank < 2*r)))
	  mynewrank = (myrank < 2*r ? myrank/2 : myrank-r);
      else
	  mynewrank = -1;
      
      if (mynewrank >= 0) {    /* begin -- only for nodes with new rank */

#define OLDRANK(new)   ((new) < r ? (new)*2 : (new)+r)

	  /*...step 5 */

	  x_start = 0;
	  x_count = count;
	  for (idx=0, x_base=1; idx<n; idx++, x_base=x_base*2) {
	      start_even[idx] = x_start;
	      count_even[idx] = x_count / 2;
	      start_odd [idx] = x_start + count_even[idx];
	      count_odd [idx] = x_count - count_even[idx];
	      if (((mynewrank/x_base) % 2) == 0) {  
		  /* even */
		  x_start = start_even[idx];
		  x_count = count_even[idx];

		  MPI_Sendrecv((computed ? scrbuf : sendbuf) + start_odd[idx]*typelng,
			     count_odd[idx], datatype->self, OLDRANK(mynewrank+x_base), 1231,
			     scr2buf + x_start*typelng,
			     x_count, datatype->self, OLDRANK(mynewrank+x_base), 1232, comm->self, &status);

		  /*
		  MPICH_Send((computed ? scrbuf : sendbuf) + start_odd[idx]*typelng,
			     count_odd[idx], datatype->self, OLDRANK(mynewrank+x_base), 1231, comm->self);
		  MPICH_Recv(scr2buf + x_start*typelng,
			     x_count, datatype->self, OLDRANK(mynewrank+x_base), 1232, comm->self, &status);
		*/

#ifdef WIN32
		  if( op_ptr->stdcall )
		      op_ptr->op_s( (computed ? scrbuf : sendbuf) + x_start*typelng,
				    scr2buf + x_start*typelng,
				    &x_count, &datatype->self );
		  else
#endif
		      (*uop)( (computed ? scrbuf : sendbuf) + x_start*typelng,
			      scr2buf + x_start*typelng,
			      &x_count, &datatype->self );
	      }
	      else { 
		  /* odd */
		  x_start = start_odd[idx];
		  x_count = count_odd[idx];

		  MPI_Sendrecv((computed ? scrbuf : sendbuf) + start_even[idx]*typelng,
			        count_even[idx], datatype->self, OLDRANK(mynewrank-x_base), 1232,
			        scr2buf + x_start*typelng,x_count, datatype->self, OLDRANK(mynewrank-x_base), 1231,
			        comm->self,&status);
		  /*
		  MPICH_Recv(scr2buf + x_start*typelng,
			     x_count, datatype->self, OLDRANK(mynewrank-x_base), 1231, comm->self, &status);
		  MPICH_Send((computed ? scrbuf : sendbuf) + start_even[idx]*typelng,
			     count_even[idx], datatype->self, OLDRANK(mynewrank-x_base), 1232, comm->self);
		*/

#ifdef WIN32
		  if( op_ptr->stdcall )
		      op_ptr->op_s( (computed ? scrbuf : sendbuf) + x_start*typelng,
				    scr2buf + x_start*typelng,
				    &x_count, &datatype->self );
		  else
#endif
		      (*uop)( (computed ? scrbuf : sendbuf) + x_start*typelng,
			      scr2buf + x_start*typelng,
			      &x_count, &datatype->self );
	      }
	      computed = 1;
	      xxx = scrbuf; scrbuf = scr2buf; scr2buf = xxx;
	  } /*for*/

	  /* valid data should now be in scrbuf, but maybe scrbuf is != recvbuf, so we have to copy some data
	     (unfortunately); we want to copy as little as possible, so we copy only that data that we have correct;
	     this is the one on which the last calculation has been taken place */
	     
	  if( root == myrank ) {
	      if( scrbuf != recvbuf )
		  memcpy( recvbuf + x_start*typelng, scrbuf + x_start*typelng, x_count*typelng );
	  }
#undef OLDRANK
      } /* end -- only for nodes with new rank */
      
      if (is_all) {
	  /*...steps 6.1 to 6.n */

	  if (mynewrank >= 0) {   /* begin -- only for nodes with new rank */

#define OLDRANK(new)   ((new) < r ? (new)*2 : (new)+r)

	      /* to get data in recvbuf without memcpy */

	      for(idx=n-1, x_base=x_size/2; idx>=0; idx--, x_base=x_base/2) {
		  if (((mynewrank/x_base) % 2) == 0) {    
		      /* even */
		      MPI_Sendrecv(recvbuf + start_even[idx]*typelng,
				 count_even[idx], datatype->self, OLDRANK(mynewrank+x_base), 1241,
				 recvbuf + start_odd[idx]*typelng,
				 count_odd[idx], datatype->self, OLDRANK(mynewrank+x_base), 1242, comm->self, &status);
		    /*
		      MPICH_Send(recvbuf + start_even[idx]*typelng,
				 count_even[idx], datatype->self, OLDRANK(mynewrank+x_base), 1241, comm->self);
		      MPICH_Recv(recvbuf + start_odd[idx]*typelng,
				 count_odd[idx], datatype->self, OLDRANK(mynewrank+x_base), 1242, comm->self, &status);
		    */
		  }
		  else { 
		      /* odd */
		      MPI_Sendrecv(recvbuf + start_odd[idx]*typelng,
				 count_odd[idx], datatype->self, OLDRANK(mynewrank-x_base), 1242,
				 recvbuf + start_even[idx]*typelng,
				     count_even[idx], datatype->self, OLDRANK(mynewrank-x_base), 1241, 
				     comm->self, &status);
		    /*
		      MPICH_Recv(recvbuf + start_even[idx]*typelng,
				     count_even[idx], datatype->self, OLDRANK(mynewrank-x_base), 1241, comm->self, &status);
		      MPICH_Send(recvbuf + start_odd[idx]*typelng,
				 count_odd[idx], datatype->self, OLDRANK(mynewrank-x_base), 1242, comm->self);
		    */
		  }
	      } /* for */
#undef OLDRANK
	      
	  } /* end -- only for nodes with new rank */
	  
	  /*...step 7 */

	  if (myrank < 2*r) {
	      if (myrank%2 == 0)  /* even */
		  MPI_Send(recvbuf, count, datatype->self, myrank+1, 1253, comm->self);
	      else   /* odd */
		  MPI_Recv(recvbuf, count, datatype->self, myrank-1, 1253, comm->self, &status);
	  }

      }
      else {    /* not is_all, i.e. Reduce */

	  /*...step 6.0 */

	  if ((root < 2*r) && (root%2 == 1)) {
	      if (myrank == 0) { /* then mynewrank==0, x_start==0
				       x_count == count/x_size  */
		  MPI_Send(scrbuf,x_count,datatype->self,root,1241,comm->self);
		  mynewrank = -1;
	      }

	      if (myrank == root) {
		  mynewrank = 0;
		  x_start = 0;
		  x_count = count;
		  for (idx=0, x_base=1; idx<n; idx++, x_base=x_base*2) {
		      start_even[idx] = x_start;
		      count_even[idx] = x_count / 2;
		      start_odd [idx] = x_start + count_even[idx];
		      count_odd [idx] = x_count - count_even[idx];
		      /* code for always even in each bit of mynewrank: */
		      x_start = start_even[idx];
		      x_count = count_even[idx];
		  }
		  MPI_Recv(recvbuf,x_count,datatype->self,0,1241,comm->self,&status);
	      }
	      newroot = 0;
	  }
	  else
	      newroot = (root < 2*r ? root/2 : root-r);

	  /*...steps 6.1 to 6.n */

	  if (mynewrank >= 0) { /* begin -- only for nodes with new rank */

#define OLDRANK(new) ((new)==newroot ? root                     \
                             : ((new)<r ? (new)*2 : (new)+r) )

	      for(idx=n-1, x_base=x_size/2; idx>=0; idx--, x_base=x_base/2) {
		  if ((mynewrank & x_base) != (newroot & x_base)) {
		      if (((mynewrank/x_base) % 2) == 0) {   /* even */
			  x_start = start_even[idx]; x_count = count_even[idx];
			  partner = mynewrank+x_base;
		      }
		      else {
			  x_start = start_odd[idx]; x_count = count_odd[idx];
			  partner = mynewrank-x_base; }
		      MPI_Send(scrbuf + x_start*typelng, x_count, datatype->self,
			       OLDRANK(partner), 1244, comm->self);
		  }
		  else {   
		      /* odd */
		      if (((mynewrank/x_base) % 2) == 0) { /* even */
			  x_start = start_odd[idx]; x_count = count_odd[idx];
			  partner = mynewrank+x_base;
		      }
		      else {
			  x_start = start_even[idx]; x_count = count_even[idx];
			  partner = mynewrank-x_base;
		      }
		      MPI_Recv((myrank == root ? recvbuf : scrbuf)
			       + x_start*typelng, x_count, datatype->self,
			       OLDRANK(partner), 1244, comm->self, &status);
		  }
	      } /*for*/

#undef OLDRANK

	  } /* end -- only for nodes with new rank */
      }

      FREE( scrbas );

      return(MPI_SUCCESS);
  } /* new_prot */
  /*otherwise:*/
  if (is_all) {
      /* Reduce to 0, then bcast */
      mpi_errno = MPID_SMI_intraReduce( Sendbuf, Recvbuf, count, datatype, op, 0, 
					comm );
      if (mpi_errno == MPI_SUCCESS) {
	  rc = MPI_Bcast ( Recvbuf, count, datatype->self, 0, comm->self );
	  if (rc)
	      mpi_errno = rc;
      }
      return (mpi_errno);
  } else {
      return MPID_SMI_intraReduce( Sendbuf, Recvbuf, count, datatype, op, root, comm );
  }
}

int MPID_SMI_Reduce (void *sendbuf, void *recvbuf, int count, struct MPIR_DATATYPE *datatype,
		     MPI_Op op, int root, struct MPIR_COMMUNICATOR *comm)
{
    return MPID_SMI_Anyreduce( sendbuf, recvbuf, count, datatype, op, root, comm, 0 );
}

int MPID_SMI_Allreduce (void *sendbuf, void *recvbuf, int count, struct MPIR_DATATYPE *datatype,
			MPI_Op op, struct MPIR_COMMUNICATOR *comm)
{
    /*    printf("This is MPID_SMI_Allreduce\n");
	  fflush(stdout); */

    return MPID_SMI_Anyreduce( sendbuf, recvbuf, count, datatype, op, -1, comm, 1 );
}

