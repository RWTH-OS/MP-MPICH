#include "nt2unix.h"
#include <map>

void main()
{
  SocketStruct* MySockets[5];


  typedef map <int, SocketStruct*> SocketMap_t;

  SocketMap_t SocketMap;

  for (int i = 0; i <= 4; i++)
    MySockets[i] = malloc (sizeof(SocketStruct));

  for (i = 0; i <= 4; i++)
    MySockets[i]->Message->buf = i+100;

  for (i = 0; i <= 4; i++)
    SocketMap.insert (SocketMap::Valuetype(i, MySockets[i]));
}


