/* $Id$ 
 * (c) 2005 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de 
 */
#include <sys/time.h>
#include "sequence.h"

#define PKTSIZE 1024*1024
#define PKT_INC 32
#define PKT_START 1
#define NBR_PKTS 1
#define SPLITSIZE 610
#define TCP_BASEPORT 2500

#define FERTIG 1
#define WEITER 0

#define VERIFY 1
#define VERBOSE 1

/* network adresses */
#define IP_SERVER1 "134.130.62.58"
#define IP_SERVER2 "134.130.62.94"
#define IP_SERVER3 "134.130.62.94"
#define IP_SERVER4 "192.168.4.1"
#define IP_SERVER5 "134.130.62.14"

#define IP_CLIENT1 "134.130.62.52"
#define IP_CLIENT2 "134.130.62."
#define IP_CLIENT3 "134.130.62."
#define IP_CLIENT4 "192.168.4.2"
#define IP_CLIENT5 "134.130.62.46"




