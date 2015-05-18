# Makefile for program verify DB data operation(chk_db)
#
DEBUG=true

CC = gcc
SRC = chk_db.c

CFLAGS += -I /usr/include/libxml2/ `mysql_config --cflags` -W -Wall

# Release should use -O2

ifneq ($(DEBUG),false)
CFLAGS += -O2
else
CFLAGS += -DCHECK_MEM_LEAK -DDEBUG -g -O0
endif

LDFLAGS = -pthread -lxml2 `mysql_config --libs`


OBJS = $(SRC:.c=.o)

all:chk_db

chk_db:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o chk_db
