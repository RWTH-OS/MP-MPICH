## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.1.2.1 2004/12/20 17:26:36 tobias Exp $

exampleexecdir = ${pkglibdir}/examples/memcpy_bench

exampleexec_PROGRAMS = memcpy_bench
exampleexec_DATA = README

AM_CPPFLAGS = -I$(top_srcdir)/include
LDADD = ../../lib/lib$(LIBNAME).la

memcpy_bench_SOURCES = memcpy_bench.c
