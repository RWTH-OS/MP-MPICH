#ifndef _USI_TCP_BASIC_H
#define _USI_TCP_BASIC_H

#include "usi_basic.h"

typedef USI_basic_size_t USI_tcp_basic_size_t;
typedef USI_basic_ssize_t USI_tcp_basic_ssize_t;
typedef USI_basic_pointer_t USI_tcp_basic_pointer_t;
typedef USI_basic_sockfd_t USI_tcp_basic_sockfd_t;

in_addr_t USI_tcp_basic_str2host(char* string);
in_port_t USI_tcp_basic_str2port(char* string);
in_addr_t USI_tcp_basic_local_IP(void);

USI_tcp_basic_sockfd_t USI_tcp_basic_socket(int family, int type);
void USI_tcp_basic_connect(USI_tcp_basic_sockfd_t sockfd,  struct sockaddr_in addr);
void USI_tcp_basic_bind(USI_tcp_basic_sockfd_t sockfd, struct sockaddr_in addr);
void USI_tcp_basic_listen(USI_tcp_basic_sockfd_t sockfd, int backlog);
USI_tcp_basic_sockfd_t USI_tcp_basic_accept(USI_tcp_basic_sockfd_t sockfd, struct sockaddr_in *client_addr);

USI_tcp_basic_ssize_t USI_tcp_basic_send(USI_tcp_basic_sockfd_t socket, USI_tcp_basic_pointer_t buf, USI_tcp_basic_size_t len);
USI_tcp_basic_ssize_t USI_tcp_basic_recv(USI_tcp_basic_sockfd_t socket, USI_tcp_basic_pointer_t buf, USI_tcp_basic_size_t len);

#if 0
char* USI_tcp_basic_argv2str(int* argc, char** argv[], char *string);
char* USI_tcp_basic_argv2str2(int* argc, char** argv[], char *string);
int   USI_tcp_basic_argv2int(int* argc, char** argv[], char *string);
int   USI_tcp_basic_argv2int2(int* argc, char** argv[], char *string);
int   USI_tcp_basic_get_arg(int* argc, char** argv[], char *string);
#endif 

void USI_tcp_basic_Error(const char*);

#endif
