/* $Id: sequence.c,v 1.2 2002/09/30 08:35:37 martin Exp $
     * (c) 2002 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
     * author: Martin Poeppe email: mpoeppe@gmx.de
     */     

int a;

void sequence_init() { a=0; }
int sequence() {
	a++;
	if (a >127 ) a=0;
	return a;
}
