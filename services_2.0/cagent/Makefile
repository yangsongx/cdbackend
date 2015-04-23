# A new code start from scratch
CC = gcc

SRC = main.c listreq.c worker_thread.c pipe.c config.c
TESTSRC = testagent.c

OBJS = $(SRC:.c=.o)
TESTOBJS = $(TESTSRC:.c=.o)

CFLAGS += -W -Wall -Wno-unused-parameter -std=c99 -I/usr/include/libxml2/

# toggle below two flags for debug/release
CFLAGS += -g -O0 -DDEBUG
#CFLAGS += -O2

LDFLAGS += -levent -pthread -lxml2

.PHONY: all
all:cagent testagent

cagent:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

testagent:$(TESTOBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -f cagent testagent
	find . -name '*.o' -exec rm -rf {} ";"
