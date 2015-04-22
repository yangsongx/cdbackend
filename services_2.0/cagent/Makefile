# A new code start from scratch
CC = gcc

SRC = main.c listreq.c worker_thread.c pipe.c config.c

OBJS = $(SRC:.c=.o)

CFLAGS += -W -Wall -Wno-unused-parameter -std=c99 -I/usr/include/libxml2/

CFLAGS += -g -O0
#CFLAGS += -O2

LDFLAGS += -levent -pthread -lxml2

.PHONY: all
all:mym 

mym:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -f mym 
	find . -name '*.o' -exec rm -rf {} ";"
