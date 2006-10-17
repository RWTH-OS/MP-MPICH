## Process this file with automake to produce Makefile.in
## $Id_SOURCES = Makefile.am 3002 2004-12-20 17:26:47Z tobias $

# Needed? Portable?
# CXXFLAGS= -finline-functions -fno-rtti

check_PROGRAMS = test_environment test_architecture test_file_mapping \
	test_file_mapping_2 test_file_mapping_3 test_pagesize test_sockets \
	event_test_1 event_test_2 test_threads test_threads_2 test_threads_3

AM_CXXFLAGS = -I$(top_srcdir)/include
LDADD = ../lib/libnt2unix.la

event_test_1_SOURCES = event_test_1.cc
event_test_2_SOURCES = event_test_2.cc
#event_test_3_SOURCES = event_test_3.cc
test_architecture_SOURCES = test_architecture.cpp
test_file_mapping_SOURCES = test_file_mapping.cpp
test_file_mapping_2_SOURCES = test_file_mapping_2.cpp
test_file_mapping_3_SOURCES = test_file_mapping_3.cpp
test_pagesize_SOURCES = test_pagesize.cpp
test_sockets_SOURCES = test_sockets.cpp insocket.cc
test_environment_SOURCES = test_environment.cpp
test_threads_SOURCES = test_threads.cpp
test_threads_2_SOURCES = test_threads_2.cpp
test_threads_3_SOURCES = test_threads_3.cpp
