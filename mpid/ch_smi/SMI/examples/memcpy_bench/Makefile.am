## Process this file with automake to produce Makefile.in
## $Id$

exampleexecdir = ${pkglibdir}/examples/memcpy_bench

exampleexec_PROGRAMS = memcpy_bench
exampleexec_DATA = README

AM_CPPFLAGS = -I$(top_srcdir)/include
LDADD = ../../lib/lib$(LIBNAME).la

memcpy_bench_SOURCES = memcpy_bench.c
