## Process this file with automake to produce Makefile.in
## $Id$

if FULL_SMI
exampleexecdir = ${pkglibdir}/examples/poisson

exampleexec_PROGRAMS = poisson
AM_CPPFLAGS = -I$(top_srcdir)/include
AM_LDFLAGS = $(EXAMPLES_LIBS)
LDADD = ../../lib/lib$(LIBNAME).la

poisson_SOURCES = poisson.c
endif
