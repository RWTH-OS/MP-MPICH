## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.1.2.1 2004/12/20 17:26:36 tobias Exp $

if FULL_SMI
exampleexecdir = ${pkglibdir}/examples/poisson

exampleexec_PROGRAMS = poisson
AM_CPPFLAGS = -I$(top_srcdir)/include
AM_LDFLAGS = $(EXAMPLES_LIBS)
LDADD = ../../lib/lib$(LIBNAME).la

poisson_SOURCES = poisson.c
endif
