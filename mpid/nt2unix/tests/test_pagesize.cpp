/* $id$ */

#include <windows.h>
#include <stdio.h>


int main (int argc, char **argv) {
  SYSTEM_INFO s; 
  
  GetSystemInfo(&s);
  
  printf("pagesize == %d\n", s.dwPageSize);

  return 0; 
}
