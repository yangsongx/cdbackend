# For image processing study
DEBUG=true

CC = gcc

SRC = peer_detect.c \
      scan_peer.c \
      list.c \
      setup_server.c

# Release should use -O2

ifeq ($(DEBUG),false)
CFLAGS += -DDEBUG -O2
else
CFLAGS += -DCHECK_MEM_LEAK -DDEBUG -g -O0
endif

LDFLAGS = -lxml2 -lrt -pthread


OBJS = $(SRC:.c=.o)

all:pd

pd:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o pd
	find . -name '*.o' -exec rm -rf {} ";"
