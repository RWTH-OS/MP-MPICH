/* $Id$ 
 * (c) 2005 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de 
 */

/* timer for profiling */

void start_time(double *t);

void stop_time(double *t);
void show_global_time(char * who);
extern int myTimer;
