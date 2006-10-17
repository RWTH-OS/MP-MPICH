# Makefile for ptmalloc, version 2
# by Wolfram Gloger 1996-1999, 2001, 2002

SYS_FLAGS  =
OPT_FLAGS  = -O1 
WARN_FLAGS = #-Wall -Wstrict-prototypes

# Flags for the compilation of malloc.c
M_FLAGS   = -DTHREAD_STATS=1 #-DMALLOC_DEBUG=1

# Flags for the threads package configuration.
# Valid discerning symbols are:
# USE_PTHREADS=1 for Posix threads
# USE_THR=1      for Solaris threads
# USE_SPROC=1    for SGI sprocs
THR_FLAGS = -DUSE_PTHREADS=1 -DUSE_TSD_DATA_HACK -D_REENTRANT
THR_LIBS  = -lpthread

MALLOC = gmpi_malloc.o

CFLAGS = $(SYS_FLAGS) $(OPT_FLAGS) $(WARN_FLAGS) $(THR_FLAGS)

gmpi_malloc.o: malloc.c malloc.h thread-m.h arena.c hooks.c
	$(CC) $(CFLAGS) $(M_FLAGS) -c malloc.c -o $@
	mv $@ ..


