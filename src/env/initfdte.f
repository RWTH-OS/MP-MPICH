      subroutine mpir_init_fdtes( MPIR_int_dte, MPIR_float_dte,
     $     MPIR_double_dte, MPIR_complex_dte, MPIR_dcomplex_dte,
     $     MPIR_logical_dte,
     $     MPIR_char_dte, MPIR_byte_dte, MPIR_2int_dte, MPIR_2real_dte, 
     $     MPIR_2double_dte, MPIR_2complex_dte, MPIR_2dcomplex_dte,
     $     MPIR_int1_dte, MPIR_int2_dte, MPIR_int4_dte, MPIR_real4_dte,
     $     MPIR_real8_dte, MPIR_packed, MPIR_ub, MPIR_lb )
      integer MPIR_int_dte, MPIR_float_dte,
     $     MPIR_double_dte, MPIR_complex_dte, MPIR_dcomplex_dte,
     $     MPIR_logical_dte, MPIR_char_dte, MPIR_byte_dte,
     $     MPIR_2int_dte, 
     $     MPIR_2real_dte, MPIR_2double_dte, MPIR_2complex_dte,
     $     MPIR_2dcomplex_dte,
     $     MPIR_int1_dte, MPIR_int2_dte, MPIR_int4_dte, MPIR_real4_dte,
     $     MPIR_real8_dte, MPIR_packed, MPIR_ub, MPIR_lb
      include '../../include/mpif.h'
c      MPI_INTEGER          = MPIR_int_dte
c      MPI_REAL             = MPIR_float_dte
c      MPI_DOUBLE_PRECISION = MPIR_double_dte
c      MPI_COMPLEX          = MPIR_complex_dte
c      MPI_DOUBLE_COMPLEX   = MPIR_dcomplex_dte
c      MPI_LOGICAL          = MPIR_logical_dte
c      MPI_CHARACTER        = MPIR_char_dte
c      MPI_BYTE             = MPIR_byte_dte
c      MPI_2REAL            = MPIR_2real_dte
c      MPI_2DOUBLE_PRECISION= MPIR_2double_dte
c      MPI_2INTEGER         = MPIR_2int_dte
c      MPI_2COMPLEX         = MPIR_2complex_dte
c      MPI_2DOUBLE_COMPLEX  = MPIR_2dcomplex_dte
c      MPI_PACKED           = MPIR_packed
c      MPI_UB               = MPIR_ub
c      MPI_LB               = MPIR_lb
C
C     optional datatypes
c      MPI_INTEGER1         = MPIR_int1_dte
c      MPI_INTEGER2         = MPIR_int2_dte
c      MPI_INTEGER4         = MPIR_int4_dte
c      MPI_REAL4            = MPIR_real4_dte
c      MPI_REAL8            = MPIR_real8_dte
      return
      end
C
      subroutine mpir_get_fsize()
      real r(2)
C     character c(2)
      double precision d(2)
      call mpir_init_fsize( r(1), r(2), d(1), d(2) )
      return
      end
