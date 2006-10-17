#include <stdio.h>
#include <iostream>
#include "nt2unix.h"
// #include "pingpong.h"


  SOCKET sock;

unsigned long Sender (void*)
{
  WSAOVERLAPPED lapped;
  lapped.hEvent = WSACreateEvent ();
  const int Count = 3;
  int nbyte = 1000000, i;
  WSABUF Buf[Count];
  DWORD Snt = 0;

  for (i = 0; i < Count; i++)
  {
    Buf[i].buf = new char[nbyte];
    Buf[i].len = nbyte;
    memset(Buf[i].buf, '2', nbyte);
  }

  printf ("Sender : calling WSASEND...\n");

  for (i = 0; i < 3; i++)
    {
      printf ("%d . Call to WSASend\n", i + 1);
      WSASend (sock, Buf, Count, &Snt, 0, &lapped, 0);
    }
  
  printf ("Sender : Calling WaitForSingleObject...\n");
  if (WaitForSingleObject (lapped.hEvent, INFINITE) == WAIT_OBJECT_0)
    printf ("Sender sent %d ", Snt," Bytes\n");

  for (i = 0; i < Count; i++)
    delete (Buf[i].buf);

  return 0;
}


int main(int argc, char **argv) {

  int status, nbyte = 1000000, i;
  int enable=1;
  DWORD BytesSent = 0, BytesReceived = 0;
  DWORD Sent = 0, Thread;
  HANDLE ThreadHandle;

  struct sockaddr_in address;
  struct hostent *hp;

  WORD wsaVersion = MAKEWORD(1,1);
  WSADATA wsaData;
  const int BufCount = 3;
  WSABUF Buffer[BufCount];
  const char s = 0;
  WSAOVERLAPPED Overlapped;
  Overlapped.hEvent = CreateEvent (0, 1, 0, 0);

// Check for correct command-line args

  if (argc < 2) {
        printf( "Usage: Client <host> \n");
        return 1;
    }

  if (WSAStartup(wsaVersion, &wsaData) != 0) {
	  return 255;
  }
  
  if ((hp = gethostbyname(argv[1])) == 0) {
	  printf("Could not find IP address for %s \n", argv[1]);
	  return 3;
  }

  printf("official host name = %s \n", hp->h_name);

  sock = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
  hp   = gethostbyname(argv[1]);
  cout << "SOCKET returned by WSASocket : " << sock << endl;

  address.sin_family = AF_INET;
  address.sin_port = htons(9001);
  
  memcpy((char *) &address.sin_addr, hp->h_addr, sizeof(address.sin_addr));
  

  if (connect(sock, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) == -1) {
    printf("client ERROR: can't connect socket");
    perror (&s);
    exit(-1);
  }

  if(setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,(char*)&enable, sizeof(int)) == -1)
	   printf("SERVER: setsockopt");

  for (i = 0; i < BufCount; i++)
  {
    Buffer[i].buf = new char[nbyte];
    Buffer[i].len = nbyte;
    memset(Buffer[i].buf, '0', nbyte);
  }

  ThreadHandle = CreateThread (0, 0, Sender, 0, 0, &Thread);

  for (i = 0; i < 3; i++)
    {
      printf ("%d . Call to WSASend\n", i + 1);
      WSASend (sock, Buffer, BufCount, &BytesSent, 0, &Overlapped, 0);
    }
 
  printf ("Calling recv...\n");

  for (int j = 0; j < 3; j++)
      for (int i = 0; i < BufCount; i++)
	{
	  BytesReceived = recv (sock, Buffer[i].buf, nbyte, MSG_WAITALL);
	  printf("Client recv: bytes = %d \n", BytesReceived);
	}

  if (WSACleanup())
    printf ("WSACleanup failed");
  
  if (!CloseHandle (Overlapped.hEvent))
    printf ("CloseHandle hEvent failed\n");;
    
  if (!CloseHandle (ThreadHandle))
    printf ("CloseHandle Thread failed\n");
    
  for (int i = 0; i < BufCount; i++)
    delete (Buffer[i].buf);

  status = closesocket(sock);

  return 0;
}

