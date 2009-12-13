## Process this file with automake to produce Makefile.in

noinst_LIBRARIES = libicc.a

libicc_a_SOURCES = icc.h iccV42.h icc.c iccstd.c

bin_PROGRAMS = icctest iccdump icclu

icctest_SOURCES = icctest.c
icctest_LDADD = libicc.a

iccdump_SOURCES = iccdump.c
iccdump_LDADD = libicc.a

icclu_SOURCES = icclu.c
icclu_LDADD = libicc.a

check_PROGRAMS = iccrw lutest

iccrw_SOURCES = iccrw.c
iccrw_LDADD = libicc.a

lutest_SOURCES = lutest.c
lutest_LDADD = libicc.a

EXTRA_DIST = License.txt Readme.txt
