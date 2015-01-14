# Makefile for OpenSIPS account info
#
DEBUG=true

CC = g++
SRC = stunlist.cc rfc3489.cc

CXXFLAGS += -I /usr/include/libxml2/ -W -Wall -Wno-unused-parameter -Wno-write-strings `mysql_config --cflags`

# Release should use -O2

ifeq ($(DEBUG),false)
CXXFLAGS += -O2
else
CXXFLAGS += -DCHECK_MEM_LEAK -DDEBUG -g -O0
endif

LDFLAGS = -pthread -lxml2 `mysql_config --libs`
ifeq ($(STATICLIB),false)
LDFLAGS += -lm -ldl -lz -lrt -static
endif

OBJS = $(SRC:.cc=.o)

all:stunlist

stunlist:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.cc.o:
	$(CC) $(CXXFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o stunlist
