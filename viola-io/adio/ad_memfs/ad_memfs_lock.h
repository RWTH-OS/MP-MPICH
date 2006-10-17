/**************************************************************************
* MEMFS                                                                   *
***************************************************************************
*                                                                         *
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*               Germany                                                   *
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              *
*                                                                         *
* See COPYRIGHT notice in base directory for details                      *
**************************************************************************/
/**************************************************************************
* File:         ad_memfs_lock.h                                           *
* Description:  							  *
*                                                                         *
* Author(s):    Marcel Birkner <Marcel.Birkner@fh-bonn-rhein-sieg.de>     *
*                                                                         *
* Last change:                                                            *
*                                                                         *
* Changelog:                                                              *
**************************************************************************/

#include <string.h>
#include <stdio.h>
#include <math.h>


/* Main Thread function */
void set_lock(int fh, int *blocks, int size, int num_server, int *error);
									/* fh=filehandle, blocks[]=blocks to lock, 
									 * size=number of blocks that need to be locked
									 * num_server=Number of MEMFS Server
									 */

/* Main Thread function */
void remove_lock(int fh, int *blocks, int size, int num_server, int *error);
									/* fh=filehandle, blocks[]=blocks to lock,
                                                                         * size=number of blocks that need to be locked
                                                                         * num_server=Number of MEMFS Server
                                                                         */


