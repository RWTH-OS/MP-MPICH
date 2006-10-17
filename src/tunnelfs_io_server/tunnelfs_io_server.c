/**************************************************************************
* TunnelFS Server                                                         * 
***************************************************************************
*                                                                         * 
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              * 
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
*                                                                         * 
* See COPYRIGHT notice in base directory for details                      * 
**************************************************************************/
/**************************************************************************
* File:         tunnelfs_io_server.c                                      * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Descritpion:  Wrapper for the main function to create a standalone      *
*               io server binary                                          *
**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "pario_threads.h"

int tunnelfs_srv(int argc, char **argv);

/**
 * Declared in adio/common/pario_threads.c
 */
extern double pario_lock_time;

int main(int argc, char **argv)
{
    int err;
    double global_start = 0.;
    double global_time = 0.;

    global_start = ptWtime();
    err = tunnelfs_srv(argc, argv);
    global_time = ptWtime() - global_start;

#ifdef LOCK_TIMING
    fprintf(stderr,
            "\n\nTIME MEASUREMENT (global/lock/rel): %es / %es / %3.2lf%%\n",
            global_time, pario_lock_time,
            (double) (pario_lock_time / global_time) * 100);;
#endif
    return err;
}
