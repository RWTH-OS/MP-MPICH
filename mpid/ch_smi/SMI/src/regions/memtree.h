/* $Id$ */
#ifndef _SMI_MEMTREE_H
#define _SMI_MEMTREE_H

#include "env/general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif
    
/* patch to enable xemacs to tab correct */
#if 0
}
#endif

typedef struct memtree_memarea_t_ {
    char* pStart;
    size_t iSize;
} memtree_memarea_t;

typedef enum memtree_hook_t_ {
    hk_null,
    hk_left,
    hk_right
} memtree_hook_t;

typedef struct memtree_node_t_ {
    memtree_memarea_t MemArea;
    int iSegmentId;
    int iLeftDepth;
    int iRightDepth;
    struct memtree_node_t_* pLeft;
    struct memtree_node_t_* pRight;
    struct memtree_node_t_* pParent;
    memtree_hook_t Hook;
} memtree_node_t;
    
    typedef memtree_node_t* memtree_t;

memtree_t _smi_memtree_empty(void);

memtree_t _smi_memtree_insert(memtree_t root, memtree_memarea_t* pMemarea, int segid);

memtree_t _smi_memtree_delete(memtree_t root, memtree_memarea_t* pMemarea);

int _smi_memtree_getsegid(memtree_t root, memtree_memarea_t* pMemarea);

boolean _smi_memtree_member(memtree_t root, memtree_memarea_t* pMemarea);

void _smi_memtree_kill(memtree_t* pRoot);

extern memtree_t _smi_memtree;

#ifdef __cplusplus
}
#endif

#endif
