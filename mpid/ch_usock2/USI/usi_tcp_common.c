#include "usi_tcp_common.h"

char USI_TCP_CMD_TABLE[9][18]={ "USI_TCP_CMD_INIT",
				 "USI_TCP_CMD_AGNT",
				 "USI_TCP_CMD_CLNT",
				 "USI_TCP_CMD_ACKN",
				 "USI_TCP_CMD_LSTN",
				 "USI_TCP_CMD_ARGV",
				 "USI_TCP_CMD_DONE",
				 "USI_TCP_CMD_FINI",
				 "USI_TCP_CMD_PERR"};

void USI_tcp_send_cmd(USI_tcp_basic_sockfd_t fd, int cmd)
{
  char buffer[USI_TCP_MAX_ARG_SIZE];

   if(cmd<9) strncpy(buffer, USI_TCP_CMD_TABLE[cmd], USI_TCP_MAX_ARG_SIZE);
/*  else*/
  USI_tcp_basic_send(fd, (USI_tcp_basic_pointer_t)buffer, USI_TCP_MAX_ARG_SIZE);
}

void USI_tcp_send_arg(USI_tcp_basic_sockfd_t fd, char* arg)
{
  unsigned char length; /* USI_BYTE ? */

  length=(char)strlen(arg)+1;
  USI_tcp_basic_send(fd, (USI_tcp_basic_pointer_t)&length, 1);

  if(length>0) USI_tcp_basic_send(fd, (USI_tcp_basic_pointer_t)arg, length);

/*
  printf(">>> send ARGV: %s\n", arg);
*/

  /* return length ? */
}

int USI_tcp_recv_cmd(USI_tcp_basic_sockfd_t fd)
{
  int i;
  char buffer[USI_TCP_MAX_ARG_SIZE];

  USI_tcp_basic_recv(fd, (USI_tcp_basic_pointer_t)buffer, USI_TCP_MAX_ARG_SIZE);

  for(i=0; i<8; i++) /* keiner bis PERR */
  {
    if(strncmp(buffer, USI_TCP_CMD_TABLE[i], USI_TCP_MAX_ARG_SIZE)==0)
    {
/*
      printf("<<< RECV COMMAND %s\n", USI_TCP_CMD_TABLE[i]);
*/
      return i;
    }
  }

  exit(-1);
}

void USI_tcp_recv_arg(USI_tcp_basic_sockfd_t fd, char** arg)
{
  unsigned char length;
  char buffer[USI_TCP_MAX_ARG_SIZE];

  USI_tcp_basic_recv(fd, (USI_tcp_basic_pointer_t)&length, 1);

  if(length>0)
  { 
    (*arg)=(char*)malloc(length*sizeof(char));
    USI_tcp_basic_recv(fd, (USI_tcp_basic_pointer_t)(*arg), length);
/*
    printf("<<< recv ARGV: %s\n", (*arg));
*/
  }
  else  (*arg)=NULL;

  /* return length ? */
}
