
# For Solaris:
#LDFLAGS=-lsocket -lresolv -lnsl -lrt

SRC = impi_client.c impi_host.c impi_router.c impi_tcp.c impi_util.c
INCLUDE =  -I../../../include -I../../../mpid/ch2 -I../../../mpid/util -I../../../mpid/ch_gateway  -I../.. -I.. -I.
DEFINES = -DMPIR_LIB_CODE -DMPID_NO_FORTRAN  -DMETA -DMETA_DEBUG -DROUTER_THREADS -DCH_USOCK_PRESENT -DHAVE_MPICHCONF_H -DYYDEBUG=1 -DMPI_LINUX

default: lib

all: client

client: 
	gcc -o impi_client $(SRC) $(LDFLAGS) -g -Wall

debug:
	gcc -o impi_client $(SRC) $(LDFLAGS) -g -Wall -D_DEBUG $(INCLUDE) $(DEFINES)
lib:
	gcc  -g -Wall -D_DEBUG -D_IMPI_ROUTER $(DEFINES) $(INCLUDE) -DMETA -DMPI_LINUX -c $(SRC) $(LDFLAGS)
	ar crl /work/carsten/src/meta-impi/lib/libmpich.a $(SRC:.c=.o)

clean:
	rm -rf impi_client 
	rm -rf *.o core cpi
	rm -rf *~
