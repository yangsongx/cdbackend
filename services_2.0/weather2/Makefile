# Makefile
#
DEBUG=true

CC = gcc
SRC = weather2.c

CFLAGS += -I /usr/include/libxml2/ -W -Wall -Wno-unused-parameter

# Release should use -O2

ifneq ($(DEBUG),false)
CFLAGS += -O2
else
CFLAGS += -DCHECK_MEM_LEAK -DDEBUG -g -O0
endif

LDFLAGS = -lxml2 -lsqlite3 -pthread

OBJS = $(SRC:.c=.o)

all:weather2

weather2:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o weather2
