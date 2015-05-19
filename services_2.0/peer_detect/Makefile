# For image processing study
DEBUG=true

CC = gcc

SRC = peer_detect.c \
      scan_peer.c \
      list.c \
      setup_server.c

#include ../../common/caredear_service_lib/common.mk
CXXFLAGS = -I/usr/include/libxml2/ -fpermissive -std=c++0x -W -Wall -Wno-unused-parameter

# Release should use -O2

ifeq ($(DEBUG),false)
CFLAGS += -DDEBUG -O2
else
CFLAGS += -DCHECK_MEM_LEAK -DDEBUG -g -O0
endif

LDFLAGS = -lxml2


OBJS = $(SRC:.c=.o)

all:peer_detect

peer_detect:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o peer_detect
	find . -name '*.o' -exec rm -rf {} ";"
