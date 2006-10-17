#ifndef _IMPI_ROUTER_H_
#define _IMPI_ROUTER_H_

#include "impi_common.h"

int IMPI_Router(int argc, char* argv[]);


#define IMPI_Gateway_recv(src_comm_lrank, dest_grank, tag, length, buffer) \
        IMPI_Gateway_export(src_comm_lrank, dest_grank, tag, length, buffer)

#define IMPI_Gateway_check(src_comm_lrank, dest_grank, tag, length) \
        IMPI_Gateway_export(src_comm_lrank, dest_grank, tag, length, NULL)

int IMPI_Gateway_export(int *src_comm_lrank, int *dest_grank, int *tag, size_t *length, void **buffer);


#define IMPI_Tunnel_send(src_comm_lrank, dest_grank, tag, length, buffer) \
        IMPI_Tunnel_import(src_comm_lrank, dest_grank, tag, length, & buffer, 0)

#define IMPI_Tunnel_buffer(src_comm_lrank, dest_grank, tag, length, buffer) \
        IMPI_Tunnel_import(src_comm_lrank, dest_grank, tag, length, buffer, 1)

int IMPI_Tunnel_import (int src_comm_lrank, int dest_grank, int tag, size_t length, void **buffer, int get_buffer_flag);

#endif
