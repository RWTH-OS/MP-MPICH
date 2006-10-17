/**************************************************************************
* VIOLA Parallel IO (ParIO)                                               *
***************************************************************************
*                                                                         *
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              *
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
*                                                                         *
* See COPYRIGHT notice in base directory for details                      *
***************************************************************************
***************************************************************************
* File:         pario_map.h                                               *
* Description:  Mapping between TUNNELFS_COMM_WORLD                       *
*               MPI_COMM_META_REDUCED                                     *
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          *
**************************************************************************/
#ifndef _PARIO_MAP_H
#define _PARIO_MAP_H

/**
 * Initialize mapping structure between communicators
 */
void pario_init_map();

/**
 * Get the rank of process in MPI_COMM_META_REDUCED for provided rank in
 * TUNNELFS_COMM_WORLD
 * @param rank rank of MPI process in TUNNELFS_COMM_WORLD
 * @return corresponding rank of MPI process in MPI_COMM_META_REDUCED
 */
int pario_map_w2r(int rank);

/**
 * Get the rank of process in TUNNELFS_COMM_WORLD for provided rank in
 * MPI_COMM_META_REDUCED
 * @param rank rank of MPI process in MPI_COMM_META_REDUCED
 * @return corresponding rank of MPI process in TUNNELFS_COMM_WORLD
 */
int pario_map_r2w(int rank);

#endif
