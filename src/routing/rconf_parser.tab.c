/* A Bison parser, made by GNU Bison 1.875d.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ZAHL = 258,
     NAME = 259,
     PATH = 260,
     LITERAL = 261,
     IP = 262,
     ATM = 263,
     ATM_PVC_ADDRESS = 264,
     IP_NETMASK = 265,
     AUTO_ROUTER = 266,
     NO_DNS = 267,
     ONLY_DNS = 268,
     NUMHOSTS = 269,
     PORTBASE = 270,
     PORTRANGE = 271,
     ROUTERTIMEOUT = 272,
     HETERO = 273,
     EXCHANGE_ORDER = 274,
     SPLITSIZE = 275,
     ISEND_NUM = 276,
     TCP = 277,
     ATM_SVC = 278,
     ATM_PVC = 279,
     DOPPELPUNKT = 280,
     MINUS = 281,
     KOMMA = 282,
     RETURN = 283,
     CONNECTIONS = 284,
     PAIR = 285,
     OPTIONS = 286,
     NICDEF = 287,
     METAHOST = 288,
     GLEICH = 289,
     CONNTO = 290,
     CONNTO_BIDIR = 291,
     GETIP = 292,
     NODES = 293,
     ROUTERS = 294,
     NET = 295,
     SECONDARY_DEVICE = 296,
     EXTRAPROCS = 297,
     MAX = 298,
     NETMASK = 299,
     NOWATCHDOG = 300,
     SMI_VERBOSE = 301,
     SMI_DEBUG = 302
   };
#endif
#define ZAHL 258
#define NAME 259
#define PATH 260
#define LITERAL 261
#define IP 262
#define ATM 263
#define ATM_PVC_ADDRESS 264
#define IP_NETMASK 265
#define AUTO_ROUTER 266
#define NO_DNS 267
#define ONLY_DNS 268
#define NUMHOSTS 269
#define PORTBASE 270
#define PORTRANGE 271
#define ROUTERTIMEOUT 272
#define HETERO 273
#define EXCHANGE_ORDER 274
#define SPLITSIZE 275
#define ISEND_NUM 276
#define TCP 277
#define ATM_SVC 278
#define ATM_PVC 279
#define DOPPELPUNKT 280
#define MINUS 281
#define KOMMA 282
#define RETURN 283
#define CONNECTIONS 284
#define PAIR 285
#define OPTIONS 286
#define NICDEF 287
#define METAHOST 288
#define GLEICH 289
#define CONNTO 290
#define CONNTO_BIDIR 291
#define GETIP 292
#define NODES 293
#define ROUTERS 294
#define NET 295
#define SECONDARY_DEVICE 296
#define EXTRAPROCS 297
#define MAX 298
#define NETMASK 299
#define NOWATCHDOG 300
#define SMI_VERBOSE 301
#define SMI_DEBUG 302




/* Copy the first part of user declarations.  */
#line 1 "rconf_parser.y"

    /* $Id: rconf_parser.y 4809 2006-10-05 09:53:09Z gsell $
     * (c) 2005 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
     * author: Martin Poeppe email: mpoeppe@gmx.de
     *
     * this is the bison file for the meta-configuration parser.
     * it retrieves the necessary information for the router AND the device
     * setup of all processes.
     *
     * These informations are stored in the structs pars_rconf and 
     * information concerning the network connections are stored in the host- and routerlist
     */
#include <stdlib.h> 
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#ifdef WIN32
#include <malloc.h>
#endif
#include "metaconfig.h"
#include "mpi_router.h"
#include "parser.h"
#include "rdebug.h"
#include "rhlist.h"
#include "auto_router.h"


#ifndef WIN32
#include <netdb.h>
#include <sys/socket.h>
#endif



void yyerror(const char* s);

/*#define PARSER_DEBUG */
#ifdef PARSER_DEBUG
#define PDEBUG(a)      RDEBUG(a)
#define PDEBUG1( a,b ) RDEBUG1(a,b)
#define PDEBUG2(a,b,c) RDEBUG2(a,b,c)
#define PDEBUG3(a,b,c,d)      RDEBUG3(a,b,c,d)
#else
#define PDEBUG( a) 
#define PDEBUG1(a,b)
#define PDEBUG2(a,b,c)
#define PDEBUG3(a,b,c,d)
#endif


#define CONF_ERROR(a) {yyerror(a);YYABORT;}

#include "newstrings.h"

       
    /* forward declarations of functions that are implemented in the third section of this file */
    Snic *rh_newNic( char *adr, int port, EnicType type );
    SnicDef *lookupNicDef( char *name );
    
    void resetGlobals();
    int saveRouter( SnicList *nicStack, ConnDefType e_connDefType, char* routerExec);
    int checkConnSections();
    void checkHeaderSection();
    int checkSubSection();
    Snode * sortInNodeList ( Snode *nodeListHead, Snode *inSortNode );


    Snode * createNodeListRange(char* a,char *b,int *start,int *end, int numprocs, SnicList * nn);
    int excludeNode; /* marks if a node was excluded with ! */

    /* variable declarations */

    /* this pointer is used by MPIR_read_metaconfig() as an interface to the parser,
       so it must not be declared static */
    struct RouterConfig *pars_rconf;

    static int countSubsections = 0;
    static int countHosts = 0;

    /* this flag is zero when 0 routers are declared in a PAIR head, so we don't have to parse connections 
       this is for multidevice configurations */
    static int Bparse_conn=0;

    static char hostA[HOSTNAMELEN], hostB[HOSTNAMELEN]; 
    static int nrRouters;   

    static int *sectionList;
    static int numSections;

    static SnicDef *nicDefList = 0;   /* list of NICDEFs */

    /* this is for metahost declarations */
    static int metaHostDefH_id;
    static Snode *metaHostDefNodelist;
    static char metaHostDefType[MAX_STRING];
    static char metaHostDefFRONTEND[MAX_STRING];
    static char metaHostDefUSER[MAX_STRING];
    static char metaHostDefEXECPATH[MAX_STRING];
    static char metaHostDefEXECNAME[MAX_STRING];
    static char metaHostDefEXECARGS[MAX_STRING];
    static char metaHostDefCONFPATH[MAX_STRING];
    static char metaHostDefCONFNAME[MAX_STRING];
    static char metaHostDefMETAKEY[MAX_STRING];
    static char metaHostDefENVFILE[MAX_STRING];
    static char metaHostDefMPIROOT[MAX_STRING];
    static char metaHostDefDevOption[MAX_STRING];
    static char * metaHostDefROUTEREXEC;
    static char **metaHostExtraProcList;
    static char *routerExec;
    static int metaHostFound=0; /* boolean, marks if the given metahost-name is found in the config */
    static int routerCounter;   
    static ConnDefType lastConnDefType;

    /* declared in lexer */
    extern int ZCounter;
    extern int SCounter;
    extern int yylex( void );


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 300 "rconf_parser.tab.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   define YYSTACK_ALLOC alloca
#  endif
# else
#  if defined (alloca) || defined (_ALLOCA_H)
#   define YYSTACK_ALLOC alloca
#  else
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE))			\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  6
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   227

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  54
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  51
/* YYNRULES -- Number of rules. */
#define YYNRULES  111
/* YYNRULES -- Number of states. */
#define YYNSTATES  215

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   302

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    53,     2,     2,     2,     2,     2,     2,
      48,    49,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    52,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    50,     2,    51,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     7,    10,    14,    15,    17,    19,    22,
      24,    26,    31,    35,    37,    40,    42,    46,    50,    54,
      58,    62,    66,    69,    72,    75,    78,    85,    89,    90,
      94,    96,   100,   104,   110,   114,   116,   118,   120,   124,
     126,   129,   132,   135,   140,   147,   150,   151,   153,   157,
     162,   166,   170,   174,   178,   182,   186,   188,   192,   195,
     197,   199,   203,   204,   206,   207,   209,   219,   220,   223,
     228,   233,   241,   247,   248,   252,   253,   258,   260,   264,
     265,   267,   269,   271,   275,   277,   281,   283,   285,   289,
     293,   298,   304,   306,   308,   310,   313,   316,   318,   325,
     327,   329,   331,   334,   337,   341,   342,   344,   346,   348,
     351,   353
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      55,     0,    -1,    56,    62,    93,    -1,    57,    59,    -1,
      14,     3,    28,    -1,    -1,     6,    -1,    61,    -1,    59,
      61,    -1,     3,    -1,    43,    -1,     4,    60,    58,    28,
      -1,    31,    28,    63,    -1,    64,    -1,    63,    64,    -1,
      28,    -1,    17,     3,    28,    -1,    15,     3,    28,    -1,
      18,     3,    28,    -1,    19,     3,    28,    -1,    20,     3,
      28,    -1,    21,     3,    28,    -1,    70,    28,    -1,    73,
      28,    -1,    65,    28,    -1,    66,    28,    -1,    40,     4,
       4,     4,     4,     3,    -1,    41,     4,    67,    -1,    -1,
      48,    68,    49,    -1,    69,    -1,    69,    27,    68,    -1,
      15,    34,     3,    -1,    16,    34,     3,    26,     3,    -1,
      44,    34,    10,    -1,    45,    -1,    46,    -1,    47,    -1,
      32,    71,    72,    -1,     4,    -1,    22,     7,    -1,    23,
       8,    -1,    24,     9,    -1,    37,    48,     4,    49,    -1,
      74,    75,    50,    75,    76,    51,    -1,    33,     4,    -1,
      -1,    28,    -1,    77,    52,    75,    -1,    77,    52,    75,
      76,    -1,     4,    34,     4,    -1,     4,    34,     5,    -1,
      38,    34,    78,    -1,    39,    34,    78,    -1,    42,    34,
      80,    -1,     4,    34,     6,    -1,    79,    -1,    78,    27,
      79,    -1,    53,    85,    -1,    85,    -1,    83,    -1,    80,
      27,    83,    -1,    -1,     4,    -1,    -1,     6,    -1,     4,
      25,    89,    25,     5,    25,    81,    25,    82,    -1,    -1,
      25,     5,    -1,    25,     5,    25,    82,    -1,     4,    89,
      87,    84,    -1,     4,    89,    48,    90,    49,    87,    84,
      -1,     4,    26,     4,    89,    86,    -1,    -1,    48,    90,
      49,    -1,    -1,    40,    48,    88,    49,    -1,     4,    -1,
      88,    27,     4,    -1,    -1,     3,    -1,    91,    -1,    92,
      -1,    91,    27,    92,    -1,     7,    -1,     7,    25,     3,
      -1,     8,    -1,     4,    -1,     4,    25,     3,    -1,    29,
      28,    95,    -1,    29,    28,    11,   104,    -1,    29,    28,
      11,    94,   104,    -1,    12,    -1,    13,    -1,    96,    -1,
      95,    96,    -1,    97,    99,    -1,    97,    -1,    30,     4,
       4,     3,    98,    28,    -1,    26,    -1,     5,    -1,   100,
      -1,    99,   100,    -1,   101,    28,    -1,    90,   102,   103,
      -1,    -1,    35,    -1,    36,    -1,    90,    -1,   103,    90,
      -1,    28,    -1,   104,    28,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   158,   158,   161,   176,   189,   190,   192,   193,   195,
     196,   198,   235,   257,   258,   261,   262,   266,   270,   274,
     278,   282,   286,   295,   296,   297,   300,   319,   333,   334,
     338,   339,   342,   345,   349,   370,   372,   374,   378,   392,
     404,   421,   434,   447,   471,   557,   590,   591,   594,   595,
     598,   614,   626,   632,   638,   641,   653,   654,   669,   674,
     679,   681,   684,   685,   687,   688,   690,   720,   724,   729,
     734,   766,   803,   814,   815,   823,   824,   831,   837,   845,
     846,   849,   860,   863,   873,   876,   879,   882,   894,   908,
     909,   914,   921,   922,   924,   925,   928,   933,   938,   983,
     984,   986,   987,   990,  1002,  1030,  1031,  1032,  1035,  1038,
    1047,  1048
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ZAHL", "NAME", "PATH", "LITERAL", "IP",
  "ATM", "ATM_PVC_ADDRESS", "IP_NETMASK", "AUTO_ROUTER", "NO_DNS",
  "ONLY_DNS", "NUMHOSTS", "PORTBASE", "PORTRANGE", "ROUTERTIMEOUT",
  "HETERO", "EXCHANGE_ORDER", "SPLITSIZE", "ISEND_NUM", "TCP", "ATM_SVC",
  "ATM_PVC", "DOPPELPUNKT", "MINUS", "KOMMA", "RETURN", "CONNECTIONS",
  "PAIR", "OPTIONS", "NICDEF", "METAHOST", "GLEICH", "CONNTO",
  "CONNTO_BIDIR", "GETIP", "NODES", "ROUTERS", "NET", "SECONDARY_DEVICE",
  "EXTRAPROCS", "MAX", "NETMASK", "NOWATCHDOG", "SMI_VERBOSE", "SMI_DEBUG",
  "'('", "')'", "'{'", "'}'", "';'", "'!'", "$accept", "script", "header",
  "hostsize", "appargs", "hosts", "proccount", "host", "optsection",
  "optionlist", "option", "netdef", "secdevdef", "secdevopts",
  "secdevoptlist", "secdevopt", "nicdef", "nicname", "nicaddr",
  "metahostdef", "metahostdefhead", "optreturn", "mhoptionlist",
  "mhoption", "nodelist", "nodelistentry", "extraproclist", "optuser",
  "optargs", "extraproclistentry", "nodeexec", "node", "nicrange",
  "netlist", "netnames", "procnum", "niclist", "nic_iter", "nic",
  "connsection", "autooption", "subsections", "subsection", "subhead",
  "routerexec", "subbody", "connectiondef", "sockconndef", "connto",
  "remotenics", "emptyLines", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,    40,    41,
     123,   125,    59,    33
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    54,    55,    56,    57,    58,    58,    59,    59,    60,
      60,    61,    62,    63,    63,    64,    64,    64,    64,    64,
      64,    64,    64,    64,    64,    64,    65,    66,    67,    67,
      68,    68,    69,    69,    69,    69,    69,    69,    70,    71,
      72,    72,    72,    72,    73,    74,    75,    75,    76,    76,
      77,    77,    77,    77,    77,    77,    78,    78,    79,    79,
      80,    80,    81,    81,    82,    82,    83,    84,    84,    84,
      85,    85,    85,    86,    86,    87,    87,    88,    88,    89,
      89,    90,    91,    91,    92,    92,    92,    92,    92,    93,
      93,    93,    94,    94,    95,    95,    96,    96,    97,    98,
      98,    99,    99,   100,   101,   102,   102,   102,   103,   103,
     104,   104
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     3,     2,     3,     0,     1,     1,     2,     1,
       1,     4,     3,     1,     2,     1,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     2,     6,     3,     0,     3,
       1,     3,     3,     5,     3,     1,     1,     1,     3,     1,
       2,     2,     2,     4,     6,     2,     0,     1,     3,     4,
       3,     3,     3,     3,     3,     3,     1,     3,     2,     1,
       1,     3,     0,     1,     0,     1,     9,     0,     2,     4,
       4,     7,     5,     0,     3,     0,     4,     1,     3,     0,
       1,     1,     1,     3,     1,     3,     1,     1,     3,     3,
       4,     5,     1,     1,     1,     2,     2,     1,     6,     1,
       1,     1,     2,     2,     3,     0,     1,     1,     1,     2,
       1,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,     0,     0,     0,     0,     1,     0,     0,     0,
       3,     7,     4,     0,     0,     2,     9,    10,     5,     8,
       0,     0,     0,     0,     0,     0,    15,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    46,     0,     6,
       0,     0,     0,     0,     0,     0,     0,    39,     0,    45,
       0,    28,    14,    24,    25,    22,    23,    47,     0,     0,
       0,    89,    94,    97,    11,    17,    16,    18,    19,    20,
      21,     0,     0,     0,     0,    38,     0,     0,    27,    46,
      92,    93,   110,     0,    90,     0,    95,    87,    84,    86,
     105,    81,    82,    96,   101,     0,    40,    41,    42,     0,
       0,     0,     0,     0,    35,    36,    37,     0,    30,     0,
      91,   111,     0,     0,     0,   106,   107,     0,     0,   102,
     103,     0,     0,     0,     0,     0,    29,     0,     0,     0,
       0,     0,     0,     0,     0,    88,    85,   108,   104,    83,
      43,    26,    32,     0,    34,    31,     0,     0,     0,     0,
      44,    46,   100,    99,     0,   109,     0,    50,    51,    55,
      79,     0,    52,    56,    59,    53,     0,    54,    60,    48,
      98,    33,    80,     0,    75,    58,     0,    79,     0,    49,
      79,     0,     0,    67,    57,     0,    61,    73,     0,     0,
       0,    70,     0,     0,    72,    77,     0,    75,    68,     0,
       0,     0,    76,    67,    64,    62,    74,    78,    71,    65,
      69,    63,     0,    64,    66
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,     2,     3,     4,    40,    10,    18,    11,     8,    31,
      32,    33,    34,    78,   107,   108,    35,    48,    75,    36,
      37,    58,   132,   133,   162,   163,   167,   212,   210,   168,
     191,   164,   194,   183,   196,   174,    90,    91,    92,    15,
      83,    61,    62,    63,   154,    93,    94,    95,   117,   138,
      84
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -158
static const short int yypact[] =
{
      -5,    10,    22,     8,    38,    25,  -158,    32,    35,     0,
      38,  -158,  -158,    -3,    33,  -158,  -158,  -158,    60,  -158,
      51,    64,    65,    66,    67,    68,  -158,    70,    71,    73,
      74,    -3,  -158,    44,    52,    54,    55,    56,    -4,  -158,
      57,    58,    59,    61,    62,    63,    69,  -158,    26,  -158,
      75,    40,  -158,  -158,  -158,  -158,  -158,  -158,    31,    34,
      88,    72,  -158,    48,  -158,  -158,  -158,  -158,  -158,  -158,
    -158,    86,    87,    85,    50,  -158,    92,   -11,  -158,    56,
    -158,  -158,  -158,    76,    77,    95,  -158,    78,    81,  -158,
      -8,    80,  -158,    48,  -158,    82,  -158,  -158,  -158,    96,
      97,    79,    83,    84,  -158,  -158,  -158,    89,    93,     2,
      77,  -158,   105,   106,   108,  -158,  -158,    48,    48,  -158,
    -158,    90,   109,   111,   112,   113,  -158,   -11,    91,    94,
      98,    99,   100,   101,    19,  -158,  -158,  -158,    48,  -158,
    -158,  -158,  -158,   103,  -158,  -158,    53,    -2,    -2,   115,
    -158,    56,  -158,  -158,   102,  -158,   118,  -158,  -158,  -158,
       5,   120,   104,  -158,  -158,   104,   110,   107,  -158,     2,
    -158,  -158,  -158,   122,   -29,  -158,    -2,   119,   115,  -158,
     119,   114,    48,   116,  -158,   117,  -158,   121,   123,   124,
     131,  -158,   132,    48,  -158,  -158,   -17,   125,   127,   129,
     126,   136,  -158,   116,   137,   140,  -158,  -158,  -158,  -158,
    -158,  -158,   130,   137,  -158
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
    -158,  -158,  -158,  -158,  -158,  -158,  -158,   135,  -158,  -158,
     128,  -158,  -158,  -158,    20,  -158,  -158,  -158,  -158,  -158,
    -158,   -78,   -53,  -158,     1,   -30,  -158,  -158,   -65,   -28,
     -47,    -1,  -158,   -40,  -158,  -157,  -117,  -158,    43,  -158,
    -158,  -158,   133,  -158,  -158,  -158,   134,  -158,  -158,  -158,
     138
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
     137,   109,   160,    16,   101,   102,   128,    59,   172,     1,
     201,   181,    20,     5,    21,    22,    23,    24,    25,   182,
     185,   155,     6,   187,   152,    26,    60,   115,   116,    27,
      28,   173,   202,   103,   104,   105,   106,    29,    30,     7,
     129,   130,     9,    17,   131,   153,    80,    81,    71,    72,
      73,   161,    87,    12,    41,    88,    89,   157,   158,   159,
      13,    38,    82,    74,    14,   189,    39,    42,    43,    44,
      45,    46,    53,   169,    47,    49,   200,    50,    51,    76,
      54,    79,    55,    56,    57,    64,    65,    66,    77,    67,
      68,    69,    85,    96,    98,    97,   100,    70,    99,   112,
     121,   122,    60,   113,    82,   111,   114,   118,   134,   135,
     120,   136,   141,   123,   142,   143,   179,   124,   125,   166,
     127,   171,   172,   144,   160,   146,   180,   195,   147,   156,
     170,   176,   148,   149,   178,   177,   198,   199,   126,   140,
     207,   190,   192,   209,   211,    19,   184,   145,   214,   165,
     186,   150,   204,   151,   205,   213,   208,   203,     0,    52,
     175,   139,   188,     0,     0,   181,     0,     0,     0,   193,
       0,     0,     0,   197,     0,   206,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    86,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   110,     0,     0,     0,     0,     0,   119
};

static const short int yycheck[] =
{
     117,    79,     4,     3,    15,    16,     4,    11,     3,    14,
      27,    40,    15,     3,    17,    18,    19,    20,    21,    48,
     177,   138,     0,   180,     5,    28,    30,    35,    36,    32,
      33,    26,    49,    44,    45,    46,    47,    40,    41,    31,
      38,    39,     4,    43,    42,    26,    12,    13,    22,    23,
      24,    53,     4,    28,     3,     7,     8,     4,     5,     6,
      28,    28,    28,    37,    29,   182,     6,     3,     3,     3,
       3,     3,    28,   151,     4,     4,   193,     4,     4,     4,
      28,    50,    28,    28,    28,    28,    28,    28,    48,    28,
      28,    28,     4,     7,     9,     8,     4,    28,    48,     4,
       4,     4,    30,    25,    28,    28,    25,    27,     3,     3,
      28,     3,     3,    34,     3,     3,   169,    34,    34,     4,
      27,     3,     3,    10,     4,    34,     4,     4,    34,    26,
      28,    27,    34,    34,    27,    25,     5,     5,    49,    49,
       4,    25,    25,     6,     4,    10,   176,   127,   213,   148,
     178,    51,    25,    52,    25,    25,   203,   197,    -1,    31,
     161,   118,    48,    -1,    -1,    40,    -1,    -1,    -1,    48,
      -1,    -1,    -1,    49,    -1,    49,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    61,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    83,    -1,    -1,    -1,    -1,    -1,    93
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    14,    55,    56,    57,     3,     0,    31,    62,     4,
      59,    61,    28,    28,    29,    93,     3,    43,    60,    61,
      15,    17,    18,    19,    20,    21,    28,    32,    33,    40,
      41,    63,    64,    65,    66,    70,    73,    74,    28,     6,
      58,     3,     3,     3,     3,     3,     3,     4,    71,     4,
       4,     4,    64,    28,    28,    28,    28,    28,    75,    11,
      30,    95,    96,    97,    28,    28,    28,    28,    28,    28,
      28,    22,    23,    24,    37,    72,     4,    48,    67,    50,
      12,    13,    28,    94,   104,     4,    96,     4,     7,     8,
      90,    91,    92,    99,   100,   101,     7,     8,     9,    48,
       4,    15,    16,    44,    45,    46,    47,    68,    69,    75,
     104,    28,     4,    25,    25,    35,    36,   102,    27,   100,
      28,     4,     4,    34,    34,    34,    49,    27,     4,    38,
      39,    42,    76,    77,     3,     3,     3,    90,   103,    92,
      49,     3,     3,     3,    10,    68,    34,    34,    34,    34,
      51,    52,     5,    26,    98,    90,    26,     4,     5,     6,
       4,    53,    78,    79,    85,    78,     4,    80,    83,    75,
      28,     3,     3,    26,    89,    85,    27,    25,    27,    76,
       4,    40,    48,    87,    79,    89,    83,    89,    48,    90,
      25,    84,    25,    48,    86,     4,    88,    49,     5,     5,
      90,    27,    49,    87,    25,    25,    49,     4,    84,     6,
      82,     4,    81,    25,    82
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)		\
   ((Current).first_line   = (Rhs)[1].first_line,	\
    (Current).first_column = (Rhs)[1].first_column,	\
    (Current).last_line    = (Rhs)[N].last_line,	\
    (Current).last_column  = (Rhs)[N].last_column)
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if defined (YYMAXDEPTH) && YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  register short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;


  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short int *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 3:
#line 161 "rconf_parser.y"
    {
                      PDEBUG("** Header gelesen\n"); 
		      if (!metaHostFound) {
			  RERROR1("WARNING:metahostname \"%s\" not found in config file\nThe exact name of the metahost most be provided with the -metahost <metahostname> option\n", pars_rconf->my_metahostname);
			  /*	  YYABORT;*/
		      }
	          if (countHosts != pars_rconf->nbr_metahosts ) {
 		      	RERROR2("metaconfig error: %d metahosts specified but %d declared\n", pars_rconf->nbr_metahosts, countHosts);
			CONF_ERROR("");
     		  }

		      checkHeaderSection();
                  ;}
    break;

  case 4:
#line 176 "rconf_parser.y"
    {
		      resetGlobals();
		      PDEBUG1("NUMHOSTS = %d\n",yyvsp[-1].zahl);  
		      pars_rconf->nbr_metahosts=yyvsp[-1].zahl;
		      pars_rconf->np =0;
		      pars_rconf->nrp =0;
		      pars_rconf->netDefList=0;
		      /* initialize flag */
		      if ((pars_rconf->my_metahostname!=0) && (strcmp(pars_rconf->my_metahostname, "") != 0))
			  metaHostFound=0;
		      else metaHostFound=1; /* we have no metahost given by now */
                  ;}
    break;

  case 5:
#line 189 "rconf_parser.y"
    {yyval.pointer_to_char=0;}
    break;

  case 6:
#line 190 "rconf_parser.y"
    { yyval.pointer_to_char=newString(yyvsp[0].string);;}
    break;

  case 9:
#line 195 "rconf_parser.y"
    { yyval.pointer_to_int=(int *) malloc(sizeof( int));  *yyval.pointer_to_int=yyvsp[0].zahl; ;}
    break;

  case 10:
#line 196 "rconf_parser.y"
    { yyval.pointer_to_int=0; ;}
    break;

  case 11:
#line 198 "rconf_parser.y"
    {
                      int h_id,np;

		      if (yyvsp[-2].pointer_to_int) np=*yyvsp[-2].pointer_to_int;
		      else np=-1; /* MAX */
		      
		      /* check if the meta host is already declared */
		      if ( rh_getMetahostId(yyvsp[-3].string) != -1 ) {
			yyerror("");
			RERROR1("meta host %s already declared. \n", yyvsp[-3].string);
			YYABORT;
		      }
		      h_id = rh_addMetahost(yyvsp[-3].string, np);
		      /* deviceType defaults to ch_shmem */
		      metahostlist[h_id].deviceType=DEVICE_SHMEM;
		      /* a SMP machine is a single node */
		      metahostlist[h_id].numNodes=1;
		      metahostlist[h_id].appArgs=yyvsp[-1].pointer_to_char;
		      pars_rconf->my_nrp[h_id]=0;

		      if (np != -1) pars_rconf->np += np;
		      
		      strcpy (pars_rconf->metahostnames[countHosts], yyvsp[-3].string); 
		      pars_rconf->nrp_metahost[countHosts]=0; /* initialize ! */
		      pars_rconf->np_metahost[countHosts]=np;
		      
		      PDEBUG2("host %s has %d MPI-processes\n", yyvsp[-3].string, np);

		      if ( strcmp(yyvsp[-3].string,pars_rconf->my_metahostname) == 0) {
			  PDEBUG1("i am host number %d\n", countHosts);
			  pars_rconf->my_metahost_rank=h_id;
			  metaHostFound=1;
		      }		      
		      countHosts++;
                  ;}
    break;

  case 12:
#line 235 "rconf_parser.y"
    { 
                      /* check metahosts for empty nodelists */
                      int h;
		      for (h=0; h < rh_getNumMetahosts(); h++) {
			  if (!metahostlist[h].nodeList) {
			      /* we need at least one smp node */
			      Snode *nl=rh_newNodeClear();;
			      metahostlist[h].nodeList=nl;
			      /* node and metahost are the same */
			      nl->nodeName=newString(metahostlist[h].hostname);
			      nl->nicList=0;
			      nl->next=0;
			      nl->netList=0;
			      nl->numNets=0;
			      nl->executable=0;

			      metahostlist[h].numNodes=1;
			  }
		      }
                  ;}
    break;

  case 16:
#line 262 "rconf_parser.y"
    {                  
                      PDEBUG1("ROUTERTIMEOUT is %d\n", yyvsp[-1].zahl);
		      pars_rconf->router_timeout=yyvsp[-1].zahl;
		  ;}
    break;

  case 17:
#line 266 "rconf_parser.y"
    {                  
                      PDEBUG1("PORTBASE is %d\n", yyvsp[-1].zahl);
		      pars_rconf->tcp_portbase=yyvsp[-1].zahl;
		  ;}
    break;

  case 18:
#line 270 "rconf_parser.y"
    {                  
                      PDEBUG1("HETERO is %d\n", yyvsp[-1].zahl);
		      pars_rconf->isHetero=yyvsp[-1].zahl;
		  ;}
    break;

  case 19:
#line 274 "rconf_parser.y"
    {                  
                      PDEBUG1("EXCHANGE_ORDER is %d\n", yyvsp[-1].zahl);
		      pars_rconf->exchangeOrder=yyvsp[-1].zahl;
		  ;}
    break;

  case 20:
#line 278 "rconf_parser.y"
    {
		      PDEBUG1("SPLITSIZE is %d\n", yyvsp[-1].zahl);
		      pars_rconf->split_size=yyvsp[-1].zahl;
                  ;}
    break;

  case 21:
#line 282 "rconf_parser.y"
    {
		      PDEBUG1("ISEND_NUM is %d\n", yyvsp[-1].zahl);
		      pars_rconf->isend_num=yyvsp[-1].zahl;
                  ;}
    break;

  case 22:
#line 286 "rconf_parser.y"
    {
		      /*save NIC definition in nicDefList */
		      SnicDef *nicDefTEMP;

		      nicDefTEMP = yyvsp[-1].pointer_to_snicdef;
		      nicDefTEMP->next = nicDefList;
		      nicDefList = nicDefTEMP;

		  ;}
    break;

  case 23:
#line 295 "rconf_parser.y"
    {;}
    break;

  case 24:
#line 296 "rconf_parser.y"
    {;}
    break;

  case 25:
#line 297 "rconf_parser.y"
    {;}
    break;

  case 26:
#line 300 "rconf_parser.y"
    {
  SnetDefList *snetl=0;
                      /* check for first net */
  if (!pars_rconf->netDefList){
    pars_rconf->netDefList= malloc (sizeof(SnetDefList));   
    snetl=pars_rconf->netDefList;
  }else {
    snetl=pars_rconf->netDefList;
    while (snetl->next) snetl=snetl->next;
    snetl->next= malloc (sizeof(SnetDefList));
    snetl=snetl->next;
  }
  snetl->next=0;
  snetl->name=newString(yyvsp[-4].string);
  snetl->type=newString(yyvsp[-2].string);
  snetl->metric=yyvsp[0].zahl;
                  ;}
    break;

  case 27:
#line 319 "rconf_parser.y"
    {
    if( strcmp( yyvsp[-1].string, "ch_shmem" ) == 0 )
	pars_rconf->secondaryDevice = DEVICE_SHMEM;
    else if( strcmp( yyvsp[-1].string, "ch_smi" ) == 0 )
	pars_rconf->secondaryDevice = DEVICE_SMI;
    else if( strcmp( yyvsp[-1].string, "ch_usock" ) == 0 )
	pars_rconf->secondaryDevice = DEVICE_USOCK;
    else {
	fprintf( stderr, "%s is not a known type for a secondary device\n", yyvsp[-1].string );
	YYABORT;
    }
;}
    break;

  case 29:
#line 334 "rconf_parser.y"
    {
                ;}
    break;

  case 32:
#line 342 "rconf_parser.y"
    {
                (pars_rconf->secondaryDeviceOpts).portbase =yyvsp[0].zahl;
                ;}
    break;

  case 33:
#line 345 "rconf_parser.y"
    {
                (pars_rconf->secondaryDeviceOpts).portRangeLow  =yyvsp[-2].zahl;
                (pars_rconf->secondaryDeviceOpts).portRangeHigh =yyvsp[0].zahl;
                ;}
    break;

  case 34:
#line 349 "rconf_parser.y"
    {
		    char netmaskString[40];
		    char *t;
		    int no_network_bits;

		    strcpy( netmaskString, yyvsp[0].string );
		    
		    /* find slash in netmask */
		    t = netmaskString;
		    while( *t != '/' )
			t++;

		    
		    /* get number of host bits in netmask */
		    no_network_bits = atoi( t+1 );
		    (pars_rconf->secondaryDeviceOpts).netmask_no_host_bits = 32 - no_network_bits;

		    /* get IP address in netmask */
		    *t = '\0';
		    (pars_rconf->secondaryDeviceOpts).netmask_ip = htonl( inet_addr( netmaskString ) );
                ;}
    break;

  case 35:
#line 370 "rconf_parser.y"
    { (pars_rconf->secondaryDeviceOpts).nowatchdog = 1;
                ;}
    break;

  case 36:
#line 372 "rconf_parser.y"
    { (pars_rconf->secondaryDeviceOpts).smi_verbose = 1;
                ;}
    break;

  case 37:
#line 374 "rconf_parser.y"
    { (pars_rconf->secondaryDeviceOpts).smi_debug = 1;
                ;}
    break;

  case 38:
#line 378 "rconf_parser.y"
    {
                      /* make a new SnicDef, initialize it with values from nicname and nicaddr and
			 return it value of nicdef */
                      SnicDef *nicDefTEMP;

		      nicDefTEMP = (SnicDef *)malloc( sizeof(SnicDef) );
		      strcpy( nicDefTEMP->nicName, yyvsp[-1].string );
		      nicDefTEMP->nic = yyvsp[0].pointer_to_snic;
		      nicDefTEMP->next = 0;

		      yyval.pointer_to_snicdef = nicDefTEMP;
                  ;}
    break;

  case 39:
#line 392 "rconf_parser.y"
    {
                      /* check given name for nic and return it as value of nicname */
		      PDEBUG1("name ist %s\n",yyvsp[0].string); 
		      if ( strlen(yyvsp[0].string) >= MAX_NICDEF_NAME_LEN ) {
			  RERROR1("identifier too long (<%d)\n",MAX_NICDEF_NAME_LEN);
			  CONF_ERROR("");
		      }

		      strcpy( yyval.string, yyvsp[0].string );
                  ;}
    break;

  case 40:
#line 404 "rconf_parser.y"
    {
                      /* allocate data structure for nicaddr-data (type and address) and
			 return it at reduce time as value of non-terminal symbol nicaddr;
			 same comment for other alternatives to this rule */

                      Snic *nicTEMP;

                      PDEBUG1("adresse ist %s\n",yyvsp[0].string);
		      nicTEMP = (Snic *)malloc( sizeof(Snic) );
		      nicTEMP->port = 0;
		      nicTEMP->next = 0;
		      nicTEMP->nodeName = 0;
		      nicTEMP->nicType=ADR_TCP; 
		      strcpy(nicTEMP->nicAddress, yyvsp[0].string);

		      yyval.pointer_to_snic = nicTEMP;
                  ;}
    break;

  case 41:
#line 421 "rconf_parser.y"
    {
                      Snic *nicTEMP;

                      PDEBUG1("adresse ist %s\n",yyvsp[0].string);
		      nicTEMP = (Snic *)malloc( sizeof(Snic) );
		      nicTEMP->port = 0;
		      nicTEMP->next = 0;
		      nicTEMP->nodeName = 0;
		      nicTEMP->nicType=ADR_ATM_SVC;
		      strcpy(nicTEMP->nicAddress, yyvsp[0].string);

		      yyval.pointer_to_snic = nicTEMP;
                  ;}
    break;

  case 42:
#line 434 "rconf_parser.y"
    {
                      Snic *nicTEMP;

		      PDEBUG1("adresse ist %s\n",yyvsp[0].string);
		      nicTEMP = (Snic *)malloc( sizeof(Snic) );
		      nicTEMP->port = 0;
		      nicTEMP->next = 0;
		      nicTEMP->nodeName = 0;
		      nicTEMP->nicType=ADR_ATM_PVC;
		      strcpy(nicTEMP->nicAddress, yyvsp[0].string);

		      yyval.pointer_to_snic = nicTEMP;
		  ;}
    break;

  case 43:
#line 447 "rconf_parser.y"
    {
                      Snic *nicTEMP;
		      struct in_addr intmp;
		      struct hostent *he= gethostbyname( yyvsp[-1].string);  

		      nicTEMP = (Snic *)malloc( sizeof(Snic) );
		      nicTEMP->port = 0;
		      nicTEMP->next = 0;
		      nicTEMP->nodeName = 0;

		      if (he) {
			  nicTEMP->nicType=ADR_TCP;
			  memcpy(&intmp, he->h_addr, he->h_length);
			  strcpy(nicTEMP->nicAddress, inet_ntoa(intmp));
		      }
		      else {
			  RERROR1("warning: could not resolve hostname %s\n", yyvsp[-1].string);
			  strcpy(nicTEMP->nicAddress, yyvsp[-1].string);
		      }

		      yyval.pointer_to_snic = nicTEMP;
		  ;}
    break;

  case 44:
#line 471 "rconf_parser.y"
    { 
		      metahostlist[metaHostDefH_id].nodeList=metaHostDefNodelist;
		      metahostlist[metaHostDefH_id].numNodes=0;
		      metahostlist[metaHostDefH_id].devOptions=newString(metaHostDefDevOption);
		      /* count nodes */
		      while (metaHostDefNodelist) {
			  metahostlist[metaHostDefH_id].numNodes++;
			  metaHostDefNodelist->npFixed=metaHostDefNodelist->numRouters
			  				+metaHostDefNodelist->npExtraProcs;
			  if (metaHostDefNodelist->maxNumProcs)
			  	if (metaHostDefNodelist->npFixed > metaHostDefNodelist->maxNumProcs) {
					RERROR2("meta config error: number of fixed processes (%d) exeed number of processors (%d) ",metaHostDefNodelist->npFixed,metaHostDefNodelist->maxNumProcs);
					RERROR2("on node %s meta host %s\n",metaHostDefNodelist->nodeName,metahostlist[metaHostDefH_id].hostname);
					CONF_ERROR("")
				}
			  metaHostDefNodelist=metaHostDefNodelist->next;
		      }
		      
		      if ( strcmp(metaHostDefType, "ch_shmem") == 0) {
			  metahostlist[metaHostDefH_id].deviceType=DEVICE_SHMEM;
			  if ( metahostlist[metaHostDefH_id].numNodes != 1) 
			      CONF_ERROR("Metahost with type ch_shmem must have exactly one node");
		      }
		      
		      if ( strcmp(metaHostDefType, "ch_smi") == 0) {
			  /*	  if ( metahostlist[metaHostDefH_id].numNodes > 1) */
			      metahostlist[metaHostDefH_id].deviceType=DEVICE_SMI;
			      /* else metahostlist[metaHostDefH_id].deviceType=DEVICE_SHMEM; */
		      }

		      if ( strcmp(metaHostDefType, "ch_usock") == 0) {
			  metahostlist[metaHostDefH_id].deviceType=DEVICE_USOCK;
		      }

		      if ( strcmp(metaHostDefType, "ch_gm") == 0) {
			  metahostlist[metaHostDefH_id].deviceType=DEVICE_GM;
		      }

		      if ( strcmp(metaHostDefType, "ch_mpx") == 0) {
			  metahostlist[metaHostDefH_id].deviceType=DEVICE_MPX;
		      }

		      if ( strcmp(metaHostDefENVFILE,"") != 0 ) {
			  metahostlist[metaHostDefH_id].envFile=newString(metaHostDefENVFILE);
		      }
		      
		      if ( strcmp(metaHostDefEXECPATH,"") != 0 ) {
			  metahostlist[metaHostDefH_id].execPath=newString(metaHostDefEXECPATH);
		      }

		      if ( strcmp(metaHostDefEXECNAME,"") != 0 ) {
			  metahostlist[metaHostDefH_id].execName=newString(metaHostDefEXECNAME);
		      }

		      if ( strcmp(metaHostDefEXECARGS,"") != 0 ) {
			  metahostlist[metaHostDefH_id].appArgs=newString(metaHostDefEXECARGS);
		      }

		      if ( strcmp(metaHostDefCONFPATH,"") != 0 ) {
			  metahostlist[metaHostDefH_id].confPath=newString(metaHostDefCONFPATH);
		      }
		      if ( strcmp(metaHostDefCONFNAME,"") != 0 ) {
			  metahostlist[metaHostDefH_id].confName=newString(metaHostDefCONFNAME);
		      }

		      if ( strcmp(metaHostDefMETAKEY,"") != 0 ) {
			  metahostlist[metaHostDefH_id].metaKey=newString(metaHostDefMETAKEY);
		      }

		      if ( strcmp(metaHostDefFRONTEND,"") != 0 ) {
			  metahostlist[metaHostDefH_id].frontend=newString(metaHostDefFRONTEND);
		      }
		      if ( strcmp(metaHostDefUSER,"") != 0 ) {
			  metahostlist[metaHostDefH_id].user=newString(metaHostDefUSER);
		      }
		      
		      if ( strcmp(metaHostDefMPIROOT,"") != 0 ) {
			  metahostlist[metaHostDefH_id].mpiRoot=newString(metaHostDefMPIROOT);
		      }
		      if ( metaHostDefROUTEREXEC ) {
			  metahostlist[metaHostDefH_id].routerExec=metaHostDefROUTEREXEC;
			  metaHostDefROUTEREXEC=0;
		      }
                  ;}
    break;

  case 45:
#line 557 "rconf_parser.y"
    {     
				metaHostDefNodelist = NULL;
				metaHostDefH_id=rh_getMetahostId( yyvsp[0].string);
				if (metaHostDefH_id == -1) {
					yyerror("");
					RERROR1(" meta host %s defined but not declared \n",yyvsp[0].string);
					YYABORT;
				}
				if ( metahostlist[metaHostDefH_id].valid ) {
					yyerror("");
					RERROR1(" meta host %s is already defined, double definition is not allowed \n",yyvsp[0].string);
					YYABORT;			
				}
				metahostlist[metaHostDefH_id].valid=1;
				
				strcpy(metaHostDefFRONTEND,"n.a.");
				strcpy(metaHostDefENVFILE,"n.a.");
				strcpy(metaHostDefUSER,"n.a.");
				strcpy(metaHostDefEXECPATH,"n.a.");
				strcpy(metaHostDefEXECNAME,"n.a.");
				strcpy(metaHostDefCONFPATH,"n.a.");
				strcpy(metaHostDefCONFNAME,"n.a.");
				strcpy(metaHostDefMETAKEY,"n.a.");
				strcpy(metaHostDefMPIROOT,"n.a.");
				
				/* metaHostDefEXECARGS should be empty */
				strcpy(metaHostDefEXECARGS,"");
		      
				metaHostDefROUTEREXEC=0;
				strcpy(metaHostDefDevOption,"");
			;}
    break;

  case 47:
#line 591 "rconf_parser.y"
    {;}
    break;

  case 50:
#line 598 "rconf_parser.y"
    { 
				int ok=0;
				if (strcmp (yyvsp[-2].string,"TYPE")==0) { 
					ok=1; 
					strcpy(metaHostDefType, yyvsp[0].string); 
				}
				if (strcmp (yyvsp[-2].string,"FRONTEND")==0) { ok=1; strcpy(metaHostDefFRONTEND, yyvsp[0].string); }
				if (strcmp (yyvsp[-2].string,"EXECNAME")==0) { ok=1; strcpy(metaHostDefEXECNAME, yyvsp[0].string); }
				if (strcmp (yyvsp[-2].string,"CONFNAME")==0) { ok=1; strcpy(metaHostDefCONFNAME, yyvsp[0].string); }
				if (strcmp (yyvsp[-2].string,"METAKEY")==0)  { ok=1; strcpy(metaHostDefMETAKEY,  yyvsp[0].string); }
				if (strcmp (yyvsp[-2].string,"USER")==0) { ok=1; strcpy(metaHostDefUSER, yyvsp[0].string); }
				if (!ok) {
					fprintf(stderr, "%s is not an metahost option or right value not a valid string",yyvsp[-2].string);
					YYABORT;
				}
			;}
    break;

  case 51:
#line 614 "rconf_parser.y"
    {
				int ok=0;		       
				if (strcmp (yyvsp[-2].string,"EXECPATH")==0) { ok=1; strcpy(metaHostDefEXECPATH, yyvsp[0].string); }
				if (strcmp (yyvsp[-2].string,"CONFPATH")==0) { ok=1; strcpy(metaHostDefCONFPATH, yyvsp[0].string); }
				if (strcmp (yyvsp[-2].string,"ENVFILE")==0) { ok=1; strcpy(metaHostDefENVFILE, yyvsp[0].string); }
				if (strcmp (yyvsp[-2].string,"MPIROOT")==0) { ok=1; strcpy(metaHostDefMPIROOT, yyvsp[0].string); }
				if (strcmp (yyvsp[-2].string,"ROUTEREXEC")==0) { ok=1; metaHostDefROUTEREXEC=newString(yyvsp[0].string); }
				if (!ok) {
				fprintf(stderr, "%s is not an metahost option or right value not a valid string",yyvsp[-2].string);
				YYABORT;
				}
			;}
    break;

  case 52:
#line 626 "rconf_parser.y"
    {
				if(metaHostDefNodelist!=NULL)
					metaHostDefNodelist = sortInNodeList( metaHostDefNodelist, yyvsp[0].pointer_to_snode );
				else
					metaHostDefNodelist = yyvsp[0].pointer_to_snode;
			;}
    break;

  case 53:
#line 632 "rconf_parser.y"
    {
				if(metaHostDefNodelist!=NULL)
					metaHostDefNodelist = sortInNodeList( metaHostDefNodelist, yyvsp[0].pointer_to_snode );
				else
					metaHostDefNodelist = yyvsp[0].pointer_to_snode;
			;}
    break;

  case 54:
#line 638 "rconf_parser.y"
    {
				metahostlist[metaHostDefH_id].procgroup=1;
			;}
    break;

  case 55:
#line 641 "rconf_parser.y"
    {
				if (strcmp(yyvsp[-2].string,"DEVOPTS")==0) {
					strcpy( metaHostDefDevOption,  yyvsp[0].string);
				}
				else if (strcmp (yyvsp[-2].string,"EXECARGS")==0) {
					strcpy(metaHostDefEXECARGS, yyvsp[0].string);
				}
				else {
					CONF_ERROR("unknown metahost option");
				}
			;}
    break;

  case 56:
#line 653 "rconf_parser.y"
    { yyval.pointer_to_snode = yyvsp[0].pointer_to_snode; ;}
    break;

  case 57:
#line 654 "rconf_parser.y"
    {
		      Snode *nodelistTEMP;
		      
		      nodelistTEMP = yyvsp[-2].pointer_to_snode;
		      if (excludeNode) {
		      	if (! rh_findNodeName(yyvsp[-2].pointer_to_snode,yyvsp[0].pointer_to_snode->nodeName) ) {
				RERROR1("metaconfig error: node %s excluded but not previously defined\n",yyvsp[0].pointer_to_snode->nodeName);
				CONF_ERROR("")
			}
		      }
		      nodelistTEMP = sortInNodeList( nodelistTEMP, yyvsp[0].pointer_to_snode );

		      yyval.pointer_to_snode = nodelistTEMP;
		  ;}
    break;

  case 58:
#line 669 "rconf_parser.y"
    {
                         yyval.pointer_to_snode=yyvsp[0].pointer_to_snode;
			 /*                        RERROR1("node %s excluded\n", $2->nodeName);*/
			 excludeNode=1;
                  ;}
    break;

  case 59:
#line 674 "rconf_parser.y"
    {
		  yyval.pointer_to_snode=yyvsp[0].pointer_to_snode;
		  excludeNode=0;                 
		;}
    break;

  case 60:
#line 679 "rconf_parser.y"
    { 
		;}
    break;

  case 61:
#line 681 "rconf_parser.y"
    {
		  ;}
    break;

  case 62:
#line 684 "rconf_parser.y"
    { strcpy(yyval.string,""); ;}
    break;

  case 63:
#line 685 "rconf_parser.y"
    { strcpy(yyval.string,yyvsp[0].string); ;}
    break;

  case 64:
#line 687 "rconf_parser.y"
    { strcpy(yyval.string,""); ;}
    break;

  case 65:
#line 688 "rconf_parser.y"
    {strcpy(yyval.string,yyvsp[0].string);;}
    break;

  case 66:
#line 690 "rconf_parser.y"
    {
		  	Snode *pn;
			char * extraProcListEntryTEMP;
			int i;
			
			pn=rh_findNodeName(metaHostDefNodelist, yyvsp[-8].string);
			if (!pn) { /* node not defined? */
				RERROR2("metaconfig error in EXTRAPROC: %s not in nodelist of meta host %s.\n", yyvsp[-8].string, metahostlist[metaHostDefH_id].hostname);	
				CONF_ERROR("")
			}
			extraProcListEntryTEMP=newString("");
			ALLOC_SPRINTF2(extraProcListEntryTEMP,"%s %d ", yyvsp[-8].string,yyvsp[-6].zahl);
			extraProcListEntryTEMP=stringAppend(extraProcListEntryTEMP,yyvsp[-4].string); /* PATH */
			if (strcmp(yyvsp[-2].string,"") != 0) {
				extraProcListEntryTEMP=stringAppend3(extraProcListEntryTEMP," ",yyvsp[-2].string); /* optuser */
			}
			if (strcmp(yyvsp[0].string,"") != 0) {
				extraProcListEntryTEMP=stringAppend3(extraProcListEntryTEMP," ",yyvsp[0].string); /* optarg */
			}
			if (!pn->extraProcList) {
				pn->extraProcList=malloc(sizeof(char**));
				pn->extraProcList[0]=0;
			}
			for (i=0; pn->extraProcList[i]!=0; i++)  /**/;
			pn->extraProcList = realloc(pn->extraProcList[i],sizeof(char*)*(i+1+1));
			pn->extraProcList[i]=extraProcListEntryTEMP;
			pn->extraProcList[i+1]=0;
			pn->npExtraProcs +=yyvsp[-6].zahl;
		;}
    break;

  case 67:
#line 720 "rconf_parser.y"
    { /* no executable given for this node */
			strcpy(yyval.string_array[0],""); 
			strcpy(yyval.string_array[1],"");
		;}
    break;

  case 68:
#line 724 "rconf_parser.y"
    {
			strcpy(yyval.string_array[0],yyvsp[0].string);
			strcpy(yyval.string_array[1],"");
		;}
    break;

  case 69:
#line 729 "rconf_parser.y"
    {
			strcpy(yyval.string_array[0],yyvsp[-2].string);
			strcpy(yyval.string_array[1],yyvsp[0].string);
		;}
    break;

  case 70:
#line 734 "rconf_parser.y"
    { 
                /* no niclist found */
				Snode *nodeTEMP;
	  
				nodeTEMP = rh_newNodeClear();
				nodeTEMP->nodeName= newString (yyvsp[-3].string);
				nodeTEMP->nicList = 0;
				nodeTEMP->maxNumProcs=yyvsp[-2].zahl;
				nodeTEMP->np = 0;
				nodeTEMP->numRouters = 0;
				nodeTEMP->netList=0;
				nodeTEMP->numNets=0;
				nodeTEMP->executable=0;
				nodeTEMP->args=0;
		      
				if ( strlen(yyvsp[0].string_array[0]) != 0) {
					nodeTEMP->executable=newString(yyvsp[0].string_array[0]);
					metahostlist[metaHostDefH_id].procgroup=1;
				}
				if ( strlen(yyvsp[0].string_array[1]) != 0) {
					nodeTEMP->args=newString(yyvsp[0].string_array[1]);
				}
				
				if (yyvsp[-1].pointer_to_snetname /* netlist */) {
					if (!saveNetList(yyvsp[-1].pointer_to_snetname, nodeTEMP))
						CONF_ERROR(" ");
				}

				nodeTEMP->next = 0;
		      
				yyval.pointer_to_snode = nodeTEMP;
		;}
    break;

  case 71:
#line 766 "rconf_parser.y"
    {
				/* we have to save the niclist */
				Snode *nodeTEMP;
				SnicList *sniclistTEMP;

				sniclistTEMP = yyvsp[-3].pointer_to_sniclist;
				nodeTEMP = rh_newNodeClear();;
				nodeTEMP->nodeName= newString (yyvsp[-6].string);
				nodeTEMP->nicList=sniclistTEMP->nicList;
				nodeTEMP->maxNumProcs=yyvsp[-5].zahl;
		      
				nodeTEMP->netList=0;
				nodeTEMP->numNets=0;
				nodeTEMP->executable=0;
				nodeTEMP->args=0;   

				if ( strlen(yyvsp[0].string_array[0]) != 0) {
					nodeTEMP->executable=newString(yyvsp[0].string_array[0]);
					metahostlist[metaHostDefH_id].procgroup=1;			
				}
				if ( strlen(yyvsp[0].string_array[1]) != 0) {
					nodeTEMP->args=newString(yyvsp[0].string_array[1]);
				}
				

				if (yyvsp[-1].pointer_to_snetname /* netlist */) {
					if (!saveNetList(yyvsp[-1].pointer_to_snetname, nodeTEMP))
						CONF_ERROR(" ");			
				}

				nodeTEMP->np = 0;
				nodeTEMP->numRouters = 0;

				nodeTEMP->next = 0;
		      
				yyval.pointer_to_snode = nodeTEMP;
		;}
    break;

  case 72:
#line 803 "rconf_parser.y"
    { /* range of sequential numbered names */
				int start, end;
				Snode * nodeTEMP;

				nodeTEMP = createNodeListRange(yyvsp[-4].string,yyvsp[-2].string,&start,&end, yyvsp[-1].zahl, yyvsp[0].pointer_to_sniclist);

				if (!nodeTEMP) {CONF_ERROR("");};
					PDEBUG2("node-range start %d end %d\n", start, end);
				yyval.pointer_to_snode = nodeTEMP;
		;}
    break;

  case 73:
#line 814 "rconf_parser.y"
    { yyval.pointer_to_sniclist=0; ;}
    break;

  case 74:
#line 815 "rconf_parser.y"
    { 
		  if (!yyvsp[-1].pointer_to_sniclist) {
		    CONF_ERROR(" empty list of networks in node range");
		  }
		  yyval.pointer_to_sniclist=yyvsp[-1].pointer_to_sniclist; 
                  ;}
    break;

  case 75:
#line 823 "rconf_parser.y"
    { yyval.pointer_to_snetname=0; ;}
    break;

  case 76:
#line 824 "rconf_parser.y"
    { 
		  if (!yyvsp[-1].pointer_to_snetname) {
		    CONF_ERROR(" empty list of networks ");
		  }
		  yyval.pointer_to_snetname=yyvsp[-1].pointer_to_snetname; 
                  ;}
    break;

  case 77:
#line 831 "rconf_parser.y"
    {
                      SnetName *nname = malloc (sizeof(SnetName));
                      nname->name=newString(yyvsp[0].string);
                      nname->next=0;
                      yyval.pointer_to_snetname=nname;
                ;}
    break;

  case 78:
#line 837 "rconf_parser.y"
    {
                      SnetName *nname = malloc (sizeof(SnetName));
                      nname->name=newString(yyvsp[0].string);
                      nname->next=0;
                      (yyvsp[-2].pointer_to_snetname)->next=nname;
		      yyval.pointer_to_snetname=yyvsp[-2].pointer_to_snetname;
                ;}
    break;

  case 79:
#line 845 "rconf_parser.y"
    { yyval.zahl=0; ;}
    break;

  case 80:
#line 846 "rconf_parser.y"
    { yyval.zahl=yyvsp[0].zahl; ;}
    break;

  case 81:
#line 849 "rconf_parser.y"
    {
		      SnicList *sniclistTEMP;

		      sniclistTEMP = (SnicList *)malloc( sizeof(SnicList) );
		      sniclistTEMP->next = 0;
		      sniclistTEMP->nicList = yyvsp[0].pointer_to_snic;
		      
		      yyval.pointer_to_sniclist = sniclistTEMP;
                  ;}
    break;

  case 82:
#line 860 "rconf_parser.y"
    {
                      yyval.pointer_to_snic = yyvsp[0].pointer_to_snic;
                  ;}
    break;

  case 83:
#line 863 "rconf_parser.y"
    {
   		      Snic *nicTEMP;

		      nicTEMP = yyvsp[0].pointer_to_snic;
		      nicTEMP->next = yyvsp[-2].pointer_to_snic;
		      
		      yyval.pointer_to_snic = nicTEMP;
		  ;}
    break;

  case 84:
#line 873 "rconf_parser.y"
    {  
                      yyval.pointer_to_snic = rh_newNic( yyvsp[0].string, 0, ADR_TCP );
		  ;}
    break;

  case 85:
#line 876 "rconf_parser.y"
    { 
		      yyval.pointer_to_snic = rh_newNic( yyvsp[-2].string, yyvsp[0].zahl, ADR_TCP ); 
		  ;}
    break;

  case 86:
#line 879 "rconf_parser.y"
    { 
		      yyval.pointer_to_snic = rh_newNic( yyvsp[0].string, 0, ADR_ATM_SVC );
		  ;}
    break;

  case 87:
#line 882 "rconf_parser.y"
    { 
 		      SnicDef *nicDefTEMP;

		      nicDefTEMP = lookupNicDef( yyvsp[0].string );
		      if( !nicDefTEMP ) {
			  RERROR1("NIC %s not defined!\n", yyvsp[0].string);
			  CONF_ERROR("")
		      }
         	      PDEBUG1("name is here:%s\n", nicDefTEMP->nicName);

                      yyval.pointer_to_snic = rh_newNic( nicDefTEMP->nic->nicAddress, nicDefTEMP->nic->port, nicDefTEMP->nic->nicType);
		  ;}
    break;

  case 88:
#line 894 "rconf_parser.y"
    {
 		      SnicDef *nicDefTEMP;

		      nicDefTEMP = lookupNicDef( yyvsp[-2].string );
		      if( !nicDefTEMP ) {
			  RERROR1("NIC %s not defined!\n", yyvsp[-2].string);
			  CONF_ERROR("")
		      }
		      PDEBUG2("name is here:%s, port is %d\n", nicDefTEMP->nicName, yyvsp[0].zahl);

                      yyval.pointer_to_snic = rh_newNic( nicDefTEMP->nic->nicAddress, yyvsp[0].zahl, nicDefTEMP->nic->nicType);
                  ;}
    break;

  case 89:
#line 908 "rconf_parser.y"
    { if (!checkConnSections())  CONF_ERROR("");;}
    break;

  case 90:
#line 909 "rconf_parser.y"
    { /* automatic configuration of router processes */		  
		  if (!auto_router(AR_NO_OPT)) { 
		    CONF_ERROR("automatic router configuration failed ");
		  }
                 ;}
    break;

  case 91:
#line 914 "rconf_parser.y"
    { /* automatic configuration of router processes with options */		  
		  if (!auto_router(yyvsp[-1].autoOption)) { 
		    CONF_ERROR("automatic router configuration failed ");
		  }
                 ;}
    break;

  case 92:
#line 921 "rconf_parser.y"
    { yyval.autoOption= AR_NO_DNS; ;}
    break;

  case 93:
#line 922 "rconf_parser.y"
    { yyval.autoOption= AR_ONLY_DNS; ;}
    break;

  case 94:
#line 924 "rconf_parser.y"
    {;}
    break;

  case 95:
#line 925 "rconf_parser.y"
    {;}
    break;

  case 96:
#line 928 "rconf_parser.y"
    { 
			routerExec=0;
                      if (routerCounter < nrRouters)   CONF_ERROR("connection missing ");
		      if (routerCounter > nrRouters)   CONF_ERROR("too many connections  ");
                  ;}
    break;

  case 97:
#line 933 "rconf_parser.y"
    { 
		    if (Bparse_conn )   CONF_ERROR("connections missing for declared routers. Router number must be 0 for multidevice ");
                  ;}
    break;

  case 98:
#line 938 "rconf_parser.y"
    {
		      if (strlen(yyvsp[-4].string) > HOSTNAMELEN-1 && strlen(yyvsp[-3].string) > HOSTNAMELEN-1) {
			  RERROR1("Error in Metaconfig-File line %d: hostname too long!", ZCounter );
			  CONF_ERROR("")
		      }
		      strcpy(hostA,yyvsp[-4].string); strcpy(hostB,yyvsp[-3].string); nrRouters=yyvsp[-2].zahl;
		      PDEBUG2("%s -> %s:",hostA, hostB); PDEBUG1("%d Router\n", nrRouters);

		      /* check hosts */
		      if (rh_getMetahostId(hostA)<0 ) {
			  RERROR1("undeclared host %s ", hostA);
			  CONF_ERROR("")
		      }
		      if (rh_getMetahostId(hostB) <0 ) {
			  RERROR1("undeclared host %s ", hostB);
			  CONF_ERROR("")
		      }

		      routerCounter=0;
		      if (yyvsp[-2].zahl == 0) {
			/* _no_ routers - multidevice */ 
			Bparse_conn = 0;
			pars_rconf->useRouterFromTo[rh_getMetahostId(hostA)][rh_getMetahostId(hostB)] = 0;
		      }
		      else {
			  Bparse_conn = 1;
			  pars_rconf->useRouterFromTo[rh_getMetahostId(hostA)][rh_getMetahostId(hostB)] = 1;
		      }
		      if (yyvsp[-1].pointer_to_char) {
		      		routerExec=yyvsp[-1].pointer_to_char;
		      } else {
		      		routerExec = metahostlist[rh_getMetahostId(hostA)].routerExec;
		      }
		      /* if we have a dedicated router, we need a procgroup file for this mh */
		      if (routerExec)	
		      	   metahostlist[rh_getMetahostId(hostA)].procgroup=1;
		      /* is it me? */
		      if ( strcmp(hostA, pars_rconf->my_metahostname)==0){
			  pars_rconf->my_nrp[rh_getMetahostId(hostB)] = yyvsp[-2].zahl;
		      }
		      pars_rconf->nrp_metahost[rh_getMetahostId(hostA)] += yyvsp[-2].zahl;
		      pars_rconf->nrp += yyvsp[-2].zahl;
		      checkSubSection();   
                  ;}
    break;

  case 99:
#line 983 "rconf_parser.y"
    {yyval.pointer_to_char=0;}
    break;

  case 100:
#line 984 "rconf_parser.y"
    { yyval.pointer_to_char=newString(yyvsp[0].string); ;}
    break;

  case 103:
#line 990 "rconf_parser.y"
    {
                      if (!saveRouter( yyvsp[-1].pointer_to_sniclist , lastConnDefType, routerExec)) {CONF_ERROR("")}		      
		      routerCounter++;
		      if (lastConnDefType==ConnDefType_BI) {
			/* we skip the backward definition section, so we correct this */
			sectionList[rh_getMetahostId(hostB)*pars_rconf->nbr_metahosts+rh_getMetahostId(hostA)]=1;
			pars_rconf->nrp_metahost[rh_getMetahostId(hostB)]++;
			pars_rconf->nrp++;
		      }
                   ;}
    break;

  case 104:
#line 1002 "rconf_parser.y"
    {
                      int i = 0;
		      SnicList *sniclistTEMP, *tailStack;
		      Snic *pnic;
    		      lastConnDefType=yyvsp[-1].conn_def_type;
		      sniclistTEMP = yyvsp[-2].pointer_to_sniclist;
		      sniclistTEMP->next = yyvsp[0].pointer_to_sniclist;
		      
		      tailStack = sniclistTEMP;

		      while (tailStack){
			  i++;
			  PDEBUG1("%d. niclist:",i); 
			  pnic=tailStack->nicList;
			  while(pnic) {
			      PDEBUG1(":%s:",pnic->nicAddress);
			      pnic=pnic->next;
			  }
			  PDEBUG("\n");
			  tailStack=tailStack->next;
		      }
		      PDEBUG1("%d niclists found!\n",i);

		      yyval.pointer_to_sniclist = sniclistTEMP;
                  ;}
    break;

  case 105:
#line 1030 "rconf_parser.y"
    {  yyval.conn_def_type=ConnDefType_UNI; ;}
    break;

  case 106:
#line 1031 "rconf_parser.y"
    {   yyval.conn_def_type=ConnDefType_UNI; ;}
    break;

  case 107:
#line 1032 "rconf_parser.y"
    {   yyval.conn_def_type=ConnDefType_BI; ;}
    break;

  case 108:
#line 1035 "rconf_parser.y"
    { 
                      yyval.pointer_to_sniclist = yyvsp[0].pointer_to_sniclist;
                  ;}
    break;

  case 109:
#line 1038 "rconf_parser.y"
    {
                      SnicList *sniclistTEMP;

		      sniclistTEMP = yyvsp[0].pointer_to_sniclist;
		      sniclistTEMP->next = yyvsp[-1].pointer_to_sniclist;

		      yyval.pointer_to_sniclist = sniclistTEMP;
		  ;}
    break;


    }

/* Line 1010 of yacc.c.  */
#line 2562 "rconf_parser.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {
		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
		 yydestruct (yystos[*yyssp], yyvsp);
	       }
        }
      else
	{
	  YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
	  yydestruct (yytoken, &yylval);
	  yychar = YYEMPTY;

	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

  yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 1052 "rconf_parser.y"

/* looks for a nic definition with name in nicDefList;
   if found, returns a pointer to it, otherwise 0;
   in case of multiple definitions with the same name, pointer to
   last of them is returned */
SnicDef *lookupNicDef( char *name )
{
    SnicDef *ndSearch, *ndFound = 0;

    ndSearch = nicDefList;
    while( ndSearch ) {
	if( strcmp( ndSearch->nicName, name ) == 0 )
	    ndFound = ndSearch;
	ndSearch = ndSearch->next;
    }

    return ndFound;
}

int addRouter(ConnDefType e_connDefType, char* routerExec, int mh_idA, int mh_idB, Snic * nicsA, Snic * nicsB) {
	    Snode *foundNode;
	    char* nodeName=0;
	    int r_id;
	    int backDirection=0;	    	    
	    int repeats;

	    repeats=(e_connDefType==ConnDefType_BI)?2:1;

	    while ( backDirection < repeats) {
	     if (backDirection == 1) {
	     	int mh_id_swp;
		Snic * nics_swp;
	     	/* make a connection back, swap A and B */
		mh_id_swp=mh_idA;
		mh_idA=mh_idB;
		mh_idB=mh_id_swp;
		nics_swp=nicsA;
		nicsA=nicsB;
		nicsB=nics_swp;
	     }
	     backDirection++;
	    /* search the metahost node this router is running on */
	    foundNode=rh_findNicInHosts(metahostlist[mh_idA].nodeList, nicsA);
	    if (foundNode) 
		nodeName=newString(foundNode->nodeName);
	    else  { /* this might be an error */
	        if ( 1==1) { /* FIXME - this shoud assure there is only on node in this meta host */
		    /* one node only - assume nics are correct */
		    foundNode=metahostlist[mh_idA].nodeList;
		    nodeName=newString(metahostlist[mh_idA].nodeList->nodeName);
		} else {
		    RERROR2("saveRouter: network interface %s address not found for non-SMP metahost %s\n",
			    nicsA->nicAddress,mh_idA);
		    return 0;
		}
	    }
	    if (  pars_rconf->my_metahost_rank == mh_idA ) { /* this is my meta host - save router !*/
	    	if ((r_id=rh_addRouter(mh_idB, metahostlist[mh_idA].num_of_routers, nicsA, nicsB, 	pars_rconf->tcp_portbase))
			   == -1) {
			RERROR("saveRouter: error adding router\n");
			return 0;
	    	}	    	    
	    	routerlist[r_id].nodeName=nodeName;
		if (backDirection == 1) {
			routerlist[r_id].routerExec=routerExec;
		}
		if (backDirection == 2) {
			routerlist[r_id].routerExec=metahostlist[mh_idA].routerExec;
		}
	    }
	    if (foundNode->numRouters==0) {
	    	foundNode->routerIds= (int*) malloc( sizeof(int));
		foundNode->routerIds[0]=-1;
	    } else {		
		foundNode->routerIds= (int*) realloc( foundNode->routerIds, sizeof(int)
							 *(foundNode->numRouters+1)); 
		foundNode->routerIds[foundNode->numRouters+1]=-1;	    
  	    }
	    foundNode->routerIds[foundNode->numRouters]=metahostlist[mh_idA].num_of_routers;
	    foundNode->numRouters++;
	    foundNode->npFixed++;
	    metahostlist[mh_idA].num_of_routers++;
	} /* while */
}
/* the most important action of the parser: save the router data */
int saveRouter( SnicList *nicStack, ConnDefType e_connDefType , char* routerExec){
    SnicList *tailStack;
    SnicList *nStack;
    int mh_idB, mh_idA,r_id;
    Snic *localNics, *remoteNics;   
    
    if (routerCounter>nrRouters) {
	RERROR("too many router declarations ");
	return 0;
    }

    /* first niclist is the routers nic - save it*/
    localNics=nicStack->nicList;

    PDEBUG1("Host=%s ",hostA);
    PDEBUG2("num_of_routers=%d hostA=%s\n", metahostlist[rh_getMetahostId(hostA)].num_of_routers,
	    hostA);
 
    /* we have to save the data for ALL meta-local routers, because we may not know our local router rank,
     * e.g. the ch_shmem device forks the processes on the metahost later in MPID_Init
     */
     mh_idA = rh_getMetahostId(hostA);
	mh_idB = rh_getMetahostId(hostB);
	
	/* build list of routers i have to connect to */
	tailStack=nicStack->next;
	while(tailStack){
	    Snode *foundNode;
	    char* nodeName=0;
	
	    remoteNics=tailStack->nicList;
	    addRouter(e_connDefType, routerExec,  mh_idA, mh_idB, localNics, remoteNics);

	    tailStack = tailStack->next;
	}
    
    
    /* clear niclists */
    tailStack=nicStack;

    while (tailStack) {
	nStack=tailStack->next;
	free(tailStack);
	tailStack=nStack;
    }

    return 1;
}


int checkConnSections(){    
    int i,j;
    for(i=0; i< pars_rconf->nbr_metahosts;i++)    
	for(j=0; j< pars_rconf->nbr_metahosts;j++)
	    if (i!=j)
		if (!sectionList[i*pars_rconf->nbr_metahosts+j]) {
		    RERROR2("Subsection missing: %s -> %s\n",pars_rconf->metahostnames[i], 
			   pars_rconf->metahostnames[j]);
		    return 0;
		    
		}		   
    return 1;
}


void checkHeaderSection(){
    int i;
    
    PDEBUG("** initialisiere sectionList\n");
    /* prepare list of host-to-host pairs */
    numSections= pars_rconf->nbr_metahosts*(pars_rconf->nbr_metahosts-1);
    sectionList= (int*) malloc(sizeof(int)* pars_rconf->nbr_metahosts*pars_rconf->nbr_metahosts);
    for (i=0; i< pars_rconf->nbr_metahosts*pars_rconf->nbr_metahosts; i++){
	sectionList[i]=0;
    }
    
    pars_rconf->nbr_metahosts=countHosts;

}


int checkSubSection(){
    int i;
    int id1, id2;

    #ifdef bla
    if (countSubsections == numSections) { 
    	RERROR("too many Subsections!\n"); return 0;
    }
#endif
    /* check host-to-host pair in list */
    id1=rh_getMetahostId(hostA); id2=rh_getMetahostId(hostB);

    PDEBUG1("this section id is %d\n",id1* (pars_rconf->nbr_metahosts) + id2);
    PDEBUG("Sectionlist:"); for(i=0; i<numSections; i++)  PDEBUG1(" %d",sectionList[i]); PDEBUG("\n");
    PDEBUG2("id1: %d id2: %d\n",id1,id2);

    if (sectionList[ id1 * (pars_rconf->nbr_metahosts) + id2 ]) {
	RERROR2("section %s->%s already exists! ",hostA,hostB); 
	return 0;
    }
    sectionList[ id1 * (pars_rconf->nbr_metahosts) + id2] = 1;
    countSubsections++;
    
    return 1;
}


/* sortInNodeList sorts a node list alphabetically into another list of nodes
 * this is important for ch_smi because the process ranks are distributed in
 * this order
 * if a node is already in the list, its properties are updated.
 * inSortNodeList is consumed, i.e. it cannot be used anymore after a call to
 * this function.
 * the new root of the list is returned
 */
Snode *  sortInNodeList ( Snode *nodeListHead, Snode *inSortNodeList ){
    Snode *inSortNode = inSortNodeList;

    while (inSortNode) {
	Snode *pn, *nextInSortNode; 
	
	pn = nodeListHead;
	
	/* we have to save the next node because the next-field will be overwritten */
	nextInSortNode=inSortNode->next;

	if (!pn) {
	    /* first node */
	  if ( !excludeNode )
	    nodeListHead = inSortNode;
	}
	else {
	    /* sort in list */
	    Snode * oldpn = 0;
	    Snode *oldoldpn = 0;

	    while(pn && strcmp(inSortNode->nodeName, pn->nodeName) >= 0) {
	      oldoldpn=oldpn;
	      oldpn=pn;
	      pn=pn->next;
	    }
	    
	    if (!oldpn) {
	      /* inSortNode gets first node */
	      if ( !excludeNode ) {
		nodeListHead=inSortNode;
		inSortNode->next = pn;
	      }
	    }
	    else {
	      if (strcmp(inSortNode->nodeName, oldpn->nodeName)==0) { /* update or exclude */
		if ( excludeNode ) {
		  /* remove node oldpn from list */
		  /*		  RERROR1("removing node %s\n",oldpn->nodeName);*/

		  if (oldoldpn != 0)
		    oldoldpn->next=oldpn->next;
		  else 
		    nodeListHead=oldpn->next;

		  free(oldpn->nodeName);
		  free(oldpn);
		} else {

		  if ( inSortNode->maxNumProcs > 0) 
		    oldpn->maxNumProcs = inSortNode->maxNumProcs;
		  if ( inSortNode->nicList ) 
		    oldpn->nicList = inSortNode->nicList;
    		  if ( inSortNode->executable ) 
		    oldpn->executable = inSortNode->executable;

		}
	      }
	      else {
		inSortNode->next=pn;
		oldpn->next=inSortNode;
	      }
	    }
	}
	/* process next item in the inSortNodeList */
	inSortNode=nextInSortNode;
    } /* while(inSortNode) */
    return nodeListHead;
}


/* checks if a and b have the same form <prefix><number> and returns
   the start and end number if true
   the return value is a pointer to a node list, or 0 if an error occurs 
*/
Snode * createNodeListRange(char* a,char *b,int *start,int *end, int numprocs, SnicList* nicrange){

  int len, prefixLen,digitLen,suffixPos,digitPos;
    char *prefix,*suffix;
    Snode * rangeNodeList;
    unsigned int it_nicRange[4];

    RDEBUG("in createNodeListRange:\n");
    RDEBUG2("%s .... %s\n",a,b);
    
    /* check the syntax of the range */
    if( strlen(a) != strlen(b)) {
	RDEBUG("node range boundaries differ in length\n");
	return 0;
    }    
    
    /* allocate strings */
    prefix=(char*)malloc(sizeof(char)*strlen(a));
    suffix=(char*)malloc(sizeof(char)*strlen(a));

    /* look for range number delimiter | in a and compare with b */
    prefixLen=0;
    while ( prefixLen < strlen(a) && a[prefixLen] != '|' && a[prefixLen] == b[prefixLen]){
      prefix[prefixLen]=a[prefixLen];
      prefixLen++;
    }
    /* what happend? */
    if (a[prefixLen] != b[prefixLen]) {
      RERROR2("meta configuration error: differing prefixes in node range %s - %s\n",a,b);
      return 0;
    }
    if (prefixLen == strlen(a)) { 
      RERROR2("meta configuration error: no range digits delimiters |dd| found in node range %s - %s\n",a,b);
      return 0;
    }
    if (prefixLen == 0) {
      RERROR2("meta configuration error: no prefix found in node range %s - %s\n",a,b);
      return 0;
    }
    prefix[prefixLen]=0; /* close prefix string */
    prefixLen--;

    /* look for second range number delimiter | in a and check digits from a and b */
    suffixPos=prefixLen+2;
    digitPos=suffixPos;
    digitLen=0;
    if (suffixPos == strlen(a)) {
      RERROR2("meta configuration error: only 1 range digits delimiters |dd| found in node range %s - %s\n",a,b);
      return 0;
    }
    while ( suffixPos < strlen(a) && a[suffixPos] != '|' 
	    &&  isdigit(a[suffixPos]) && isdigit(b[suffixPos]) ) {
      suffixPos++;
      digitLen++;
    }

    /* what happend? */
    if (suffixPos == strlen(a)) { 
      RERROR2("meta configuration error: no second range digits delimiters |dd| found in node range %s - %s\n",a,b);
      return 0;
    }
    if ( a[suffixPos] == '|') {
      if (b[suffixPos] != '|' ) {
	RERROR2("meta configuration error: b must have range digits delimiters in the same position as a (a=%s - b=%s)\n",a,b);
	return 0;
      }
      /* ok, proceed! */
    } else {
      if (!isdigit(a[suffixPos])){
	RERROR1("meta configuration error: non-digit %c found in lower range digits:  %s\n",a);
	return 0;
      }
      if (!isdigit(b[suffixPos])){
	RERROR1("meta configuration error: non-digit %c found in upper range digits:  %s\n",b);
	return 0;
      }
      if ( b[suffixPos] != '|') {
	RERROR2("meta configuration error: b must have range digits delimiters in the same position as a (a=%s - b=%s)\n",a,b);
	return 0;
      }
    }
    if (digitLen == 0){
      RERROR2("meta configuration error: no range digits found in node range %s - %s\n",a,b);
      return 0;
    }
    suffixPos++;
    strcpy(suffix,"");
    {
      int pos=0;
      while (suffixPos + pos <= strlen(a)) {
	suffix[pos]=a[suffixPos+pos];
	pos++;
      }
    }
	
    /* get start and end */
    *start = atoi( a+ digitPos);
    *end = atoi( b+ digitPos);
   
    /* check if a nicrange is provided */
    if  (nicrange) {
      Snic *nicStart=nicrange->nicList;
      struct in_addr intmp;
      if( (intmp.s_addr = inet_addr( nicStart->nicAddress )) == (in_addr_t)-1 ) {
	RERROR1(" wrong ip %s ",nicStart->nicAddress);
	return 0;
      }
      it_nicRange[0]=(unsigned int) ((unsigned char*)&intmp)[0];
      it_nicRange[1]=(unsigned int) ((unsigned char*)&intmp)[1];
      it_nicRange[2]=(unsigned int) ((unsigned char*)&intmp)[2];
      it_nicRange[3]=(unsigned int) ((unsigned char*)&intmp)[3];
    }


    /* now build a nodeList */
    {
      int nr;
      char *c_digits;
      char *leadingZeros;
      char * tempname;
      Snode *oldpnode=0;    

      c_digits = malloc(sizeof(char) * (digitLen+1));
      leadingZeros = malloc(sizeof(char) * (digitLen+1));
      tempname=newString(a);

	for ( nr=*start; nr <= *end; nr++ ) {
	    Snode * pnode;
	    int i, dec, count=0;

	    sprintf(c_digits,"%d",nr);
	    strcpy(leadingZeros,"");

	    for (i=0; i < digitLen-strlen(c_digits); i++)
	      strcat(leadingZeros,"0");

	    strcpy(tempname,prefix);
	    strcat(tempname,leadingZeros);
	    strcat(tempname,c_digits);
	    strcat(tempname,suffix);
	    		
	    pnode=rh_newNodeClear();

	    /* link new element */
	    if (oldpnode)
		oldpnode->next = pnode;
	    else  /* save root of this list */
		rangeNodeList = pnode;
	  
	    pnode->next=0;
	    pnode->numNets=0;
	    pnode->netList=0;
	    /*	    if (netnames) saveNetList(netnames, pnode);*/
	    pnode->nicList=0;
	    pnode->numRouters=0;
	    pnode->routerIds=0;
	    pnode->maxNumProcs=numprocs;
	    pnode->nodeName= newString(tempname);
	    PDEBUG1("adding node %s\n",tempname);	    
	    if (nicrange) {
	      char  anic[200];
	      
	      sprintf(anic,"%d.%d.%d.%d",it_nicRange[0],it_nicRange[1],it_nicRange[2],it_nicRange[3]);
	      pnode->nicList= rh_newNic(anic,nicrange->nicList->port , ADR_TCP);
	      PDEBUG1("with address %s\n",anic);
	      it_nicRange[3]++;
	      if  (it_nicRange[3] == 256) {
		it_nicRange[3]=0;
		it_nicRange[2]++;
	      }
	    }
	    oldpnode=pnode;
	}
	free(c_digits);
	free(leadingZeros);
	free(tempname);
    } 
    
    free(prefix);
    free(suffix);

    return rangeNodeList;
}

int getNetId(char* name) {
  SnetDefList* nl = pars_rconf->netDefList;
  int i=0;
  while (nl) {
    if (strcmp(nl->name, name) == 0) 
      return i;
    nl=nl->next;
    i++;
  }
  return -1;
}

int saveNetList(SnetName * netnames, Snode *  node) {
  int i;
  SnetName * nn = netnames;
  while (nn) {
    node->numNets++;
    nn=nn->next;			  
  }
  node->netList= malloc (node->numNets*sizeof(int));

  nn=netnames;
  i=0;
  while (nn) {
    node->netList[i]=getNetId(nn->name);
    if (node->netList[i] == -1)
      RERROR1("undefined network %s ",nn->name);
    nn=nn->next;
    i++;
  }
}

/* reset globals for multiple parsing */
void resetGlobals() {
    countSubsections = 0;
    countHosts = 0;
    Bparse_conn=0;

    nrRouters=0;   

    numSections=0;

    nicDefList = 0;

    metaHostFound=0;
    routerCounter=0;   
    SCounter=1;
    ZCounter=1;

}

void yyerror(const char* s)
{
   fprintf(stderr, "error in meta config file, line %d near column %d %s ******\n",
	   ZCounter,SCounter,s);
}



