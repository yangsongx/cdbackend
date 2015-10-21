# Makefile for report user behavior for Toutiao
#
DEBUG=true

CC = gcc

SRC = toutiao.c

#include ../../common/caredear_service_lib/common.mk
CFLAGS = -W -Wall -Wno-unused-parameter

# Release should use -O2

ifeq ($(DEBUG),false)
CFLAGS += -DDEBUG -O2
else
CFLAGS += -DCHECK_MEM_LEAK -DDEBUG -g -O0
endif

LDFLAGS = -lxml2 -lcurl

# Let static link as default
# use `make STATICLIB=false' to build out shared lib.
ifeq ($(STATICLIB),false)
LDFLAGS += -lm -ldl -lz -lrt -static
endif

OBJS = $(SRC:.c=.o)

all:toutiao

toutiao:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o toutiao
	find . -name '*.o' -exec rm -rf {} ";"
