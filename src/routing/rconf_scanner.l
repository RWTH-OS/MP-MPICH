%{ 
/* $Id$ 
 * (c) 2005 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de 
 */
#include "parser.h"
#include "rconf_parser.tab.h"

int Byte1;
int Byte2;
int Byte3;
int Byte4;

int ZCounter=1;
int SCounter=1;

void error(); 

%}

ZIFFER [0-9]
ZAHL   [0-9]+
universalname [a-z_A-Z\*][\|0-9a-z_A-Z\*\-\.]*
HEX    [0-9a-fA-F]
NUMHOSTS "NUMHOSTS"
PORTBASE "PORTBASE"
PORTRANGE "PORTRANGE"
AUTO_ROUTER "AUTO_ROUTER"
NO_DNS "NO_DNS"
ONLY_DNS "ONLY_DNS"
ROUTERTIMEOUT "ROUTERTIMEOUT"
HETERO "HETERO"
EXCHANGE_ORDER "EXCHANGE_ORDER"
SPLITSIZE "SPLITSIZE"
ISEND_NUM "ISEND_NUM"
CONNECTIONS "CONNECTIONS"
PAIR "PAIR"
OPTIONS "OPTIONS"
NICDEF "NICDEF"
METAHOST "METAHOST"
TCP "TCP"
ATM_SVC "ATM_SVC"
ATM_PVC "ATM_PVC"
GETIP "GETIP"
NODES "NODES"
ROUTERS "ROUTERS"
NET "NET"
SECONDARY_DEVICE "SECONDARY_DEVICE"
NETMASK "NETMASK"
EXTRAPROCS "EXTRAPROCS"
MAX "MAX"
NOWATCHDOG "NOWATCHDOG"
SMI_VERBOSE "SMI_VERBOSE"
SMI_DEBUG "SMI_DEBUG"
%%
=     { return (GLEICH); }
[/][*][^\n]*[*][/] 
[#].*[\n]             { yyless(yyleng-1);
                      ZCounter++;
/* return RETURN;*/
}
\'[^\n\']*\'		{ SCounter+=yyleng;
						strcpy(yylval.string, yytext+1);
						yylval.string[strlen(yylval.string)-1]=0;
						return(LITERAL); }                      
{OPTIONS}			{SCounter+=yyleng;
						return (OPTIONS);}     
{PAIR}				{SCounter+=yyleng;
						return (PAIR);}       
{NICDEF}			{SCounter+=yyleng;
						return (NICDEF);}       
{NODES}				{SCounter+=yyleng;
						return (NODES);}       
{ROUTERS}			{SCounter+=yyleng;
						return (ROUTERS);}       
{METAHOST}			{SCounter+=yyleng;
						return (METAHOST);}       
{TCP}				{SCounter+=yyleng;
						return (TCP);}       
{ATM_SVC}			{SCounter+=yyleng;
						return (ATM_SVC);}  
{ATM_PVC}			{SCounter+=yyleng;
						return (ATM_PVC);}  
{CONNECTIONS}		{SCounter+=yyleng;
						return (CONNECTIONS);}                       
{GETIP}				{SCounter+=yyleng;
						return (GETIP);}                       
{PORTBASE}			{SCounter+=yyleng;
						return(PORTBASE);}
{PORTRANGE}			{SCounter+=yyleng;
						return(PORTRANGE);}
{ROUTERTIMEOUT}		{SCounter+=yyleng;
						return(ROUTERTIMEOUT);}
{AUTO_ROUTER}		{SCounter+=yyleng;
						return(AUTO_ROUTER);}
{NO_DNS}			{SCounter+=yyleng;
						return(NO_DNS);}
{ONLY_DNS}			{SCounter+=yyleng;
						return(ONLY_DNS);}
{HETERO}			{SCounter+=yyleng;
						return(HETERO);}
{EXCHANGE_ORDER}	{SCounter+=yyleng;
						return(EXCHANGE_ORDER);}
{SPLITSIZE}			{SCounter+=yyleng;
						return(SPLITSIZE);}
{ISEND_NUM}			{SCounter+=yyleng;
						return(ISEND_NUM);}
{NUMHOSTS}			{SCounter+=yyleng;
						return(NUMHOSTS);}
{NET}				{SCounter+=yyleng;
						return(NET);}
{SECONDARY_DEVICE}  {SCounter+=yyleng;
						return(SECONDARY_DEVICE);}
{NETMASK}           {SCounter+=yyleng;
						return(NETMASK);}
{NOWATCHDOG}		{SCounter+=yyleng;
						return(NOWATCHDOG);}
{SMI_VERBOSE}		{SCounter+=yyleng;
						return(SMI_VERBOSE);}
{SMI_DEBUG}			{SCounter+=yyleng;
						return(SMI_DEBUG);}

({ZIFFER}){1,3}(\.({ZIFFER}){1,3}){3}\/({ZIFFER}){1,2} {sscanf(yytext,"%d.%d.%d.%d",&Byte1,&Byte2,&Byte3,&Byte4);
               SCounter+=yyleng; 
               if(((Byte1 > 255) || (Byte2 > 255)) || ((Byte3 > 255) || (Byte4 > 254)))
               {
                  printf("Fehler IP Zeile:%d\nSpalten:%d-%d\n",ZCounter,SCounter-yyleng,SCounter);
               }
               else 
               { 
                 strcpy(yylval.string, yytext);
                 return(IP_NETMASK);  
               } 

           } 

{EXTRAPROCS}  	 {SCounter+=yyleng;
                       return(EXTRAPROCS);}
{MAX}  	 {SCounter+=yyleng;
                       return(MAX);}
(\/[^" "\t\t\n,;:]*)+(\/)*                {SCounter+=yyleng;
                       strcpy(yylval.string, yytext);
                       return PATH;
}
{universalname}         {SCounter+=yyleng;
                       if (strlen (yytext) >= MAX_STRING) yyerror("identifier too long");
                       strcpy(yylval.string, yytext);
                       return(NAME);}
"<->"              {return(CONNTO_BIDIR);}   
"->"              {return(CONNTO);}   
({HEX}{2,2}(\.){0,1}){19,19}{HEX}{2,2} { SCounter+=yyleng;
/*printf("%s\n",yytext);*/
             strcpy(yylval.string, yytext);
             return (ATM);
}                   
({ZIFFER}){1,3}\.({ZIFFER}){1,3}\.({ZIFFER}){1,3} { strcpy(yylval.string, yytext);
 return(ATM_PVC_ADDRESS);
}
({ZIFFER}){1,3}(\.({ZIFFER}){1,3}){3} {sscanf(yytext,"%d.%d.%d.%d",&Byte1,&Byte2,&Byte3,&Byte4);
               SCounter+=yyleng; 
               if(((Byte1 > 255) || (Byte2 > 255)) || ((Byte3 > 255) || (Byte4 > 254)))
               {
                  printf("Fehler IP Zeile:%d\nSpalten:%d-%d\n",ZCounter,SCounter-yyleng,SCounter);
               }
               else 
               { 
                 strcpy(yylval.string, yytext);
                 return(IP);  
               } 

           } 
{ZAHL}                {SCounter+=yyleng;
                       yylval.zahl=atoi(yytext);
                       return(ZAHL);}                      
":"                   {SCounter+=yyleng;
                       return(DOPPELPUNKT);}
                      
"-"                   {SCounter+=yyleng;
                       return(MINUS);}
                
","                   {SCounter+=yyleng;
                       return(KOMMA);}
^[" "\t][" "\t]*\n    {SCounter+=yyleng; /* eat up line containing only whitspaces */
                       ZCounter++;
                       if (SCounter != 1) { 
                          SCounter=1; 
                       }
                      }
[" "\t]*                   {SCounter+=yyleng;}
\n                    {  ZCounter++;
                         if (SCounter != 1) { 
                              SCounter=1; 
                              return(RETURN);
                         }
                      }
"("                    {SCounter+=yyleng;
                       return( '(' );}
")"                    {SCounter+=yyleng;
                       return( ')' );}
.                      {SCounter+=yyleng;
return (yytext[0]); 
}
%%
yywrap()
{}
/*main()
{
   yyparse();
   error();
}
*/

