/* $Id$ */

#ifndef _SMI_SENDRECV_H
#define _SMI_SENDRECV_H

#include "env/general_definitions.h"

#define SMI_MP_MAXSIZE (sizeof(smi_mp_packet_t))
#define SMI_MP_MAXDATA (128 - (2*sizeof(int)))

typedef struct smi_mp_packet_t_ {
  char data[SMI_MP_MAXDATA];
  int size;
  volatile int tag;
} smi_mp_packet_t;

smi_error_t _smi_init_mp(void);
smi_error_t _smi_finalize_mp(void);

smi_error_t SMI_Send(void *buf, int count, int dest);
smi_error_t SMI_Recv(void *buf, int count, int dest);

smi_error_t SMI_Isend(void *buf, int count, int dest);
smi_error_t SMI_Send_wait(int dest);

smi_error_t SMI_Sendrecv(void *send_buf, void *recv_buf, int count, int dest);

#endif /* __SENDRECV_H__ */
