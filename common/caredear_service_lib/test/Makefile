# Makefile for build test client code
#
CC = gcc

SRC = client_demo.c

# Release should use -O2
CFLAGS = -W -Wall -Wno-old-style-declaration -Wno-implicit-function-declaration -Wno-unused-parameter -g -O0 -DDEBUG -I../ -I /usr/include/libxml2/
LDFLAGS=-pthread
OBJS = $(SRC:.c=.o)

DEMO_SRC = server_demo.c
DEMO_LDFLAGS = -L ../ -lcds -pthread -levent -lcrypto -lxml2
DEMO_OBJS = $(DEMO_SRC:.c=.o)

KEEP_ALIVE_SRC = keep_alive.c
KEEP_ALIVE_OBJS = $(KEEP_ALIVE_SRC:.c=.o)

all:cdemo sdemo keep_alive

sdemo:$(DEMO_OBJS)
	$(CC) -o $@ $^ $(DEMO_LDFLAGS)

cdemo:$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

keep_alive:$(KEEP_ALIVE_OBJS)
	$(CC) -o $@ $^

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o cdemo sdemo
