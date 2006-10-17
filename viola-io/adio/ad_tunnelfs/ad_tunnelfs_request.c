/**************************************************************************
* TunnelFS                                                                * 
***************************************************************************
*                                                                         * 
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              * 
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
*                                                                         * 
* See COPYRIGHT notice in base directory for details                      * 
**************************************************************************/
/**************************************************************************
* File:         ad_tunnelfs_request.c                                     * 
* Description:  Request handling between IO and COMM Requests             *
*               (in MPICH IO-Requests and COMM-Requests for send/recv     *
*               actually differ!)                                         *
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "ad_tunnelfs.h"

/**
 * Chained list of communication requests
 */
struct tunnelfs_request_list_tD
{
    MPI_Request *request;
    struct tunnelfs_request_list_tD *next;
};

/**
 * Type alias for list structure
 */
typedef struct tunnelfs_request_list_tD tunnelfs_request_list_t;

/**
 * Pending communication requests
 */
typedef struct
{
    MPIO_Request *io_request;           /**< Pointer to MPI IO Request handle */
    tunnelfs_request_list_t *reqlist;   /**< Pointer to entry in internal list */
    void *buf;                          /**< associated buffer */
    int num_blocks;                     /**< number of total blocks */
    int pending;                        /**< number of pending blocks */
    int source;                         /**< source for data transfer */
    int tag;                            /**< MPI message tag used */
    MPI_Comm comm;                      /**< MPI communicator used */
}
tunnelfs_request_t;

static tunnelfs_request_t *tunnelfs_request_queue = NULL;
static int tunnelfs_request_queue_size = 0;
static int tunnelfs_request_queue_last = -1;

/**
 * Create a request for an MPICH IO request
 * @param request Pointer to MPI-IO request
 * @return internal id for io request
 */
int tunnelfs_request_create(MPIO_Request *request)
{
    if ((tunnelfs_request_queue_last < 0) ||
        (tunnelfs_request_queue_last == tunnelfs_request_queue_size))
    {
        tunnelfs_request_queue_size += 20;
        tunnelfs_request_queue =
            (tunnelfs_request_t *) calloc(tunnelfs_request_queue_size,
                                          sizeof(tunnelfs_request_t));
        if (tunnelfs_request_queue == NULL)
        {
            ERR(TUNNELFS_ERR_ALLOC);
        }
    }
    tunnelfs_request_queue_last++;
    tunnelfs_request_queue[tunnelfs_request_queue_last].io_request = request;

    return tunnelfs_request_queue_last;
}

/**
 * Delete a request for an MPICH IO request
 * @param request Pointer to MPI-IO request
 */
void tunnelfs_request_delete(MPIO_Request *request)
{
    int idx = -1;

    /* search request */
    idx = tunnelfs_request_get_idx(request);

    if (idx != -1)
    {
        /* if idx is valid check if all requests have been handled */
        if (tunnelfs_request_queue[idx].reqlist != NULL)
        {
            MPI_Status status;

            /* SOMETHING IS SERIOUSLY WRONG! Let's try to fix it */
            fprintf(stderr,
                    "WARNING: You are trying to deallocate incomplete communication requests. Something is seriously wrong in the message flow. Waiting for the requests to finish before dealolocation!\n");

            while (tunnelfs_request_queue[idx].reqlist->request != NULL)
            {
                tunnelfs_request_list_t *temp_req_ptr;

                /* wait for request to complete */
                MPI_Wait(tunnelfs_request_queue[idx].reqlist->request,
                         &status);

                /* free datastructure and go on to the next */
                temp_req_ptr = tunnelfs_request_queue[idx].reqlist->next;
                FREE(tunnelfs_request_queue[idx].reqlist);

                tunnelfs_request_queue[idx].reqlist = temp_req_ptr;
            }
        }

    }
    if (tunnelfs_request_queue_last > 0)
    {
        memcpy(&(tunnelfs_request_queue[idx]),
               &(tunnelfs_request_queue[tunnelfs_request_queue_last]),
               sizeof(tunnelfs_request_t));
    }
    tunnelfs_request_queue_last--;
}

void tunnelfs_request_attach(int idx, MPI_Request *request)
{
}

void tunnelfs_request_detach(int idx, MPI_Request *request)
{
}

int tunnelfs_request_get_idx(MPIO_Request *request)
{
    int idx = -1;

    while ((tunnelfs_request_queue_size > 0) &&
           (idx < tunnelfs_request_queue_size) &&
           (tunnelfs_request_queue[idx].io_request != request))
        idx++;

    if (idx < tunnelfs_request_queue_size)
        return -1;
    else
        return idx;
}

void *tunnelfs_request_get_buf(int idx)
{
    if ((idx < 0) || (idx >= tunnelfs_request_queue_size))
        ERR(TUNNELFS_ERR_OUT_OF_BOUNDS);

    return tunnelfs_request_queue[idx].buf;
}

int tunnelfs_request_get_num_blocks(int idx)
{
    if ((idx < 0) || (idx >= tunnelfs_request_queue_size))
        ERR(TUNNELFS_ERR_OUT_OF_BOUNDS);

    return tunnelfs_request_queue[idx].num_blocks;
}

int tunnelfs_request_get_pending(int idx)
{
    if ((idx < 0) || (idx >= tunnelfs_request_queue_size))
        ERR(TUNNELFS_ERR_OUT_OF_BOUNDS);

    return tunnelfs_request_queue[idx].pending;
}

int tunnelfs_request_get_source(int idx)
{
    if ((idx < 0) || (idx >= tunnelfs_request_queue_size))
        ERR(TUNNELFS_ERR_OUT_OF_BOUNDS);

    return tunnelfs_request_queue[idx].source;
}

int tunnelfs_request_get_tag(int idx)
{
    if ((idx < 0) || (idx >= tunnelfs_request_queue_size))
        ERR(TUNNELFS_ERR_OUT_OF_BOUNDS);

    return tunnelfs_request_queue[idx].tag;
}

MPI_Comm tunnelfs_request_get_comm(int idx)
{
    if ((idx < 0) || (idx >= tunnelfs_request_queue_size))
        ERR(TUNNELFS_ERR_OUT_OF_BOUNDS);

    return tunnelfs_request_queue[idx].comm;
}

void tunnelfs_request_set_buf(int idx, void *buf)
{
    if ((idx < 0) || (idx >= tunnelfs_request_queue_size))
        ERR(TUNNELFS_ERR_OUT_OF_BOUNDS);

    tunnelfs_request_queue[idx].buf = buf;
}

void tunnelfs_request_set_num_blocks(int idx, int num)
{
    if ((idx < 0) || (idx >= tunnelfs_request_queue_size))
        ERR(TUNNELFS_ERR_OUT_OF_BOUNDS);

    tunnelfs_request_queue[idx].num_blocks = num;
    tunnelfs_request_queue[idx].pending = num;
}

void tunnelfs_request_set_source(int idx, int source)
{
    if ((idx < 0) || (idx >= tunnelfs_request_queue_size))
        ERR(TUNNELFS_ERR_OUT_OF_BOUNDS);

    tunnelfs_request_queue[idx].source = source;
}

void tunnelfs_request_set_tag(int idx, int tag)
{
    if ((idx < 0) || (idx >= tunnelfs_request_queue_size))
        ERR(TUNNELFS_ERR_OUT_OF_BOUNDS);

    tunnelfs_request_queue[idx].tag = tag;
}

void tunnelfs_request_set_comm(int idx, MPI_Comm comm)
{
    if ((idx < 0) && (idx >= tunnelfs_request_queue_size))
        ERR(TUNNELFS_ERR_OUT_OF_BOUNDS);

    tunnelfs_request_queue[idx].comm = comm;
}
