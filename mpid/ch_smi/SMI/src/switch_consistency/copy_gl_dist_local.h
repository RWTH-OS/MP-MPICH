/* $Id: copy_gl_dist_local.h,v 1.1 2004/03/19 22:14:21 joachim Exp $ */

#ifndef __COPY_GL_DIST_LOCAL_H
#define __COPY_GL_DIST_LOCAL_H

#ifdef __cplusplus
extern "C" {
#endif


smi_error_t _smi_copy_globally_distributed_to_local(int global_region_id, int local_region_id);

#ifdef __cplusplus
}
#endif


#endif
