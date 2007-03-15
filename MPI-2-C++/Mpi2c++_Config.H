// -*- c++ -*-
//
// Copyright 1997-1999, University of Notre Dame.
// Authors:  Jeremy G. Siek, Michael P. McNally, Jeffery M. Squyres, 
//           Andrew Lumsdaine
//
// This file is part of the Notre Dame C++ bindings for MPI
//
// You should have received a copy of the License Agreement for the
// Notre Dame C++ bindings for MPI along with the software;  see the
// file LICENSE.  If not, contact Office of Research, University of Notre
// Dame, Notre Dame, IN  46556.
//
// Permission to modify the code and to distribute modified code is
// granted, provided the text of this NOTICE is retained, a notice that
// the code was modified is included with the above COPYRIGHT NOTICE and
// with the COPYRIGHT NOTICE in the LICENSE file, and that the LICENSE
// file is distributed with the modified code.
//
// LICENSOR MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.
// By way of example, but not limitation, Licensor MAKES NO
// REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY
// PARTICULAR PURPOSE OR THAT THE USE OF THE LICENSED SOFTWARE COMPONENTS
// OR DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS, TRADEMARKS
// OR OTHER RIGHTS.
//

#ifndef _MPIPP_CONFIG_H
#define _MPIPP_CONFIG_H

#define _MPIPP_USEEXCEPTIONS_ 1
#define _MPIPP_DEBUG_         0

#define HAVE_BOOL             1 

#define SIZEOF_INT            4
#define SIZEOF_MPI2CPP_BOOL_T 1

#if SIZEOF_INT != SIZEOF_MPI2CPP_BOOL_T
#define _MPIPP_BOOL_NE_INT_   1
#else
#define _MPIPP_BOOL_NE_INT_   0
#endif

// Does our compiler support namespaces?
#define _MPIPP_USENAMESPACE_  1

// Compile for Profiling?
#define _MPIPP_PROFILING_     0

// What kind of signals do we have?
#define MPI2CPP_BSD_SIGNAL    0
#define MPI2CPP_SYSV_SIGNAL   0

// Is the ERR_PENDING constant defined?
#define HAVE_PENDING          0

// Can virtual functions return derived class instead
// of base class?
#define VIRTUAL_FUNC_RET      0

// For mpich, tell it to use correct MPI_Handler_function definition
// Later versions of MPICH do this already; the duplicate definition
// can cause trouble
// #define USE_STDARG 1

#if _MPIPP_PROFILING_
#define _REAL_MPI_ PMPI
#else
#define _REAL_MPI_ MPI
#endif

#if _MPIPP_USENAMESPACE_
#define _MPIPP_STATIC_ 
#define _MPIPP_EXTERN_ extern
#define _MPIPP_INLINE_ inline
#else
#define _MPIPP_STATIC_ static
#define _MPIPP_EXTERN_
#define _MPIPP_INLINE_ 
#endif

#if MPI2CPP_HAVE_BOOL
typedef bool MPI2CPP_BOOL_T;
#define MPI2CPP_FALSE false
#define MPI2CPP_TRUE true
#else
enum MPI2CPP_BOOL_T { MPI2CPP_FALSE, MPI2CPP_TRUE };
#ifdef bool
#undef bool
#endif
#ifdef false
#undef false
#endif
#ifdef true
#undef true
#endif
#define bool MPI2CPP_BOOL_T
#define false MPI2CPP_FALSE
#define true MPI2CPP_TRUE
#endif

#define MPI2CPP_BSD_SIGNAL 0
#define MPI2CPP_SYSV_SIGNAL 0

// Flags for FORTRAN, optional C, and optional Fortran datatypes

#define FORTRAN 0
#define ALL_OPTIONAL_FORTRAN 0
#define SOME_OPTIONAL_FORTRAN 0
#define OPTIONAL_C 0

//
// Architecture/OS's that will need flags in the test suite to
// ignore certain tests
//
 
#define LAM61 0
#define LAMUNKNOWN 0

#define MPICH1013 0
#define MPICH110 0
#define MPICH111 0
#define MPICH112 1
#define MPICHUNKNOWN 0
 
#define IBM21014 0
#define IBM21015 0
#define IBM21016 0
#define IBM21017 0
#define IBM21018 0
#define IBMUNKNOWN 0
#define IBM_SP (IBM21014 | IBM21015 | IBM21016 | IBM21017 | IBM21018 | IBMUNKNOWN)
 
#define SGI20 0
#define SGI30 0
#define SGIUNKNOWN 0

#define HPUX0102 0
#define HPUX0103 0
#define HPUX0105 0
#define HPUXUNKNOWN 0

#define CRAY 0
#define CRAY1104 0
#define CRAYUNKNWON 0

#define G_PLUS_PLUS 0

#define ATTR int
#define MPI2CPP_ATTR int

// #$%@#%@#%@#% AIX!!!!

#define MPI2CPP_AIX 0

#ifdef _MSC_VER
#pragma warning( disable : 4800 )
#endif

#endif
