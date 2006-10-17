/* $Id: safety.h,v 1.1 2004/03/19 22:14:15 joachim Exp $ */

#include "synchronization/store_barrier.h"
#include "general_definitions.h"

/*
    We implement a 'save barrier' regarding two issues:
    
    The first one is data coherency. To guarantee this, we flush all write buffer
    when entering the barrier and we flush all read buffers before leaving it.
 
    Second regards correctness. We have to deal with the possibility that writes
    are discarded and that reads not not return the proper value. To deal with 
    discarded writes, after each written datum, it is read to check if it properly
    arrived at the destination side. 
 
    Dealing with improper reads (also with those thar are required to check for
    discarded writes) is more complicated. The key idea is to be able to know in
    advance which return values are possible. If those are very few, one can be 
    quite sure that the correct datum has been returned. Furthermore, we should
    omit to read values which could be something like a magic number ('0') and to
    read several variables which should possess the same value. In the case that
    the system returns a faulty quantity, it is probable that it is a magic number
    or the quantity just returned before.
    To do so, we start the barrier counters not at '0' and not all with the same
    quantity. The counter of process i starts with '123+i'. The value of process j,
    process i expects when polling for it to enter the barrier are:
 
    MyBarrierCounter + (j-i), MyBarrierCounter + (j-i) - 1, MyBarrierCounter + (j-i) + 1
 
    Any more ideas to improve security ??
  */

#ifndef __SAFETY_H
#define __SAFETY_H


/*****/
/* This to avoid "Magic Numbers" */
/*****/

#define DEP_TRUE(depend_on) (124+(depend_on))  
#define DEP_FALSE(depend_on) (123+(depend_on))

/*****/
/* This to ensure the write stream buffer to be written back */
/*****/

#define SEC_INDEX(Index) (INTS_PER_STREAM*(Index)+INTS_PER_STREAM-1)
#define SEC_LOCATION(Base,Index) ((Base)[SEC_INDEX(Index)])


/*****/
/* This to ensure the Data written has arrived correct */
/*****/

#define SEC_SET(a,b)  do {(a) = (b);  _smi_local_store_barrier();} while ((a) != (b));
#define SEC_SET_P(a,b)  do {(a) = (b);} while ((a) != (b));


/*****/
/* A Test for plausibility of Counter Values (true if sourcevalue is not plausible */
/*****/

#define CNT_TRASH(source,myvalue) (abs((source)-(myvalue))>1)


/****/
/*Read a Value until it is plausible */
/*****/

int _smi_SECRead(volatile int *source,int valid_start, int valid_end);

#endif


     











