/* $Id$ 
 * (c) 2005 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de 
 */

#ifndef AUTO_ROUTER_H
#define AUTO_ROUTER_H
    
/* Options for AUTO_ROUTER */
typedef enum _EautoAutoOption { AR_NO_OPT, AR_NO_DNS , AR_ONLY_DNS } EautoAutoOption;

/*
 * auto_router tries to configure the routers in a simple automatic way.
 * We need one NIC defined for each metahost or valid names for an dns lookup. 
 */

extern int auto_router(EautoAutoOption ao);

#endif
