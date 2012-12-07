# Makefile for C/C++

include ../Makefile.common
include depends.inc

EXECS = ep3

CGMROOT = /mnt/data/cgmlib
CGMLIBDIR = $(CGMROOT)/lib

.cpp.o:
	$(CCC) $(CFLAGS) -I$(CGMROOT)/include -c $<

all: $(EXECS)

ep3: main.o PairCommObject.o utils.o
	$(CCLINKER) -o $@ main.o PairCommObject.o utils.o -L$(CGMLIBDIR) -lcgm $(MPILDFLAGS)

