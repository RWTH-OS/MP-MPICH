      subroutine get_arg(i,s)
      integer       i
      character*(*) s
      call getarg(i,s)
      return
      end
      
      subroutine n_arg(i)
      integer       i
      i=iarcg()
      return
      end
