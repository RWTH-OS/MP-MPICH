
#ifndef _IMPI_HOST_H_
#define _IMPI_HOST_H_

#include "impi_common.h"
#include "impi_tcp.h"

int IMPI_Export_packets(IMPI_Conn conn);
int IMPI_Import_packets(IMPI_Conn conn, int *lrank_to_grank);

#endif
