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




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



