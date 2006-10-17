/**************************************************************************
* MEMFS                                                                   * 
***************************************************************************
*                                                                         * 
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              * 
*                                                                         * 
* See COPYRIGHT notice in base directory for details                      * 
**************************************************************************/
/**************************************************************************
* File:         ad_memfs_service.h                                        * 
* Description:                                                            *
* Author(s):    Jan Seidel <Jan.Seidel@fh-bonn-rhein-sieg.de>             * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/ 

#ifndef AD_MEMFS_SERVICE_H
#define AD_MEMFS_SERVICE_H


#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ad_memfs.h"
#include "ad_memfs_functions.h"

/* Include for memory debugger DMALLOC */
#if 0
#ifdef DMALLOC
#include "dmalloc.h"
#endif
#endif

void *memfs_service(void *servercount);

#endif 





