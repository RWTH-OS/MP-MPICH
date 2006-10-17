#ifndef _USI_H
#define _USI_H

#ifndef _WIN32

typedef unsigned char USI_Byte;       /* <-- 8-bit unsigned / Byte */

typedef signed int USI_Int4;          /* <-- 32-bit signed integer */

typedef unsigned int USI_Uint4;       /* <-- 32-bit unsigned integer */

typedef signed long int USI_Int8;     /* <-- 64-bit signed integer */

typedef unsigned long int USI_Uint8;  /* <-- 64-bit unsigned integer */

typedef void* USI_Pointer;            /* <-- typeless memory pointer */

#endif

#define USI_BLOCK 1
#define USI_DONT_BLOCK 0

#define USI_SUCCESS 0
#define USI_ERROR  -1
#define USI_RETRY   1
#define USI_PENDING 2

#define USI_NAME_LENGTH 256
#define USI_MAX_PROCS 256
#define USI_MAX_TIMEOUT -1

typedef USI_Int4  USI_rank_t;
typedef USI_Uint4 USI_size_t;

typedef struct _USI_Private
{
  USI_Pointer data;
  USI_size_t size;
} USI_Private;

struct _USI_Protocol;
struct _USI_Request;

typedef struct _USI_Handle
{
  USI_rank_t size;
  USI_rank_t rank;
  struct _USI_Protocol* first_protocol;
  struct _USI_Protocol** protocol_table;
} USI_Handle;

typedef struct _USI_Protocol
{
  int    protocol_argc;
  char** protocol_argv;
  USI_rank_t local_np;
  USI_rank_t local_rank;
  /* XXX DO WE NEED THE HANDLE HERE??? -> OR Do WE NEED ONLY THE PROTOCOL-POINTER ??? */
  int (*USI_Protocol_Establish)(int* argc, char** argv[], USI_rank_t* rank, USI_rank_t* size, USI_Handle* handle, struct _USI_Protocol* this);
  int (*USI_Protocol_Send)(USI_rank_t dest, void* buffer, USI_size_t size, USI_Byte blocking, struct _USI_Request* request, USI_Handle handle, struct _USI_Protocol* this);
  int (*USI_Protocol_Recv)(USI_rank_t src,  void* buffer, USI_size_t size, USI_Byte blocking, struct _USI_Request* request, USI_Handle handle, struct _USI_Protocol* this);
  int (*USI_Protocol_Test)(USI_Byte blocking, struct _USI_Request* request, USI_Handle handle, struct _USI_Protocol* this);
  int (*USI_Protocol_Probe)(USI_rank_t src, USI_Byte blocking, USI_Handle handle, struct _USI_Protocol* this);
  
  USI_Private private;
  struct _USI_Protocol* next_protocol;
} USI_Protocol;

typedef struct _USI_Request
{
  void* buffer;
  USI_size_t final_size;
  volatile USI_size_t actual_size;
  USI_rank_t rank;
  USI_Handle* handle;
  USI_Byte active_send;
  USI_Byte active_recv;
} USI_Request;

int USI_Establish(int* argc, char** argv[], USI_rank_t* rank, USI_rank_t* size, USI_Handle* handle);
int USI_Finalize(void);

int USI_Send(USI_rank_t dest, void* buffer, USI_size_t size, USI_Byte blocking, USI_Request* request, USI_Handle handle);
int USI_Recv(USI_rank_t src,  void* buffer, USI_size_t size, USI_Byte blocking, USI_Request* request, USI_Handle handle);

int USI_Test(USI_Byte blocking, USI_Request* request, USI_Handle handle);
int USI_Probe(USI_rank_t src, USI_Byte blocking, USI_Handle handle);

#endif
