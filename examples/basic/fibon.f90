!------------------------------------------------------------------------------
!
!  Program name:
!
!    fibonacci
!
!  Purpose:
!
!    This program calls a recursive function to calculate the nth value of
!    the Fibonacci sequence. 
!    fib(1) = 1, fib(2) = 1, fib(i) = fib(i-1) + fib(i-2), i.e.,
!    1, 1, 2, 3, 5, 8, 13, ...
!
!
!------------------------------------------------------------------------------

PROGRAM fibonacci

  IMPLICIT NONE
  INTEGER :: n, nfib
  INTEGER, SAVE :: count = 0

  WRITE (*, *) 'Input n:' 
  READ (*, *) n

  nfib = fib(n)
  WRITE (*, *) 'The nth value of the Fibonacci sequence, where n = ', n, &
               ', is: ', nfib
  WRITE (*, *) 'Function fib was invoked ', count, ' times.'

  CONTAINS
  
    RECURSIVE FUNCTION fib(n) RESULT(fib_result)

      INTEGER, INTENT(in) :: n
      INTEGER :: fib_result

      count = count + 1                     ! Increment count of function calls

      SELECT CASE (n)
        CASE (1)                            ! First value is 1
          fib_result = 1
        CASE (2)                            ! Second value is 1
          fib_result = 1
        CASE (3:)
          fib_result = fib(n-1) + fib(n-2)  ! Any others is sum of two previous
      END SELECT

      ! WRITE (*,*) 'n, fib = ', n, fib_result  ! Write out intermediate values

    END FUNCTION fib

END PROGRAM fibonacci
