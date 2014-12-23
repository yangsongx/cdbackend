# Makefile for keep_alive ping program
#
DEBUG=true

CC = gcc
SRC = keep_alive.c

CFLAGS += -I /usr/include/libxml2/

# Release should use -O2

ifeq ($(DEBUG),false)
CFLAGS += -O2
else
CFLAGS += -DCHECK_MEM_LEAK -DDEBUG -g -O0
endif

LDFLAGS = -pthread -lxml2


OBJS = $(SRC:.c=.o)

all:keep_alive

keep_alive:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o keep_alive
