/*
 *  $Id$
 *
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"
/* MPIR_Type_xxx routines are prototyped in mpipt2pt.h */
#include "mpipt2pt.h"
#include "sbcnst2.h"
#define MPIR_SBinit MPID_SBinit
#define MPIR_SBalloc MPID_SBalloc
#define MPIR_SBfree MPID_SBfree
#define MPIR_trmalloc MPID_trmalloc

/* #define DEBUG(a) {a}  */
#define DEBUG(a)


#ifdef FORTRANCAPS
#define mpir_init_fdtes_ MPIR_INIT_FDTES
#define mpir_init_fsize_  MPIR_INIT_FSIZE
#define mpir_get_fsize_   MPIR_GET_FSIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpir_init_fdtes_ mpir_init_fdtes__
#define mpir_init_fsize_  mpir_init_fsize__
#define mpir_get_fsize_   mpir_get_fsize__
#elif !defined(FORTRANUNDERSCORE)
#define mpir_init_fdtes_ mpir_init_fdtes
#define mpir_init_fsize_  mpir_init_fsize
#define mpir_get_fsize_   mpir_get_fsize
#endif

#ifdef WIN32_FORTRAN_STDCALL
/* This is used on Windows to distinguish between
  FORTRAN compilers that use STDCALL convention ond those
  that use C convention. */
#define FORTRAN_API __stdcall
#else
#define FORTRAN_API
#endif


void mpir_get_fsize_ ANSI_ARGS(( void ));
void FORTRAN_API mpir_init_fsize_ ANSI_ARGS(( float *, float *, double *, double * ));

/* Global memory management variables for fixed-size blocks */
void *MPIR_dtes;      /* sbcnst datatype elements */

/* Static space for predefined datatypes */
struct MPIR_DATATYPE MPIR_I_CHAR, MPIR_I_SHORT, MPIR_I_INT, MPIR_I_LONG,
                            MPIR_I_UCHAR, MPIR_I_USHORT, MPIR_I_UINT, 
                            MPIR_I_ULONG, MPIR_I_FLOAT, MPIR_I_DOUBLE, 
                            MPIR_I_LONG_DOUBLE, MPIR_I_LONG_LONG_INT, 
                            MPIR_I_BYTE;
/* Derived and special types */
struct MPIR_DATATYPE MPIR_I_PACKED, MPIR_I_LONG_DOUBLE_INT,
                            MPIR_I_UB, MPIR_I_LB,
                            MPIR_I_2INTEGER, 
                            MPIR_I_FLOAT_INT, MPIR_I_DOUBLE_INT, 
                            MPIR_I_LONG_INT, MPIR_I_SHORT_INT, MPIR_I_2INT,
                            MPIR_I_REAL, MPIR_I_DOUBLE_PRECISION, 
                            MPIR_I_COMPLEX, MPIR_I_DCOMPLEX, 
                            MPIR_I_LONG_DOUBLE_INT, 
                            MPIR_I_LOGICAL, MPIR_I_INTEGER, MPIR_I_2FLOAT,
                            MPIR_I_2DOUBLE;

/* Definition for pointer to packed datatype */
struct MPIR_DATATYPE *MPIR_PACKED_PTR = 0;

/* Global pre-assigned datatypes */
void MPIR_Setup_base_datatype ANSI_ARGS((MPI_Datatype, struct MPIR_DATATYPE *, 
					 MPIR_NODETYPE, int));
void MPIR_Setup_complex_datatype ANSI_ARGS((MPI_Datatype, MPI_Datatype, 
					    struct MPIR_DATATYPE * ));
void MPIR_Type_contiguous ANSI_ARGS(( int, MPI_Datatype, 
				      struct MPIR_DATATYPE *, MPI_Datatype ));

/* C Datatypes for MINLOC and MAXLOC functions. 
   These allow us to construct the structures to match the compiler that
   is used to build MPICH, and, if the user uses the recommended compile
   scripts or makefiles, they will match the compiler that they use.
 */
typedef struct {
  float  var;
  int    loc;
} MPI_FLOAT_INT_struct;
MPI_FLOAT_INT_struct MPI_FLOAT_INT_var;

typedef struct {
  double var;
  int    loc;
} MPI_DOUBLE_INT_struct;
MPI_DOUBLE_INT_struct MPI_DOUBLE_INT_var;

typedef struct {
  long   var;
  int    loc;
} MPI_LONG_INT_struct;
MPI_LONG_INT_struct MPI_LONG_INT_var;

typedef struct {
  short  var;
  int    loc;
} MPI_SHORT_INT_struct;
MPI_SHORT_INT_struct MPI_SHORT_INT_var;

#if defined(HAVE_LONG_DOUBLE)
typedef struct {
  long double   var;
  int           loc;
} MPI_LONG_DOUBLE_INT_struct;
MPI_LONG_DOUBLE_INT_struct MPI_LONG_DOUBLE_INT_var;
#endif

/* Fortran datatypes */
/* MPI_Datatype MPIR_logical_dte; */
struct MPIR_DATATYPE MPIR_int1_dte, MPIR_int2_dte, MPIR_int4_dte, 
		MPIR_real4_dte, MPIR_real8_dte;
/* FORTRAN Datatypes for MINLOC and MAXLOC functions */
struct MPIR_DATATYPE MPIR_I_2REAL, MPIR_I_2DOUBLE_PRECISION, 
                     MPIR_I_2COMPLEX, MPIR_I_2DCOMPLEX;

/* Sizes of Fortran types; computed when initialized. */
static int MPIR_FSIZE_C = 0;  /* Characters */
static int MPIR_FSIZE_R = 0;  /* Reals */
static int MPIR_FSIZE_D = 0;  /* Doubles */

void MPIR_Init_dtes()
{
    MPI_Datatype   type[3], temptype;
    MPI_Aint       disp[3];
    int            blln[3];
    MPIR_NODETYPE  nodetype;

    /* set up pre-defined data types */
	TR_PUSH("MPIR_Init_dtes");
    DEBUG(PRINTF("[%d] About to create datatypes\n", MPIR_tid);)

    MPIR_dtes       = MPIR_SBinit( sizeof( struct MPIR_DATATYPE ), 100, 100 );

    MPIR_Setup_base_datatype( MPI_INT, &MPIR_I_INT, MPIR_INT, sizeof(int) );

    /* 
       Fortran requires that integers be the same size as 
       REALs, which are half the size of doubles.  Note that
       logicals must be the same size as integers.  Note that
       much of the code does not know about MPIR_LOGICAL or MPIR_FORT_INT
       yet. 

       We still need a FORT_REAL and FORT_DOUBLE type for some systems
     */
#if defined(MPID_NO_FORTRAN) || defined(WIN32)
    MPIR_FSIZE_C = sizeof(char);
    MPIR_FSIZE_R = sizeof(float);
    MPIR_FSIZE_D = sizeof(double);
#else
    mpir_get_fsize_();
#endif
    /* Rather than try to duplicate the Fortran types (e.g., 
       MPI_INTEGER = MPI_INT), we just generate new types
     */
    nodetype = MPIR_FORT_INT;
    if (sizeof(int) == MPIR_FSIZE_R) nodetype = MPIR_INT;
    else if (sizeof(long) == MPIR_FSIZE_R) nodetype = MPIR_LONG;
    MPIR_Setup_base_datatype( MPI_INTEGER, &MPIR_I_INTEGER, nodetype, 
			      MPIR_FSIZE_R );
    MPIR_Setup_base_datatype( MPI_DOUBLE, &MPIR_I_DOUBLE, 
			      MPIR_DOUBLE, sizeof(double) );
#if !defined(MPID_NO_FORTRAN)
    /* 
       Check for consistent definition of ints in the interfaces.
       If we get this message, we need to make changes in all of the 
       Fortran wrappers (replacing int types and adding casts)

       This is nearly out-of-date.  Most of the wrappers now correctly
       use MPI_Fint instead of int.
     */
    if (sizeof(int) != MPIR_FSIZE_R) {
	FPRINTF( stderr, 
	    "WARNING: Fortran integers are different in size from C ints\n" );
	}
#endif
    MPIR_Setup_base_datatype( MPI_LOGICAL, &MPIR_I_LOGICAL, MPIR_LOGICAL, 
			      MPIR_FSIZE_R );
    MPIR_Setup_base_datatype( MPI_FLOAT, &MPIR_I_FLOAT, 
			      MPIR_FLOAT, sizeof(float) );
    MPIR_Setup_base_datatype( MPI_COMPLEX, &MPIR_I_COMPLEX, 
			      MPIR_COMPLEX, 2 * MPIR_FSIZE_R );
    MPIR_I_COMPLEX.align  = MPIR_FSIZE_R;
    /* Hunt for Fortran real size */
    /* The original code here depended on Fortran being correctly 
       implemented.  Unfortunately, some vendors (e.g., Cray for the T3x)
       choose to have REAL = 8 bytes but don't want to have DOUBLE PRECISION
       = 16 bytes.  This is SPECIFICALLY prohibited by the Fortran standard
       (which requires that double precision be exactly twice the size of
       real).  
     */
    if (sizeof(float) == MPIR_FSIZE_R) {
	MPIR_Setup_base_datatype( MPI_REAL, &MPIR_I_REAL, MPIR_FLOAT,
				  MPIR_FSIZE_R );
	MPIR_Type_contiguous( 2, MPI_FLOAT, &MPIR_I_2FLOAT, MPI_2REAL );
	}
    else if (sizeof(double) == MPIR_FSIZE_R) {
	MPIR_Setup_base_datatype( MPI_REAL, &MPIR_I_REAL, MPIR_DOUBLE,
				  MPIR_FSIZE_R );
	MPIR_Type_contiguous( 2, MPI_DOUBLE, &MPIR_I_2DOUBLE, MPI_2REAL );
	}
    else {
	/* This won't be right */
	MPIR_Setup_base_datatype( MPI_REAL, &MPIR_I_REAL, MPIR_FLOAT,
				  MPIR_FSIZE_R );
	MPIR_Type_contiguous( 2, MPI_FLOAT, &MPIR_I_2FLOAT, MPI_2REAL );
	}

    /* Note that dcomplex is needed for src/pt2pt/pack_size.c */
    if (sizeof(double) == MPIR_FSIZE_D) {
	MPIR_Setup_base_datatype( MPI_DOUBLE_PRECISION, 
				  &MPIR_I_DOUBLE_PRECISION, MPIR_DOUBLE,
				  MPIR_FSIZE_D );
	MPIR_Setup_base_datatype( MPI_DOUBLE_COMPLEX, &MPIR_I_DCOMPLEX, 
				  MPIR_DOUBLE_COMPLEX, 2 * MPIR_FSIZE_D );
	MPIR_I_DCOMPLEX.align = MPIR_FSIZE_D;

	MPIR_Type_contiguous( 2, MPI_DOUBLE, &MPIR_I_2DOUBLE, 
			      MPI_2DOUBLE_PRECISION );
	}
#if defined(HAVE_LONG_DOUBLE)
    else if (sizeof(long double) == MPIR_FSIZE_D) {
	MPIR_Setup_base_datatype( MPI_DOUBLE_PRECISION, 
				  &MPIR_I_DOUBLE_PRECISION, MPIR_LONGDOUBLE, 
				  MPIR_FSIZE_D );
	/* These aren't correct (we need a ldcomplex datatype in 
	   global_ops.c */
	MPIR_Setup_base_datatype( MPI_DOUBLE_COMPLEX, &MPIR_I_DCOMPLEX, 
				  MPIR_DOUBLE_COMPLEX, 2 * MPIR_FSIZE_D );
	MPIR_I_DCOMPLEX.align = MPIR_FSIZE_D;

	MPIR_Type_contiguous( 2, MPI_DOUBLE_PRECISION, &MPIR_I_2DOUBLE, 
			      MPI_2DOUBLE_PRECISION );
	}
#endif
    else {
	/* we'll have a problem with the reduce/scan ops */
	MPIR_Setup_base_datatype( MPI_DOUBLE_PRECISION, 
				  &MPIR_I_DOUBLE_PRECISION, MPIR_DOUBLE, 
				  MPIR_FSIZE_D );
	MPIR_Setup_base_datatype( MPI_DOUBLE_COMPLEX, &MPIR_I_DCOMPLEX, 
				  MPIR_DOUBLE_COMPLEX, 2 * MPIR_FSIZE_D );
	MPIR_I_DCOMPLEX.align = MPIR_FSIZE_D;

	MPIR_Type_contiguous( 2, MPI_DOUBLE, &MPIR_I_2FLOAT, 
			      MPI_2DOUBLE_PRECISION );
	}

    MPIR_Setup_base_datatype( MPI_LONG, &MPIR_I_LONG, 
			      MPIR_LONG, sizeof(long) );
    MPIR_Setup_base_datatype( MPI_SHORT, &MPIR_I_SHORT, 
			      MPIR_SHORT, sizeof(short) );
    MPIR_Setup_base_datatype( MPI_CHAR, &MPIR_I_CHAR, 
			      MPIR_CHAR, sizeof(char) );
    MPIR_Setup_base_datatype( MPI_BYTE, &MPIR_I_BYTE, 
			      MPIR_BYTE, sizeof(char) );
    MPIR_Setup_base_datatype( MPI_UNSIGNED_CHAR, &MPIR_I_UCHAR, 
			      MPIR_UCHAR, sizeof(char) );
    MPIR_Setup_base_datatype( MPI_UNSIGNED_SHORT, &MPIR_I_USHORT, 
			      MPIR_USHORT, sizeof(short) );
    MPIR_Setup_base_datatype( MPI_UNSIGNED_LONG, &MPIR_I_ULONG, 
			      MPIR_ULONG, sizeof(unsigned long) );
    MPIR_Setup_base_datatype( MPI_UNSIGNED, &MPIR_I_UINT, 
			      MPIR_UINT, sizeof(unsigned int) );
    MPIR_Setup_base_datatype( MPI_PACKED, &MPIR_I_PACKED, MPIR_PACKED, 1 );
    MPIR_PACKED_PTR = &MPIR_I_PACKED;
    MPIR_Setup_base_datatype( MPI_UB, &MPIR_I_UB, MPIR_UB, 0 );
    MPIR_I_UB.align	     = 1;
    MPIR_I_UB.elements	     = 0;
    MPIR_I_UB.count	     = 0;

    MPIR_Setup_base_datatype( MPI_LB, &MPIR_I_LB, MPIR_LB, 0 );
    MPIR_I_LB.align	     = 1;
    MPIR_I_LB.elements	     = 0;
    MPIR_I_LB.count	     = 0;


#if defined(HAVE_LONG_DOUBLE)
    MPIR_Setup_base_datatype( MPI_LONG_DOUBLE, &MPIR_I_LONG_DOUBLE, 
			      MPIR_LONGDOUBLE, sizeof(long double) );
#else
    MPIR_Setup_base_datatype( MPI_LONG_DOUBLE, &MPIR_I_LONG_DOUBLE, 
			      MPIR_LONGDOUBLE, 2*sizeof(double) );
#endif

    /* Initialize C and FORTRAN types for MINLOC and MAXLOC */
    MPIR_Type_contiguous( 2, MPI_COMPLEX, &MPIR_I_2COMPLEX, MPI_2COMPLEX );
    MPIR_Type_contiguous( 2, MPI_DOUBLE_COMPLEX, &MPIR_I_2DCOMPLEX, 
			  MPI_2DOUBLE_COMPLEX );

    /* Initialize C & FORTRAN 2int type for MINLOC and MAXLOC */
    MPIR_Type_contiguous( 2, MPI_INT, &MPIR_I_2INT, MPI_2INT );

    /* Note Fortran requires sizeof(INTEGER) == sizeof(REAL) */
    MPIR_Type_contiguous( 2, MPI_INTEGER, &MPIR_I_2INTEGER, MPI_2INTEGER );

    /* Initialize C types for MINLOC and MAXLOC */
    /* I'm not sure that this is 100% portable */
    blln[0] = blln[1] = blln[2] = 1;
    type[1] = MPI_INT;   
    type[2] = MPI_UB;
    disp[0] = 0;

    type[0] = MPI_FLOAT;     
    disp[1] = (char *)&MPI_FLOAT_INT_var.loc - 
      (char *)&MPI_FLOAT_INT_var;
    disp[2] = sizeof(MPI_FLOAT_INT_struct);
    MPI_Type_struct ( 3, blln, disp, type, &temptype );
    MPIR_Setup_complex_datatype( temptype, MPI_FLOAT_INT, &MPIR_I_FLOAT_INT );

    type[0] = MPI_DOUBLE;
    disp[1] = (char *)&MPI_DOUBLE_INT_var.loc - 
      (char *)&MPI_DOUBLE_INT_var;
    disp[2] = sizeof(MPI_DOUBLE_INT_struct);
    MPI_Type_struct ( 3, blln, disp, type, &temptype );
    MPIR_Setup_complex_datatype( temptype, MPI_DOUBLE_INT, 
				 &MPIR_I_DOUBLE_INT );

    type[0] = MPI_LONG;
    disp[1] = (char *)&MPI_LONG_INT_var.loc - 
      (char *)&MPI_LONG_INT_var;
    disp[2] = sizeof(MPI_LONG_INT_struct);
    MPI_Type_struct ( 3, blln, disp, type, &temptype );
    MPIR_Setup_complex_datatype( temptype, MPI_LONG_INT, &MPIR_I_LONG_INT );

    type[0] = MPI_SHORT;
    disp[1] = (char *)&MPI_SHORT_INT_var.loc - 
      (char *)&MPI_SHORT_INT_var;
    disp[2] = sizeof(MPI_SHORT_INT_struct);
    MPI_Type_struct ( 3, blln, disp, type, &temptype );
    MPIR_Setup_complex_datatype( temptype, MPI_SHORT_INT, &MPIR_I_SHORT_INT );

#if defined(HAVE_LONG_DOUBLE)
    type[0] = MPI_LONG_DOUBLE;
    disp[1] = (char *)&MPI_LONG_DOUBLE_INT_var.loc - 
      (char *)&MPI_LONG_DOUBLE_INT_var;
    disp[2] = sizeof(MPI_LONG_DOUBLE_INT_struct);
    MPI_Type_struct ( 3, blln, disp, type, &temptype );
    MPIR_Setup_complex_datatype( temptype, MPI_LONG_DOUBLE_INT, 
				 &MPIR_I_LONG_DOUBLE_INT );
#else
    /* use just double if long double not available */
    type[0] = MPI_DOUBLE;
    disp[1] = (char *)&MPI_DOUBLE_INT_var.loc - 
      (char *)&MPI_DOUBLE_INT_var;
    disp[2] = sizeof(MPI_DOUBLE_INT_struct);
    MPI_Type_struct ( 3, blln, disp, type, &temptype );
    MPIR_Setup_complex_datatype( temptype, MPI_LONG_DOUBLE_INT, 
				 &MPIR_I_LONG_DOUBLE_INT );
#endif

#if defined(HAVE_LONG_LONG_INT)
    MPIR_Setup_base_datatype( MPI_LONG_LONG_INT, &MPIR_I_LONG_LONG_INT,
			      MPIR_LONGLONGINT, sizeof(long long) );
#else
    MPIR_Setup_base_datatype( MPI_LONG_LONG_INT, &MPIR_I_LONG_LONG_INT,
			      MPIR_LONGLONGINT, 2*sizeof(long) );
#endif

    /* Set the values of the Fortran versions */
    /* Logical and character aren't portable in the code below */
#ifndef MPID_NO_FORTRAN
    DEBUG(PRINTF("[%d] About to setup Fortran datatypes\n", MPIR_tid);)
    /* Try to generate int1, int2, int4, real4, and real8 datatypes */
#ifdef FOO
    MPIR_int1_dte  = 0;
    MPIR_int2_dte  = 0;
    MPIR_int4_dte  = 0;
    MPIR_real4_dte = 0;
    MPIR_real8_dte = 0;
    /* If these are changed to create new types, change the code in
       finalize.c to free the created types */
    if (sizeof(char) == 1)   MPIR_int1_dte = MPI_CHAR;
    if (sizeof(short) == 2)  MPIR_int2_dte = MPI_SHORT;
    if (sizeof(int) == 4)    MPIR_int4_dte = MPI_INT;
    if (sizeof(float) == 4)  MPIR_real4_dte = MPI_FLOAT;
    if (sizeof(double) == 8) MPIR_real8_dte = MPI_DOUBLE;
#endif
    /* These need to be converted into legal integer values for systems
       with 64 bit pointers */
/* values are all PARAMETER in the Fortran. */
#endif
	TR_POP;
}

/*
 * Having initialized the datatype, we also need to free them
 */
void MPIR_Free_dtes()
{
    /* We can't use MPI_Type_free, because it checks for predefined
       types. */
	TR_PUSH("MPIR_Free_dtes");
       MPIR_Free_perm_type( MPI_INT );
       MPIR_Free_perm_type( MPI_INTEGER );
       MPIR_Free_perm_type( MPI_DOUBLE );
       MPIR_Free_perm_type( MPI_LOGICAL );
       MPIR_Free_perm_type( MPI_FLOAT );
       MPIR_Free_perm_type( MPI_COMPLEX );
       MPIR_Free_perm_type( MPI_REAL );
       MPIR_Free_perm_type( MPI_2REAL );
       MPIR_Free_perm_type( MPI_DOUBLE_PRECISION );
       MPIR_Free_perm_type( MPI_DOUBLE_COMPLEX );
       MPIR_Free_perm_type( MPI_2DOUBLE_PRECISION );
       MPIR_Free_perm_type( MPI_LONG );
       MPIR_Free_perm_type( MPI_SHORT );
       MPIR_Free_perm_type( MPI_CHAR );
       MPIR_Free_perm_type( MPI_BYTE );
       MPIR_Free_perm_type( MPI_UNSIGNED_CHAR );
       MPIR_Free_perm_type( MPI_UNSIGNED_SHORT );
       MPIR_Free_perm_type( MPI_UNSIGNED_LONG );
       MPIR_Free_perm_type( MPI_UNSIGNED );
       MPIR_Free_perm_type( MPI_PACKED );
       MPIR_Free_perm_type( MPI_UB );
       MPIR_Free_perm_type( MPI_LB );
       MPIR_Free_perm_type( MPI_LONG_DOUBLE );
       MPIR_Free_perm_type( MPI_2COMPLEX );
       MPIR_Free_perm_type( MPI_2DOUBLE_COMPLEX );
       MPIR_Free_perm_type( MPI_2INT );
       MPIR_Free_perm_type( MPI_2INTEGER );
       MPIR_Free_perm_type( MPI_FLOAT_INT );
       MPIR_Free_perm_type( MPI_DOUBLE_INT );
       MPIR_Free_perm_type( MPI_LONG_INT );
       MPIR_Free_perm_type( MPI_SHORT_INT );
       MPIR_Free_perm_type( MPI_LONG_DOUBLE_INT );
       MPIR_Free_perm_type( MPI_LONG_LONG_INT );
    /* Free the parts of the structure types */

#if defined(HAVE_LONG_DOUBLE)
/*    {MPI_Datatype t = MPI_LONG_DOUBLE_INT;
    MPI_Type_free( &t );} */
/*     MPI_Type_free( &MPI_LONG_DOUBLE ); */
#endif
#if defined(HAVE_LONG_LONG_INT)
  /*  MPI_Type_free( &MPI_LONG_LONG_INT ); */
#endif
	   TR_POP;
}

/* This routine sets up a datatype that is specified as a small integer (val)
   and has a local reference
 */
void MPIR_Setup_base_datatype( 
	MPI_Datatype         val,
	struct MPIR_DATATYPE *lval,
	MPIR_NODETYPE        type,
	int                  size)
{
	TR_PUSH("MPIR_Setup_base_datatype");
    MPIR_SET_COOKIE(lval,MPIR_DATATYPE_COOKIE);
    lval->dte_type       = type;
    lval->committed      = 1;
    lval->is_contig      = 1;
    lval->lb             = 0;
    lval->ub             = size;
    lval->extent         = size;
    lval->size           = size;
    lval->align          = size;
    lval->stride         = size;
    lval->elements       = 1;
    lval->count          = 1;
    lval->blocklen       = 1;
    lval->basic          = 1;
    lval->permanent      = 1;
    lval->old_type       = lval;
    lval->ref_count      = 1;
    lval->flattened	 = NULL;
    lval->flatcount	 = 0;
    MPIR_RegPointerIdx( val, lval );
    lval->self           = val;
	TR_POP;
}

/* 
   This takes a datatype created with the datatype routines, copies the
   data into the "newtype" structure, and frees the old datatype storage 
 */
void MPIR_Setup_complex_datatype( 
	MPI_Datatype oldtype, 
	MPI_Datatype newtype,
	struct MPIR_DATATYPE *newtype_ptr)
{
    struct MPIR_DATATYPE *oldtype_ptr;
    oldtype_ptr = MPIR_ToPointer( oldtype );
    memcpy( newtype_ptr, oldtype_ptr, sizeof(struct MPIR_DATATYPE) );
    /* Should this be MPI_Type_free( &oldtype->self )? */
    MPIR_RmPointer( oldtype );
    MPIR_SBfree ( MPIR_dtes, oldtype_ptr );
    MPIR_Type_permanent ( newtype_ptr );
    MPIR_RegPointerIdx( newtype, newtype_ptr );
    newtype_ptr->self = newtype;
    MPI_Type_commit( &newtype );
}

void MPIR_Type_contiguous( 
	int cnt, 
	MPI_Datatype old_type, 
	struct MPIR_DATATYPE *newtype_ptr, 
	MPI_Datatype newtype )
{
    MPI_Datatype tmp_type;
    MPI_Type_contiguous( cnt, old_type, &tmp_type );
    MPIR_Setup_complex_datatype( tmp_type, newtype, newtype_ptr );
    /* MPI_Type_free( &tmp_type ); */
}


#ifndef MPID_NO_FORTRAN
/* 
   This routine computes the sizes of the Fortran data types.  It is 
   called from a Fortran routine that passes consequtive elements of 
   an array of the three Fortran types (character, real, double).
   Note that Fortran REQUIRES that integers have the same size as reals.
 */
void FORTRAN_API mpir_init_fsize_( 
	float *r1, 
	float *r2, 
	double *d1, 
	double *d2 )
{
/* MPIR_FSIZE_C = (int)(c2 - c1); */
/* Because of problems in passing characters, we pick the most likely size
   for now */
MPIR_FSIZE_C = sizeof(char);
MPIR_FSIZE_R = (int)( (char*)r2 - (char*)r1 );
MPIR_FSIZE_D = (int)( (char*)d2 - (char*)d1 );
}
#endif /* MPID_NO_FORTRAN */
