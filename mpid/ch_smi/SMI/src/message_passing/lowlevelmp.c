/* $Id$ */

#ifdef WIN32
#include <windows.h>
#define FRIENDLY_POLLING
#endif

#include "env/smidebug.h"

#include <stdlib.h>
#include <stdio.h>

#include "lowlevelmp.h"
#include "env/general_definitions.h"
#include "startup/startup.h"
#include "env/safety.h"
#include "utility/statistics.h"
#ifdef FRIENDLY_POLLING
#include <sched.h>
#endif

static int my_node_rank;
static int nbr_nodes;
static int my_local_rank;
static int nbr_local_procs;
static int my_rank;
static int nbr_procs;
static int volatile *sgmt_addr;
static int volatile *loc_sgmt_addr;
static int barrier_cnt = 0;
static int local_barrier_cnt = 0;

#ifdef WIN32
#pragma optimize( "",off)
#endif

/*****************************************************************************/
/* This function reads a specified data range (starting from 'from_adr' with */
/* size 'elem_count*sizeof(int)') and copies it to the data range starting   */
/* at 'to_adr'. For security reasons, it is assumed, that the first int      */
/* after the real data range contains a 4-byte CRC, that is checked. If the  */
/* transmission failed, it is done once more.                                */
/*****************************************************************************/
static void _smi_read_with_crc(volatile int *from_adr, int *to_adr, size_t elem_count)
{
    int             crc;
    size_t             i;
    
    _smi_flush_read_buffers();
    
    do {
	crc = 0;
	for (i = 0; i < elem_count; i++) {
	    to_adr[i] = from_adr[i];
	    crc += to_adr[i];
	}
	crc += 13;
    }
    while (crc != from_adr[elem_count] && _smi_ll_sgmt_ok);
}


/*****************************************************************************/
/* This function writes a specified data range (starting from 'from_adr' with */
/* size 'elem_count*sizeof(int)') to the data range starting at 'to_adr'. For */
/* security reasons, a CRC is computed and stored thereafter. All written    */
/* data is checked against the original until the transmission succeds.      */
/*****************************************************************************/
static void  _smi_write_with_crc(int *from_adr, volatile int *to_adr, size_t elem_count)
{
    int             crc, ok;
    size_t			i;

    do {
	crc = 0;
	ok = 1;
	for (i = 0; i < elem_count; i++) {
	    to_adr[i] = from_adr[i];
	    crc += from_adr[i];
	}
	crc += 13;
	to_adr[elem_count] = crc;
	
	_smi_flush_write_buffers();
	
	for (i = 0; i < elem_count; i++)
	    if (from_adr[i] != to_adr[i])
		ok = 0;
	if (to_adr[elem_count] != crc)
	    ok = 0;
    } while (ok == 0 && _smi_ll_sgmt_ok);
}



void _smi_ll_init(int NodeRank, int NbrNodes, int volatile* BaseAddr,
		   int LocalRank, int NbrLocalProcs, 
		   int volatile* LocBaseAddr,
		   int GlobalRank, int NbrProcs)
{
    DSECTION("_smi_init_mpi");
    
    DSECTENTRYPOINT;
    
    my_node_rank = NodeRank;
    DNOTICEI("my_node_rank=", my_node_rank);
    
    nbr_nodes = NbrNodes;
    DNOTICEI("nbr_nodes=", nbr_nodes);
   
    /* if there is only one node */ 
    if (NbrNodes<2) 
        /* make sure, that the shared memory is used */
        /* this is a compatibility patch             */
        sgmt_addr = LocBaseAddr;
    else
        sgmt_addr = BaseAddr;
    DNOTICEP("sgmt_addr", sgmt_addr);
    
    my_local_rank = LocalRank;
    DNOTICEI("my_local_rank=", my_local_rank);
    
    nbr_local_procs = NbrLocalProcs;
    DNOTICEI("nbr_local_procs", nbr_local_procs);
    
    loc_sgmt_addr = LocBaseAddr;
    DNOTICEP("loc_sgmt_addr", loc_sgmt_addr);
    
    my_rank = GlobalRank;
    DNOTICEI("my_rank", my_rank);
    
    nbr_procs = NbrProcs;
    DNOTICEI("nbr_procs",nbr_procs);
    
#if TWO_STEP_BARRIER
    barrier_cnt = DEP_FALSE(my_node_rank);
#else
    barrier_cnt = DEP_FALSE(my_rank);
#endif
    DNOTICEI("barrier_cnt", barrier_cnt);

    local_barrier_cnt = DEP_FALSE(my_local_rank);
    DNOTICEI("local_barrier_cnt", local_barrier_cnt);

    _smi_ll_sgmt_ok = TRUE;

    DSECTLEAVE;
}

/*
 * We implement a 'save barrier' regarding two issues:
 * 
 * The first one is data coherency. To guarantee this, we flush all write buffer
 * when entering the barrier and we flush all read buffers before leaving it.
 * 
 * Second regards correctness. We have to deal with the possibility that writes
 * are discarded and that reads not not return the proper value. To deal with
 * discarded writes, after each written datum, it is read to check if it
 * properly arrived at the destination side.
 * 
 * Dealing with improper reads (also with those thar are required to check for
 * discarded writes) is more complicated. The key idea is to be able to know
 * in advance which return values are possible. If those are very few, one
 * can be quite sure that the correct datum has been returned. Furthermore,
 * we should omit to read values which could be something like a magic number
 * ('0') and to read several variables which should possess the same value.
 * In the case that the system returns a faulty quantity, it is probable that
 * it is a magic number or the quantity just returned before. To do so, we
 * start the barrier counters not at '0' and not all with the same quantity.
 * The counter of process i starts with '123+i'. The value of process j,
 * process i expects when polling for it to enter the barrier are:
 * 
 * barrier_cnt + (j-i), barrier_cnt + (j-i) - 1, barrier_cnt +
 * (j-i) + 1
 * 
 * Any more ideas to improve security ??
 */
static int _smi_barrier_basic(int rank, int size, int* count, volatile int* shmem, int do_flush) {
    int i;
    int ChkSum;
    
    /* The barrier works much faster if no flushing is done at all      */
    /* This does not seem to influence the functionality of the barrier */
    do_flush = FALSE;

    if (size > 1 && _smi_ll_sgmt_ok) {
	(*count) += 1;
	do {
	    shmem[INTS_PER_STREAM * rank] = *count;
	    if (do_flush) {
		_smi_flush_write_buffers();
		_smi_flush_read_buffers();
	    }
	} while (shmem[INTS_PER_STREAM * rank] != *count);
	
	if (do_flush) 
	    _smi_flush_read_buffers();
	
	for (i = 0; i < size; i++)
	    do {
#ifdef FRIENDLY_POLLING
		sched_yield();
#endif 
		ChkSum = shmem[INTS_PER_STREAM * i] + (rank - i);
	    } while (ChkSum < *count
		     || (ChkSum != (*count) - 1
			 && ChkSum != (*count) 
			 && ChkSum != (*count) + 1
			 )
		);
	
	if (do_flush) 
	    _smi_flush_read_buffers();
	
    }
    
    return(0);
}

int _smi_ll_barrier(void)
{
    REMDSECTION("_smi_ll_barrier");
    int             MemOffset;
    
    SMI_STAT_ENTRY(barrier);
    DSECTENTRYPOINT;
    
    MemOffset = BASESGMT_OFFSET_BARRIER + 15;
    
    if (_smi_ll_sgmt_ok) {
#if TWO_STEP_BARRIER 
	/* local shmem-Barrier */
	DNOTICE("local Shmem-Barrier");
	_smi_barrier_basic(my_local_rank, nbr_local_procs, &local_barrier_cnt, 
			   loc_sgmt_addr + MemOffset, FALSE);
	
	/* SCI-Barrier across nodes */
	DNOTICE("SCI-Barrier across nodes");
	_smi_barrier_basic(my_node_rank, nbr_nodes, &barrier_cnt, 
			   sgmt_addr + MemOffset, TRUE);
#else
	DNOTICE("old barrier across nodes");
	_smi_barrier_basic(my_rank, nbr_procs, &barrier_cnt, 
		       sgmt_addr + MemOffset, TRUE);
#endif
    }
	
    SMI_STAT_EXIT(barrier);
    DSECTLEAVE;
    return(0);
}

int _smi_ll_bcast(int *buffer, size_t count, int sender_rank, int my_rank)
{
    int             MemOffset = BASESGMT_OFFSET_DATASHARE;
    SMI_STAT_ENTRY(bcast);
    
    if (_smi_ll_sgmt_ok) {
	_smi_ll_barrier();
	
	if (sender_rank == my_rank) {
	    _smi_write_with_crc(buffer, &sgmt_addr[MemOffset], count);
	    
	    _smi_ll_barrier();
	} else {
	    _smi_ll_barrier();
	    
	    _smi_read_with_crc(&sgmt_addr[MemOffset], buffer, count);
	    
	}
	
	_smi_ll_barrier();
    }

    SMI_STAT_EXIT(bcast);
    return (0);
}


int _smi_ll_allgather(int *SendBuf, size_t count, int *RecvBuf, int my_rank)
{
    int             i;
    int             MemOffset = BASESGMT_OFFSET_DATASHARE;
    SMI_STAT_ENTRY(allgather);
    
    if (_smi_ll_sgmt_ok) {
	_smi_ll_barrier();
	
	_smi_write_with_crc(SendBuf, &sgmt_addr[MemOffset + (count + 1) * my_rank], count);
	
	_smi_ll_barrier();
	
	for (i = 0; i < nbr_procs; i++)
	    _smi_read_with_crc(&sgmt_addr[MemOffset + (count + 1) * i], &RecvBuf[i * count], count);
    
	_smi_ll_barrier();
    }

    SMI_STAT_EXIT(allgather);
    return (0);
}

boolean _smi_ll_all_true(boolean bTest)
{
    int             i;
    int             MemOffset = BASESGMT_OFFSET_DATASHARE;
    boolean         RetVal;
    SMI_STAT_ENTRY(alltrue);
    
    if (_smi_ll_sgmt_ok) {
	_smi_ll_barrier();
	
	_smi_write_with_crc(&bTest, &(sgmt_addr[MemOffset + 2 * my_rank]), 1);
	
	_smi_ll_barrier();
	
	for (i = 0; i < nbr_nodes; i++) {
	    _smi_read_with_crc(&(sgmt_addr[MemOffset + 2 * i]), &RetVal, 1);
	if (RetVal == false)
	    return (false);
	}
    }
    
    SMI_STAT_EXIT(alltrue);
    return (TRUE);
}

boolean _smi_ll_all_equal_pointer(void *Pointer)
{
    int             i;
    int             MemOffset = BASESGMT_OFFSET_DATASHARE;
    void           *pComp;
    int             len = sizeof(void *) / sizeof(int);
    
    if (_smi_ll_sgmt_ok) {
	_smi_ll_barrier();
	
	_smi_write_with_crc((int *) &Pointer, (int *) &(sgmt_addr[MemOffset + (1 + len) * my_rank]), len);
	
	_smi_ll_barrier();
	
	for (i = 0; i < nbr_procs; i++) {
	    _smi_read_with_crc((int *) &(sgmt_addr[MemOffset + (i + len) * i]), (int *) &pComp, len);
	    if (pComp != Pointer)
		return (false);
	}
    }
    
    return (TRUE);
}

static int _smi_ll_send_recv(int *SendBuf, int count, int *RecvBuf, int sender, int receiver, int my_rank)
{
    int             MemOffset = BASESGMT_OFFSET_DATASHARE;
    
    if (_smi_ll_sgmt_ok) {
	if (my_rank == sender) {
	    _smi_write_with_crc(SendBuf, &(sgmt_addr[MemOffset]), count);
	}
	_smi_ll_barrier();
	if (my_rank == receiver) {
	    _smi_read_with_crc(&(sgmt_addr[MemOffset]), RecvBuf, count);
	}
	_smi_ll_barrier();
    }
    
    return (0);
}


int _smi_ll_commrank(int *rank)
{
    if (rank != NULL)
	*rank = my_rank;
    return (0);
}


int _smi_ll_commsize(int *size)
{
    if (size != NULL)
	*size = nbr_procs;
    return (0);
}


#ifdef WIN32
#pragma optimize( "",on)
#endif
