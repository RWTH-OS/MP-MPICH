#ifndef _USI_BASIC_H
#define _USI_BASIC_H

#include "usi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#else
#include <Ws2tcpip.h>
#include <Iphlpapi.h>
#include <Iprtrmib.h>
#endif

#define USI_MAX_ARG_LENGTH 256
#define USI_MAX_ARG_NUMBER  64

typedef size_t  USI_basic_size_t;
typedef ssize_t USI_basic_ssize_t;
typedef void*   USI_basic_pointer_t;

#ifndef _WIN32
#include <time.h>
#define USI_basic_sleep(x) { struct timespec y; y.tv_sec=0; y.tv_nsec=x*1000000; nanosleep(&y, NULL); }
#else
#define USI_basic_sleep(x) Sleep(x)
#endif

#ifndef _WIN32
 typedef int USI_basic_sockfd_t;
#else
 typedef SOCKET USI_basic_sockfd_t;
#endif

#ifndef _WIN32
 typedef socklen_t USI_basic_socklen_t;
#else
 typedef int USI_basic_socklen_t;
#endif

#ifndef _WIN32
#define USI_basic_error_code errno
#else
#define USI_basic_error_code WSAGetLastError()
#endif

USI_basic_sockfd_t USI_basic_socket(int family, int type);
int USI_basic_connect(USI_basic_sockfd_t sockfd,  struct sockaddr *addr);
int USI_basic_bind(USI_basic_sockfd_t sockfd, struct sockaddr *addr);
int USI_basic_listen(USI_basic_sockfd_t sockfd, int backlog);
USI_basic_sockfd_t USI_basic_accept(USI_basic_sockfd_t sockfd, struct sockaddr *addr, USI_basic_socklen_t* len);

USI_basic_ssize_t USI_basic_send(USI_basic_sockfd_t socket, USI_basic_pointer_t buf, USI_basic_size_t len);
USI_basic_ssize_t USI_basic_recv(USI_basic_sockfd_t socket, USI_basic_pointer_t buf, USI_basic_size_t len);

int   USI_basic_check_argv  (int* argc, char** argv[], char* string, char* end);
char* USI_basic_pop_argv2str(int* argc, char** argv[], char* string, char* end);
int   USI_basic_pop_argv2int(int* argc, char** argv[], char *string, char* end);
char* USI_basic_get_argv2str(int* argc, char** argv[], char* string, char* end);
int   USI_basic_get_argv2int(int* argc, char** argv[], char *string, char* end);

void USI_basic_Error(const char*);

#endif
