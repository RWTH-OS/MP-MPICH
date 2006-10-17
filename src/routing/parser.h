/* $Id$ 
 * (c) 2005 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de 
 */

#ifndef _PARSER_H
#define _PARSER_H

#include "rhlist.h"
#include "auto_router.h"

#define MAX_STRING 1000
#define MAX_NICDEF_NAME_LEN 100
#define HOSTNAMELEN MPI_MAXHOSTNAMELEN

    /* type declarations */

    typedef enum _ConnDefType {ConnDefType_UNI,ConnDefType_BI} ConnDefType;
    
    /* list of net names */
    struct _SnetName {
      char* name;
      struct _SnetName * next;
    };
    typedef struct _SnetName SnetName;

    /* a list of address-lists */
    struct _SnicList {
	Snic * nicList;
	struct _SnicList *next;
    } ;
    typedef struct _SnicList SnicList;

    /* struct for NICDEF entries */
    struct _SnicDef {
      Snic *nic;
      char nicName[MAX_NICDEF_NAME_LEN];
      struct _SnicDef * next;
    };
    typedef struct _SnicDef SnicDef;

typedef union
{
  int zahl;
  char string[MAX_STRING]; /* FIXME buffer overrun */
  char string_array[2][MAX_STRING];
  char *pointer_to_char;
  char ** pointer_to_pointer_to_char;
  
  int * pointer_to_int;
  Snic *pointer_to_snic;
  SnicDef *pointer_to_snicdef;
  SnicList *pointer_to_sniclist;
  Snode *pointer_to_snode;
  SnetName * pointer_to_snetname;
  EautoAutoOption autoOption;
  ConnDefType conn_def_type; 
} YYSTYPE;

#define YYSTYPE_IS_DECLARED

#endif
