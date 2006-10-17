int a;

void sequence_init() { a=0; }
int sequence() {
	a++;
	if (a >127 ) a=0;
	return a;
}
