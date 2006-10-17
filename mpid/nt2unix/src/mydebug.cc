#include "mydebug.h"

#ifdef _DEBUG
int D_REC_DEPTH = -1;
char D_REC_CHAR = '-';
std::ostream* D_COUT = &std::cout;
int D_DO_DEBUG = TRUE;
#endif
