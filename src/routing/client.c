/* $Id$ 
 * (c) 2005 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de 
 */
#include <stdio.h>
#include "tcp_connection.h"
#include "client_server.h"


int main() {
	int conn1, i ,j;
	char sndbuf[PKTSIZE];

	sequence_init();
	init_connections();
	conn1 = add_connection("134.130.62.94",2500,"134.130.62.98",2500,CONN_CLIENT);
	add_socket(conn1, "134.130.62.94",2501,"134.130.62.98",2501 );
	add_socket(conn1, "134.130.62.94",2502,"134.130.62.98",2502 );
	add_socket(conn1, "134.130.62.94",2503,"134.130.62.98",2503 ); 
	if (tcp_establish_connection(conn1) < 0 ) {
		printf("kann Verbindung nicht aufbauen!\n");
		exit(-1);
	}

	for (i = 0; i < PKTSIZE; i += 128) {
		if (i + 1 == PKTSIZE) 
		    *((char*) sndbuf)=FERTIG;
		else 
		    *((char*) sndbuf)=WEITER;

		for ( j = 1; j < i; j++) 
			((char*)sndbuf)[j]=sequence();

		fprintf(stderr, "sending %d Bytes, first Byte: %d\n", i,
			(int) ((char*)sndbuf)[1]); fflush(stderr);
		
		tcp_send_message(sndbuf, i, conn1);
	}
		
	close_connections(conn1);	
	return 1;
};

