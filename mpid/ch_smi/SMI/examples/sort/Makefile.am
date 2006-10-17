## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.1.2.1 2004/12/20 17:26:37 tobias Exp $

exampleexecdir = ${pkglibdir}/examples/sort

exampleexec_PROGRAMS = mergesort sort_rmtread sort_memcpy sort_undivided

AM_CPPFLAGS = -I$(top_srcdir)/include
LDADD = ../../lib/lib$(LIBNAME).la

mergesort_SOURCES = mergesort.c
sort_rmtread_SOURCES = sort_rmtread.c
sort_memcpy_SOURCES = sort_memcpy.c
sort_undivided_SOURCES = sort_undivided.c

