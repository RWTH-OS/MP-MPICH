/* $Id: idstack.h,v 1.1 2004/03/19 22:14:20 joachim Exp $ */
#ifndef __IDSTACK_H__
#define __IDSTACK_H__

#include "env/general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif
    
/* patch to enable xemacs to tab correct */
#if 0
}
#endif


typedef struct idstack_node_t_ {
    int id;
    struct idstack_node_t_* next;
} idstack_node_t;

void _smi_idstack_push(int id);

int _smi_idstack_pop(void);

#ifdef __cplusplus
}
#endif

#endif

