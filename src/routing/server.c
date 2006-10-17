#include <stdio.h>
#include <unistd.h>

#include "tcp_connection.h"
#include "client_server.h"

int main(){
    int conn1;
    char recbuf[PKTSIZE];
    int size,j,i;
	
    sequence_init();
    init_connections();
    conn1=add_connection("134.130.62.98" ,2500, "134.130.62.94",2500, CONN_SERVER);
    add_socket(conn1,"134.130.62.98" ,2501, "134.130.62.94",2501);
    add_socket(conn1,"134.130.62.98" ,2502, "134.130.62.94",2502);
    add_socket(conn1,"134.130.62.98" ,2503, "134.130.62.94",2503);
    if (tcp_establish_connection(conn1) < 0) {
	printf("kann Verbindung nicht aufbauen!\n");
	exit(-1);
    }

    i = 0;
    while(1) {
	printf("testing for data...\n");
	while (!tcp_select(conn1, 0)) {  
	    printf("."); 
	    fflush(stdout);
	    sleep(1); 
	}
	printf("\n");
	printf("message available!\n");

	size = receive_message(recbuf, PKTSIZE, conn1);
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
    tcp_close_connections(conn1);	
    return 1;
}
