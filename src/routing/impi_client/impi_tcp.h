
#ifndef _IMPI_TCP_H_
#define _IMPI_TCP_H_

#include "impi_common.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

typedef struct _IMPI_Conn
{
  int    socket;
  struct sockaddr_in local_addr;
  struct sockaddr_in remote_addr;
} IMPI_Conn;

in_addr_t IMPI_TCP_Str2host(char* string);
in_port_t IMPI_TCP_Str2port(char* string);

int IMPI_TCP_Socket(int family, int type);
void IMPI_TCP_Connect(int sockfd,  struct sockaddr_in addr);
void IMPI_TCP_Bind(int sockfd, struct sockaddr_in addr);
void IMPI_TCP_Listen(int sockfd, int backlog);
int IMPI_TCP_Accept(int sockfd, struct sockaddr_in *client_addr);
size_t IMPI_TCP_Recv(int socket, void *buf, size_t len);
size_t IMPI_TCP_Send(int socket, void *buf, size_t len);
int IMPI_TCP_Select(int socket, int blocking);

#endif
