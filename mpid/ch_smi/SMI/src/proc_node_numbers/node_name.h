/* $Id$ */

#ifndef _SMI_NODENAME_H_
#define _SMI_NODENAME_H_

#include <env/general_definitions.h>
#include <env/smidebug.h>
#if (defined SOLARIS) || (defined SOLARIS_X86)
#include <netdb.h>
#else
#define MAXHOSTNAMELEN  256
#endif

smi_error_t SMI_Get_node_name (char *nodename, size_t *namelen);

#endif
