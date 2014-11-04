ARCH := $(shell uname -m)
X64 = x86_64
CC = gcc
PROGS = cagent

ifeq ($(STATICLIB),true)
LDFALGS = -levent -lm -static
else
LDFLAGS = -levent -lm
endif

CFLAGS = -Wall -g -O2 -I/usr/local/include $(M64)

all: $(PROGS)

STPROG = cds_cluster.o ketama.o

ketama.o: ketama.c ketama.h
	$(CC) $(CFLAGS) -c -o $@ ketama.c

cds_cluster.o: cds_cluster.c ketama.h
	$(CC) $(CFLAGS) -c -o $@ cds_cluster.c

cagent: $(STPROG)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o *~ $(PROGS)
