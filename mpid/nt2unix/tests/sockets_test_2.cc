#include "nt2unix.h"
#include <map>
#include <iostream>
#include <deque>
#include <memory>

void main()
{
  WSABUF Buffers[5];
  
  WSABUF It;

  MessageQueue.clear();

  for (int i = 0; i <= 4; i++)
    Buffers[i].buf = (char*) malloc (sizeof(char));

  for (i = 0; i <= 4; i++)
    Buffers[i].buf = (char)i+100;
  
  for (i = 0; i <= 4; i++)
    MessageQueue.push_back (Buffers[i]);

  cout << "Get Values :" << endl;

  for (i = 0; i <= 4; i++)
    {
      It = MessageQueue.front();
      cout << (int) It.buf << endl;
      MessageQueue.pop_front();
    }

  for (i = 0; i <= 4; i++)
    free (Buffers[i].buf);
}
