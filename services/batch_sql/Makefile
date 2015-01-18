# Makefile for OpenSIPS account info
#
DEBUG=true

CC = gcc
SRC = insert.c

CFLAGS += -I /usr/include/libxml2/ -W -Wall `mysql_config --cflags`

# Release should use -O2

ifeq ($(DEBUG),false)
CFLAGS += -O2
else
CFLAGS += -DCHECK_MEM_LEAK -DDEBUG -g -O0
endif

LDFLAGS = -levent -lcrypto -pthread -lxml2 `mysql_config --libs`
ifneq ($(STATICLIB),false)
LDFLAGS += -lm -ldl -lz -lrt -static
endif

OBJS = $(SRC:.c=.o)

all:insert

insert:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o insert 
