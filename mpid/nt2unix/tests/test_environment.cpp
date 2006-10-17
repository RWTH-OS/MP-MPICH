/* $id$ */

#include <windows.h>
#include <stdio.h>


int main (int argc, char **argv) {
  LPSTR environment;
  LPSTR position;
  LPSTR commandline;
  MIB_IPADDRTABLE ipatab; 
  DWORD iptab_size; 

  environment = GetEnvironmentStrings();
  position = environment;

  while(strlen(position) > 0) {
	printf("%s\n", position);	
	position += strlen(position) + 1;
  }

  commandline=GetCommandLine();

  printf("commandline: %s\n",commandline);

  GetIpAddrTable(&ipatab, &iptab_size, false); 

  return 0; 
}
