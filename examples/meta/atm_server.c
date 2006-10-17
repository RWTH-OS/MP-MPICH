/* example:
/home/stef/mp-mpich/bin/mpicc -fPIC -DMPID_NO_FORTRAN -DMPID_NO_FORTRAN  -g -DMETA -DMETA_ATM -DMPI_SHARED_LIBS -D_REENTRANT -DDOLPHIN_SCI -DHAVE_MPICHCONF_H -I/home/stef/mp-mpich/src/routing -I /home/stef/download/linux-atm-2.4.0/src/saal/ -L /home/stef/download/linux-atm-2.4.0/src/saal/ -lsaal atm_server.c sequence.c  -o atm_server
*/

#include <stdio.h>
#include <unistd.h>

#include "connection.h"
#include <atm.h>
#include <linux/atmsvc.h>
#include <saal.h>
#include "atm_client_server.h"


int fd;

static void q_data_ind(void *user_data,void *data,int length) {
    read(fd, data, length);
    printf("read\n"); exit(-1);
}

static void q_cpcs_send(void *user_data,void *data,int length)
{
    write(fd, data, length);
    printf("write ...[%i, %i]\n", fd, length);
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

int atm_sscop_read(void *buffer, unsigned size, int fd, unsigned long maxSDUsize) {
    fd_set set, perm;
    int ret;
    SSCOP_DSC *dsc;
    SAAL_DSC saal_dsc;
/*     static unsigned char svc_buffer[sizeof(struct atmsvc_msg)+1]; */
/*     static struct atmsvc_msg* msg = (struct atmsvc_msg*) svc_buffer; */
    static struct atmsvc_msg msg;
    int read_size;

    gettimeofday(&now,NULL);

    start_saal(&saal_dsc, &ops, 0, sscop_q2110);
    saal_estab_req(&saal_dsc, NULL, 0);

    FD_ZERO(&perm);
    FD_SET(fd, &perm);
    gettimeofday(&now,NULL);

    while (1) {
	set = perm;
	/*
	 * Here we have a small race condition: if a signal is delivered after
	 * poll_signals tests for it but before select sleeps, we miss that
	 * signal. If it is sent again, we're of course likely to get it. This
	 * isn't worth fixing, because those signals are only used for
	 * debugging anyway.
	 */
	ret = select(fd+1,&set,NULL,NULL,next_timer());
	if (ret < 0) {
	    if (errno != EINTR) perror("select");
	}
	else {
	    gettimeofday(&now,NULL);
	    if (FD_ISSET(fd,&set)) {
		printf("read socket\n");
		read_size = read(fd, &msg, sizeof(msg));
		printf("size %i - msg->type: %i %i\n", read_size, ((unsigned char*)&msg)[1], as_itf_notify);
		
	    }

	    expire_timers();
	      /* expire timers after handling messges to make sure we don't
		 time out unnecessarily because of scheduling delays */
	}
    }


}


int atm_my_read(void *buffer, unsigned size, int fd, unsigned long maxSDUsize) {
    atm_packet packet_send, packet_recv;
    unsigned int sent, received;


    while (1) {
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

	printf("received: %u\n", received);

	/* send length of transmission */
	memset(&packet_send, 0, ATM_CELL_SIZE);
	packet_send.flag = 0x84;
	packet_send.payload[0] = (char) ((received & 0xFF000000) >> 24);
	packet_send.payload[1] = (char) ((received & 0x00FF0000) >> 16);
	packet_send.payload[2] = (char) ((received & 0x0000FF00) >> 8);
	packet_send.payload[3] = (char)  (received & 0x000000FF);

	if( ( send(fd, &packet_send, ATM_CELL_SIZE,0)) != ATM_CELL_SIZE ) {
	    perror("send");
	    exit(-1);
	}
	printf("send answer\n");
	sleep (1);

	/* connection established arrived? */
	if( ( recv(fd, &packet_recv, ATM_CELL_SIZE,0)) != ATM_CELL_SIZE ) {
	    perror("recv");
	    exit(-1);
	}
	if ( packet_recv.flag == 0xFF )
	    break;
    };
    printf("Ready...");

}



int main(){
/*     int fd; */
    char recbuf[PKTSIZE];
    int size,j,i;

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


    sequence_init();
#ifdef USE_MPICH_FUNCTIONS
    init_connections(1024);
#endif

    memset(&addr1,0,sizeofaddr1);
    addr1.sap_family = PF_ATMPVC;
    addr1.sap_addr.itf = 0;
    addr1.sap_addr.vpi = 0;
    addr1.sap_addr.vci = 42;
    memset(&addr2,0,sizeof(addr2));
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
    if ((fd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) == -1) {
	perror("socket");
	exit(-1);
    }
    text2qos(qos, &bqos, T2Q_DEFAULTS);
    if( setsockopt(fd, SOL_ATM, SO_ATMQOS, (struct atm_qos *)&bqos, sizeof(struct atm_qos)) == -1) {
	perror("setsockopt - qos");
	exit(-1);
    }
    if (bind(fd, (struct sockaddr*)&addr1, sizeofaddr1) == -1) {
	perror("bind");
	exit(-1);
    }
/*     if (listen(fd, 1)) { */
/* 	perror("listen"); */
/* 	exit(-1); */
/*     } */
#endif

    i = 0;
    while(1) {
	printf("testing for data...\n");

#ifdef USE_MPICH_FUNCTIONS
 	size = receive_message(recbuf, PKTSIZE, fd);
#else
/* 	if( accept(fd, (struct sockaddr*)&addr1, &sizeofaddr1) == -1) { */
/* 	    perror("accept"); */
/* 	    exit(-1); */
/* 	} */
/* 	size = atm_my_read(recbuf, PKTSIZE, fd, 99999); */
 	size = atm_sscop_read(recbuf, PKTSIZE, fd, 99999);
#endif
	if (size != i) {
	    fprintf(stderr, "expected %d Bytes, got %d Bytes\n", i, size); fflush(stderr);
	}
	    
	fprintf(stderr, "got %d Bytes, first Byte: %d\n", size, (int)((char*)recbuf)[1]); fflush(stderr);
	    
	if (*((char*)recbuf) == FERTIG) {
	    printf("Dienstschluss!\n");
	    break;
	}

	for ( j = 1; j < size; j++) 
	    if (((char*)recbuf)[j] != sequence()) {
		fprintf(stderr,"error in data: size= %d, pos = %d!\n",size,j);fflush(stderr);
		exit(-1);
		    
	    }
	i += 128;
    }
#ifdef USE_MPICH_FUNCTIONS
    close_connections();
#else
    close(fd);
#endif
    return 1;
}
