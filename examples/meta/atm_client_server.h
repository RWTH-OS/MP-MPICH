#define PKTSIZE 1000000
#define PKTS 10000
#define FERTIG 1
#define WEITER 0
#include "sequence.h"

#undef USE_MPICH_FUNCTIONS 1



#define ATM_CELL_SIZE 53
#define ATM_HEADER_SIZE 5

typedef struct {
    unsigned char flag;
    unsigned char seq_nbr;
    unsigned char ack_nbr;
    unsigned char checksum[2];
    unsigned char payload[ATM_CELL_SIZE - ATM_HEADER_SIZE];
} atm_packet;
