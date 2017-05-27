ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -Wall
AM_CFLAGS	=
AM_CXXFLAGS =
AM_LDFLAGS	=

lib_LTLIBRARIES = libbitstream.la
libbitstream_la_SOURCES = ./src/bitstream.c

check_PROGRAMS	= \
				  test1 \
				  test2 \
				  test3

test1_SOURCES	= ./tests/test1.c
test1_LDADD		= libbitstream.la

test2_SOURCES	= ./tests/test2.c
test2_LDADD		= libbitstream.la

test3_SOURCES	= ./tests/test3.c
test3_LDADD		= libbitstream.la

TESTS = $(check_PROGRAMS)
