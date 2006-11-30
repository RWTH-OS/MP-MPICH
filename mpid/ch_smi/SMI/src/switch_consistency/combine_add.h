/* $Id$ */

#ifndef __COMBINE_ADD_H
#define __COMBINE_ADD_H


#include "env/general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif


/********************************************************************************/
/***                                                                          ***/
/*** Add element-wise local arrays of all participating processes that are    ***/
/*** contained in each process'  region "local"  so that the result is        ***/
/*** afterwards contained in the globally shared region "shared".             ***/
/***                                                                          ***/
/*** int comb_mode  'ored' flags that describe what to do in mode detail:     ***/
/***                FLOATINGPOINT    : the values are floating point quantities        ***/
/***                FIXPOINT : all values are integer quantities               ***/
/***                HIGHPRECISION    : 8-byte values (if not set: 4-byte values        ***/
/***                SPARSE  : each process array contains just a small        ***/
/***                          fraction of non-zero values                     ***/
/***                                                                          ***/
/***                in all these above cases, param1 states the total number  ***/
/***                elements in the arrays, starting right at the beginning   ***/
/***                of the regions                                            ***/
/***                                                                          ***/
/***                BAND    : in each                                         ***/
/********************************************************************************/
smi_error_t _smi_combine_add(region_t* shared, region_t* local, int param1,
		    int param2, int comb_mode);
 
#ifdef __cplusplus
}
#endif

#endif
