#ifndef MPIR_OP_COOKIE
/* MPI combination function */
#define MPIR_OP_COOKIE 0xca01beaf

#ifdef WIN32
typedef void (__stdcall MPI_User_function_s) ( void *, void *, int *, MPI_Datatype * ); 
#endif

struct MPIR_OP {
#ifdef _WIN32
    int stdcall;
    union {
	MPI_User_function *op;
	MPI_User_function_s *op_s;
    };
#else
    MPI_User_function *op;
#endif
    MPIR_COOKIE 
	int               commute;
    int               permanent;
};

typedef struct { 
  float re;
  float im; 
} s_complex;

typedef struct { 
  double re;
  double im; 
} d_complex;


#endif
