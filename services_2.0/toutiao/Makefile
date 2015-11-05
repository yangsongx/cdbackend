# Makefile for report user behavior for Toutiao
#
DEBUG=true

CC = g++

SRC = toutiao.cc

#include ../../common/caredear_service_lib/common.mk
CXXFLAGS = `mysql_config --cflags` -W -Wall -Wno-unused-parameter -std=c++0x

# Release should use -O2

ifeq ($(DEBUG),false)
CXXFLAGS += -DDEBUG -O2
else
CXXFLAGS += -DCHECK_MEM_LEAK -DDEBUG -g -O0
endif

LDFLAGS = -lcrypto -lssl -lxml2 -lcurl -lmemcached `mysql_config --libs`

# Let static link as default
# use `make STATICLIB=false' to build out shared lib.
ifeq ($(STATICLIB),false)
LDFLAGS += -lm -ldl -lz -lrt -static
endif

OBJS = $(SRC:.cc=.o)

all:toutiao

toutiao:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.cc.o:
	$(CC) $(CXXFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o toutiao
	find . -name '*.o' -exec rm -rf {} ";"
