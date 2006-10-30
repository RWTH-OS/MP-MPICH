c     
c     This is a test for MPI_ALLGATHERV for a maximum of 1024 processes.
c     
      program testmpi
      integer           mnprocs, lcwk1
      double precision  accurc
      parameter         ( mnprocs = 1024, lcwk1 = mnprocs*3 )
      parameter         ( accurc = 1d-9 )
      integer           comm, rc, myid, nprocs, ierr, i,
     &     recvts(0:mnprocs-1), displs(0:mnprocs-1)
      double precision  wrkbuf(3), cwk1(lcwk1)
      double precision  lobrdr, hibrdr, elem
      logical           error, gerr
      include           'mpif.h'
c     
      call MPI_INIT( ierr )
      comm = MPI_COMM_WORLD
      call MPI_COMM_RANK( comm, myid, ierr )
      call MPI_COMM_SIZE( comm, nprocs, ierr )
c     
c     check that the maximum number of processes is not exceeded
c     
      if (nprocs.gt.mnprocs) then
         print *, 'Can only be run with a maximum of', mnprocs,
     &        ' processes!'
         call MPI_ABORT( comm, 1 )
      end if
c     
c     initialize receive buffer with -10
c     
      do i = 1, lcwk1
         cwk1(i) = -10
      end do
c     
c     initialize send buffer with rank in MPI_COMM_WORLD
c     
      do i=1,3
         wrkbuf(i) = myid
      end do
c     
c     initialize receive count and displacements
c     
      do i = 0, nprocs-1
         recvts(i) = 3
         displs(i) = 3 * i
      end do
c     
      call MPI_ALLGATHERV( wrkbuf, recvts(myid), MPI_DOUBLE_PRECISION,
     &     cwk1, recvts, displs, MPI_DOUBLE_PRECISION,
     &     comm, ierr )
c     
c     check gathered data
c     
      error = .false.
      do i = 1, nprocs
         lobrdr = DBLE(i-1) - accurc
         hibrdr = DBLE(i-1) + accurc
         do j = 1, 3
c     check that received data is equal to expected data within
c     a reasonable level of accuracy
            elem = cwk1((i-1)*3 + j)
            if (elem.lt.lobrdr) then
               error = .true.
            else if (elem.gt.hibrdr) then
               error = .true.
            end if
         end do
      end do
c
c     build global error indicator via MPI_REDUCE
c
      call MPI_REDUCE( error, gerr, 1, MPI_LOGICAL, MPI_LOR, 0, comm,
     &     ierr )
c
c     print message on process 0
c
      if( myid.eq.0 ) then
         if ( gerr ) then
            print *, 'Some errors have occurred during MPI_ALLGATHERV'
         else
            print *, ' No Errors'
         end if
      end if
         
      call MPI_FINALIZE(rc)
c     
      end
c     
