
# Makefile for NewUser Service (nus)
#
DEBUG=true

CC = g++
SRC = test.cc


#include ../../common/caredear_service_lib/common.mk
CXXFLAGS = -std=c++0x -g -O0

# Release should use -O2


LDFLAGS = -lmemcached


OBJS = $(SRC:.cc=.o)

all:a.out

a.out:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.cc.o:
	$(CC) $(CXXFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o a.out
