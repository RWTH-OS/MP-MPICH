      subroutine mpir_init_fcm()
      include '../../include/mpif.h'
C
C     Tell C where MPI_BOTTOM is
      call mpir_init_bottom( MPI_BOTTOM )
      return
      end
C
      subroutine mpir_init_flog( itrue, ifalse )
      logical itrue, ifalse
C
      itrue  = .TRUE.
      ifalse = .FALSE.
      return
      end
