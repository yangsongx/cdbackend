# Makefile for NetDisk Service
#
CC = gcc

SRC = nds_main.c nds_sql.c

include ../../common/caredear_service_lib/common.mk
CFLAGS = $(COMMON_CFLAGS) -I../../common/caredear_service_lib/ `mysql_config --cflags`

# Release should use -O2

ifeq ($(DEBUG),false)
CFLAGS += -DDEBUG -O2
else
CFLAGS += -DCHECK_MEM_LEAK -DDEBUG -g -O0
endif

LDFLAGS = -L ../../common/caredear_service_lib/ -lcds -pthread -levent -lcrypto -lxml2 `mysql_config --libs`

# Let static link as default
# use `make STATICLIB=false' to build out shared lib.
ifeq ($(STATICLIB),false)
LDFLAGS += -lm -ldl -lz -lrt -static
endif

OBJS = $(SRC:.c=.o)

all:nds

nds:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o nds a.out
