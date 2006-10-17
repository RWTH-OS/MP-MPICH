#include <stdio.h>
#include <winsock2.h>

#include "pingpong.h"
extern "C" {
void table_top(int, int, int, int, double, char *);
void table_body(int, int, int, int, int, double, double*, FILE *);
}
void InitTimer(void);

LARGE_INTEGER StartTime;
LARGE_INTEGER GetStartTime(void);
double ElapsedMicroseconds();
double timer();

int main(int argc, char **argv)
{

  struct sockaddr_in serverAddress;
  struct hostent *hp;	
  SOCKET MasterSocket, UserSocket;

  double     tf[SAMPLES + 1], t_call;
  int        log2nbyte, nbyte, ns, n_pp, npes;
  int        n_init, n_mess;
  int        enable = 1;
  char       *Title ="Single Messages --- MPI";
  FILE       *ifp;
  DWORD BytesSent, BytesReceived;
  char* inBuf;

  WORD wsaVersion = MAKEWORD(1,1);
  WSADATA wsaData;
  WSABUF Buffer;
  
  npes = 2;
  
  // Check for correct command-line args
    if (argc < 2) {
        printf( "Usage: Server <host> \n");
        return 1;
    }

  if (WSAStartup(wsaVersion, &wsaData) != 0) {
	  return 255;
  }

  // SetThreadAffinityMask(GetCurrentThread(),1);

  if (( hp = gethostbyname(argv[1])) == 0) {
	  printf("Could not find IP address for %s \n", argv[1]);
	  return 3;
  }

  printf("official host name = %s \n", hp->h_name);

  StartTime = GetStartTime(); /* Get StartTime */

  tf[0] = timer();
  for (ns = 1; ns <= SAMPLES; ++ns) {
     tf[ns] = timer();
  }
  t_call = (tf[SAMPLES] - tf[0]) / SAMPLES;

  n_init = 2 * PINGPONGS;
  n_mess = 2 * PINGPONGS;

  ifp = fopen("socket-genias.dat","w");
  table_top(2, n_init, n_mess, SAMPLES, t_call, Title);
  
  MasterSocket = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, 0);
  if (MasterSocket == -1) {
    printf("Server: socket ERROR \n");
    exit(-1);
  }

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = INADDR_ANY;
  serverAddress.sin_port = htons(9001);
  
  if (bind(MasterSocket, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in))) {
    printf("Server: ERROR can't bind MasterSocket");
    exit(-1);
  }
  
  listen(MasterSocket, 1);
  UserSocket = accept(MasterSocket, NULL, NULL);
  
  if(setsockopt(UserSocket, IPPROTO_TCP, TCP_NODELAY,(char*)&enable, sizeof(int)) == -1)
	   printf("SERVER: setsockopt");

  for (log2nbyte = 0; log2nbyte <= LOG2N_MAX; ++log2nbyte) {

	 nbyte = (1<< log2nbyte);
     Buffer.buf = new char[nbyte];
	 Buffer.len = nbyte;
     memset(Buffer.buf, '1', nbyte);
	 inBuf = new char[nbyte];

     tf[0] = timer();

	 for (ns = 0; ns < SAMPLES; ns++) {
        for (n_pp = 0; n_pp < PINGPONGS; n_pp++) {
		   BytesReceived = recv (UserSocket, inBuf, nbyte, 0);
		   printf("SERVER recv: bytes = %d \n", BytesReceived);
		  // if (BytesReceived < nbyte)
			//   printf("ErrorCode : %d \n", WSAGetLastError());

			//bytes = RecvSocket(sock, &Buffer, 1, &BytesReceived, 0, 0, 0);
		   if (!WSASend (UserSocket, &Buffer, 1, &BytesSent, 0, 0, 0))
			   printf("SERVER send: bytes = %d \n", BytesSent);
		   //else
			//   printf("ErrorCode : %d \n", WSAGetLastError());
		   //bytes = SendSocket(sock, &Buffer, 1, &BytesSent, 0, 0, 0);
        }

        tf[ns+1] = timer();
	 }

     table_body(2, nbyte, n_init, n_mess, SAMPLES, t_call, tf, ifp);
  }


  closesocket(UserSocket);
  closesocket(MasterSocket);
  return 0;
}


