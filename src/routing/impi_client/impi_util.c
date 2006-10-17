
#include "impi_util.h"

static void swap_endian(void *buf, int size)
{
#if !IMPI_AM_BIGENDIAN
  int i;
  char tmp;
  char *a = buf;
  char *b = buf + size - 1;

  for (i = 0; i < size / 2; i++) {
    tmp = *a;
    *(a++) = *b;
    *(b--) = tmp;
  }
#endif
}

#define IMPI_Int2_ntoh IMPI_Int2_hton
short int IMPI_Int2_hton(short int value)
{
  swap_endian(&value, 2);

  return value;
}

#define IMPI_Int4_ntoh IMPI_Int4_hton
IMPI_Int4 IMPI_Int4_hton(IMPI_Int4 value)
{
  swap_endian(&value, 4);
  
  return value;
}

#define IMPI_Int8_ntoh IMPI_Int8_hton
IMPI_Int8 IMPI_Int8_hton(IMPI_Int8 value)
{
  swap_endian(&value, 8);

  return value;
}


char* IMPI_Get_argv2str(int* argc, char** argv[], char* string, char* end)
{
  int i, j;

  for(i=0; i<(*argc); i++)
  {
    if((*argv)[i])
    {
      if(strcmp((*argv)[i], end)==0) return NULL;
      if(strcmp((*argv)[i], string)==0)
      {
	/*
	 |   Found the key!
	 |   If the next value is not a switch, then check if it is not zero.
	 |   If the next value is zero, then check the one after the next (and so on...)
	*/
	for(j=i+1; j<(*argc); j++)
	{
	  if((*argv)[j]) return((*argv)[j]);
	}
      }
    }
  }

  return NULL;
}

char* IMPI_Pop_argv2str(int* argc, char** argv[], char* string, char* end)
{
  int i, j, k;
  char* ret;

  for(i=0; i<(*argc); i++)
  {
    if((*argv)[i])
    {
      if(strcmp((*argv)[i], end)==0) return NULL;
      if(strcmp((*argv)[i], string)==0)
      {
	/*
	 |   Found the key!
	 |   If the next value is not a switch, then check if it is not zero.
	 |   If the next value is zero, then check the one after the next (and so on...)
 	 */
	for(j=i+1; j<(*argc); j++)
	{
	  if((*argv)[j])
	  {
	    if(strncmp((*argv)[j], end, strlen(end))==0)
	    {
	      (*argv)[i] = 0;
	      return NULL;
	    }
	    ret=(*argv)[j];
	    (*argv)[j] = 0;

	    for(k=j+1; k<(*argc); k++)
	    {
	      if((*argv)[k])
	      {
		if(strncmp((*argv)[k], end, strlen(end))==0) (*argv)[i] = 0;
		break;
	      }
	    }
	    return ret;
	  }
	}
      }
    }
  }

  return NULL;
}

void  IMPI_Error(char* msg)
{
  fprintf(stderr, "### IMPI Error: %s\n", msg);
  fflush(stderr);
  exit(-1);
}
