#ifndef _IMPI_CLIENT_H_
#define _IMPI_CLIENT_H_

#include "impi_common.h"
#include "impi_tcp.h"

typedef struct _IMPI_Data_struct
{
  IMPI_Int4  impi_rank;
  IMPI_Int4  impi_size;
  IMPI_Proc  **proc_world;
  IMPI_Uint4 num_c_procs[32];
  IMPI_Conn  partner_conn[32];

} IMPI_Data_struct;

extern IMPI_Data_struct IMPI_Data;

int IMPI_Client(int argc, char* argv[]);

#endif
