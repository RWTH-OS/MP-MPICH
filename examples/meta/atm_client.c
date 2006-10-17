/* example:
/home/stef/mp-mpich/bin/mpicc -fPIC -DMPID_NO_FORTRAN -DMPID_NO_FORTRAN  -g -DMETA -DMETA_ATM -DMPI_SHARED_LIBS -D_REENTRANT -DDOLPHIN_SCI -DHAVE_MPICHCONF_H -I/home/stef/mp-mpich/src/routing -I /home/stef/download/linux-atm-2.4.0/src/saal/ -L /home/stef/download/linux-atm-2.4.0/src/saal/ -lsaal atm_client.c sequence.c  -o atm_client
*/


#include <stdio.h>
#include "connection.h"
#include <atm.h>
//#include "nsscop/sscop.h"
#include <saal.h>
#include "atm_client_server.h"


int fd;

static void q_data_ind(void *user_data,void *data,int length) {
    read(fd, data, length);
    printf("read\n"); exit(-1);
}

static void q_cpcs_send(void *user_data,void *data,int length)
{
    int send_size;
    send_size = write(fd, data, length);
    printf("write ...[%i, %i, %i]\n", fd, length, send_size);
}


static SAAL_USER_OPS ops = {
    NULL, /* no q_estab_ind - 5.5.6.9 says 5.5.6.11 and 5.5.6.11 says "may" */
    NULL,
    NULL,
    NULL, /* no q_rel_conf - what to do ? */
    NULL,
    q_data_ind,
    NULL, /* no q_unitdata */
    q_cpcs_send
};



void atm_sscop_init() {
/*     start_sscop(); */
}

int atm_sscop_write(void *buffer, unsigned size, int fd, unsigned long maxSDUsize) {
    SSCOP_DSC *dsc;
    SAAL_DSC saal_dsc;

    gettimeofday(&now,NULL);

    start_saal(&saal_dsc, &ops, 0, sscop_q2110);
    saal_estab_req(&saal_dsc, NULL, 0);

    close(fd);
    exit(-1);
}

int atm_my_write(void *buffer, unsigned size, int fd, unsigned long maxSDUsize) {
    atm_packet packet_send, packet_recv;
    unsigned int sent, received;

    memset(&packet_send, 0, ATM_CELL_SIZE);
    packet_send.flag = 0x84;
    packet_send.payload[0] = (char) ((size & 0xFF000000) >> 24);
    packet_send.payload[1] = (char) ((size & 0x00FF0000) >> 16);
    packet_send.payload[2] = (char) ((size & 0x0000FF00) >> 8);
    packet_send.payload[3] = (char)  (size & 0x000000FF);

    do {
	printf("size: %i\n", size);
	/* send length of transmission */
	if( ( send(fd, &packet_send, ATM_CELL_SIZE,0)) != ATM_CELL_SIZE ) {
	    perror("send");
	    exit(-1);
	}


	/* wait for response */
	printf("Waiting...\n");
	if( ( recv(fd, &packet_recv, ATM_CELL_SIZE,0)) != ATM_CELL_SIZE ) {
	    perror("recv");
	    exit(-1);
	}
	received = ( (unsigned int)packet_recv.payload[0] << 24 )
	    + ( (unsigned int)packet_recv.payload[1] << 16 )
	    + ( (unsigned int)packet_recv.payload[2] << 8 )
	    + ( (unsigned int)packet_recv.payload[3] );
	
	printf("received back: %u\n", received);
	sleep (1);

    } while ( received != size || packet_recv.flag != 0x84 );
    /* send connection established */
    memset(&packet_send, 0, ATM_CELL_SIZE);
    packet_send.flag = 0xFF;
    if( ( send(fd, &packet_send, ATM_CELL_SIZE,0)) != ATM_CELL_SIZE ) {
	perror("send");
	exit(-1);
    }

    printf("Ready...");
}



int main() {
/*     int fd; */
    int i ,j;
    char sndbuf[PKTSIZE];

    ATM_PVC_PARAMS addr1;
    socklen_t sizeofaddr1 = sizeof(addr1);
    ATM_PVC_PARAMS addr2;
    socklen_t sizeofaddr2 = sizeof(addr2);
    char *qos = "cbr,aal5:max_pcr=0,min_pcr=10000,sdu=0";
#ifdef USE_MPICH_FUNCTIONS
;
#else
    struct atm_qos bqos;
#endif

printf("%i -\n", sizeof(atm_packet));
    sequence_init();
#ifdef USE_MPICH_FUNCTIONS
    init_connections(1024);
#endif

    memset(&addr1,0,sizeof(addr1));
    addr1.sap_family = PF_ATMPVC;
    addr1.sap_addr.itf = 0;
    addr1.sap_addr.vpi = 0;
    addr1.sap_addr.vci = 42;
    memset(&addr2,0,sizeofaddr2);
    addr2.sap_family = PF_ATMPVC;
    addr2.sap_addr.itf = 0;
    addr2.sap_addr.vpi = 0;
    addr2.sap_addr.vci = 42;

#ifdef USE_MPICH_FUNCTIONS
    fd = add_connection((struct sockaddr *)&addr1, (struct sockaddr *)&addr2, PF_ATMPVC, qos, CONN_CLIENT);
    if (establish_connection(fd) < 0 ) {
 	printf("kann Verbindung nicht aufbauen!\n");
 	exit(-1);
    }
#else
    if ((fd = socket(PF_ATMPVC,SOCK_DGRAM,ATM_AAL5)) == -1) {
	perror("socket");
	exit(-1);
    }
    text2qos(qos, &bqos, T2Q_DEFAULTS);
    if( setsockopt(fd, SOL_ATM, SO_ATMQOS, (struct atm_qos *)&bqos, sizeof(struct atm_qos)) == -1) {
	perror("setsockopt - qos");
	exit(-1);
    }
    if( connect(fd, (struct sockaddr *)&addr2, sizeofaddr2) < 0 ) {
	perror("connect");
	exit(-1);
    }
#endif


    for (i = 0; i < PKTSIZE; i += 128) {
	if (i + 1 == PKTSIZE) 
	    *((char*) sndbuf)=FERTIG;
	else 
	    *((char*) sndbuf)=WEITER;

	for ( j = 1; j < i; j++) 
	    ((char*)sndbuf)[j]=sequence();

	fprintf(stderr, "sending %d Bytes, first Byte: %d\n", i,
		(int) ((char*)sndbuf)[1]); fflush(stderr);

#ifdef USE_MPICH_FUNCTIONS		
 	send_message(sndbuf, i, fd);
#else
	atm_sscop_write(sndbuf, i, fd, 999999);
#endif
	sleep(1);
    }

#ifdef USE_MPICH_FUNCTIONS		
    close_connections();
#else
    close(fd);
#endif
    return 1;
}
