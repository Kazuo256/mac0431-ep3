# Makefile for C/C++

include ../Makefile.common
include depends.inc

EXECS = ep3

CGMROOT = /mnt/data/cgmlib
CGMLIBDIR = $(CGMROOT)/lib

.cpp.o:
	$(CCC) $(CFLAGS) -I$(CGMROOT)/include -c $< -g

all: $(EXECS)

ep3: main.o
	$(CCLINKER) -o $@ main.o -L$(CGMLIBDIR) -lcgm $(MPILDFLAGS)

