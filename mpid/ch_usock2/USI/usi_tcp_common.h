#ifndef _USI_TCP_COMMON_H
#define _USI_TCP_COMMON_H

#include "usi_tcp_basic.h"

#define _USI_VERBOSE

#define USI_TCP_LISTEN_PORT "54321"
#define USI_TCP_AGENT_STDOUT stdout
#define USI_VERBOSE_STDOUT   stdout

#define USI_TCP_MAX_ARG_SIZE 256
#define USI_TCP_MAX_ARG_NUM 32

typedef USI_Byte USI_tcp_Command_t;

typedef struct _USI_tcp_Environ_t
{
  USI_rank_t rank;
  USI_rank_t size;
} USI_tcp_Environ_t;


#define USI_TCP_CMD_INIT 0
#define USI_TCP_CMD_AGNT 1
#define USI_TCP_CMD_CLNT 2
#define USI_TCP_CMD_ACKN 3
#define USI_TCP_CMD_LSTN 4
#define USI_TCP_CMD_ARGV 5
#define USI_TCP_CMD_DONE 6
#define USI_TCP_CMD_FINI 7
#define USI_TCP_CMD_PERR 8

extern char USI_TCP_CMD[][];
			  
typedef struct _USI_tcp_Socket
{
  USI_tcp_basic_sockfd_t socket;
  struct sockaddr_in addr;
  USI_Byte active_send;
  USI_Byte active_recv;
} USI_tcp_Socket;


typedef struct _USI_tcp_Data
{
  USI_rank_t* rank_map;
  USI_tcp_Socket* socket_array;
  fd_set fd_set_all;
} USI_tcp_Data;

void USI_tcp_send_cmd(USI_tcp_basic_sockfd_t fd, int cmd);
void USI_tcp_send_arg(USI_tcp_basic_sockfd_t fd, char* arg);
int  USI_tcp_recv_cmd(USI_tcp_basic_sockfd_t fd);
void USI_tcp_recv_arg(USI_tcp_basic_sockfd_t fd, char** arg);

#endif
