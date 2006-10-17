
#ifndef _IMPI_DEFS_H_
#define _IMPI_DEFS_H_

#define IMPI_MAX_TIMEOUT 100

#define _IMPI_DEBUG_

typedef signed   char          IMPI_Int1;
typedef unsigned char          IMPI_Uint1;
typedef signed   int           IMPI_Int4;
typedef unsigned int           IMPI_Uint4;
typedef signed   long long int IMPI_Int8;
typedef unsigned long long int IMPI_Uint8; 

#define IMPI_CMD_AUTH 0x41555448 /* ASCII for 'AUTH' */
#define IMPI_CMD_IMPI 0x494D5049 /* ASCII for 'IMPI' */
#define IMPI_CMD_COLL 0x434F4C4C /* ASCII for 'COLL' */
#define IMPI_CMD_DONE 0x444F4E45 /* ASCII for 'DONE' */
#define IMPI_CMD_FINI 0x46494E49 /* ASCII for 'FINI' */

#define IMPI_NO_LABEL 0x0000

#define IMPI_C_VERSION        0x1000
#define IMPI_C_NHOSTS         0x1100
#define IMPI_C_NPROCS         0x1200
#define IMPI_C_DATALEN        0x1300
#define IMPI_C_TAGUB          0x1400
#define IMPI_C_COLL_XSIZE     0x1500
#define IMPI_C_COLL_MAXLINEAR 0x1600

#define IMPI_H_IPV6    0x2000
#define IMPI_H_PORT    0x2100
#define IMPI_H_NPROCS  0x2200
#define IMPI_H_ACKMARK 0x2300
#define IMPI_H_HIWATER 0x2400

#define IMPI_P_IPV6 0x3000
#define IMPI_P_PID  0x3100
  
typedef struct {
  IMPI_Int4 cmd;
  IMPI_Int4 len;
} IMPI_Cmd;

typedef struct {
  IMPI_Uint4 auth_mask;
} IMPI_Client_auth;

typedef struct {
  IMPI_Int4 which;
  IMPI_Int4 len;
} IMPI_Server_auth;

typedef union {
  IMPI_Int4 rank;
  IMPI_Int4 size;
} IMPI_Impi;

typedef struct {
  IMPI_Int4 label;
} IMPI_Coll;

typedef struct {
  IMPI_Int4 label;
  IMPI_Uint4 client_mask;
} IMPI_Chdr;

typedef struct {
  IMPI_Uint4 major;
  IMPI_Uint4 minor;
} IMPI_Version;

typedef struct {
  IMPI_Uint1 p_hostid[16]; /* host identifier */
  IMPI_Int8 p_pid;            /* local process identifier */
} IMPI_Proc;

typedef struct {
  IMPI_Uint4 pk_type;       /* packet type */
#define IMPI_PK_DATA      0 /* message data */
#define IMPI_PK_DATASYNC  1 /* message data (sync) */
#define IMPI_PK_PROTOACK  2 /* protocol ACK */
#define IMPI_PK_SYNCACK   3 /* synchronization ACK */
#define IMPI_PK_CANCEL    4 /* cancel request */
#define IMPI_PK_CANCELYES 5 /* 'yes' cancel reply */
#define IMPI_PK_CANCELNO  6 /* 'no' cancel reply */
#define IMPI_PK_FINI      7 /* agent end-of-communications */
  IMPI_Uint4 pk_len;        /* packet data length */
  IMPI_Proc pk_src;         /* source process */
  IMPI_Proc pk_dest;        /* destination process */
  IMPI_Uint8 pk_srqid;      /* source request ID */
  IMPI_Uint8 pk_drqid;      /* destination request ID */
  IMPI_Uint8 pk_msglen;     /* total message length */

  IMPI_Int4 pk_lsrank;      /* comm-local source rank */
  IMPI_Int4 pk_tag;         /* message tag */
  IMPI_Uint8 pk_cid;        /* context ID */
  IMPI_Uint8 pk_seqnum;     /* message sequence # */
  IMPI_Int8 pk_count;       /* QoS: message count */
  IMPI_Uint8 pk_dtype;      /* QoS: message datatype */
  IMPI_Uint8 pk_reserved;   /* for future use */
} IMPI_Packet;

#endif

