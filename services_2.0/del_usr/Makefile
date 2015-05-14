# Makefile for keep_alive ping program
#
DEBUG=true

CC = gcc
SRC = del_usr.c

CFLAGS += -I /usr/include/libxml2/ `mysql_config --cflags` -W -Wall

# Release should use -O2

ifneq ($(DEBUG),false)
CFLAGS += -O2
else
CFLAGS += -DCHECK_MEM_LEAK -DDEBUG -g -O0
endif

LDFLAGS = -pthread -lxml2 `mysql_config --libs`


OBJS = $(SRC:.c=.o)

all:del_usr

del_usr:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o del_usr 
