# Makefile for Activation Service
#
DEBUG=true
CDSBASE=../../common/cds_base/
BASECLS=../

CC = g++

SRC = password_manager.cc \
      PasswordManager.pb.cc \
      PasswordOperation.cc \
      PasswordConfig.cc \
      $(BASECLS)Config.cc \
      $(BASECLS)Operation.cc




#include ../../common/caredear_service_lib/common.mk
CXXFLAGS = -I$(CDSBASE) -I$(BASECLS) -I/usr/include/libxml2/ `mysql_config --cflags` -fpermissive -std=c++0x -W -Wall -Wno-unused-parameter

# Release should use -O2

ifeq ($(DEBUG),false)
CXXFLAGS += -DDEBUG -O2
else
CXXFLAGS += -DCHECK_MEM_LEAK -DDEBUG -g -O0
endif

LDFLAGS = -L . -L $(CDSBASE) -lcds -pthread -levent -lssl -lcrypto -lxml2 -lprotobuf -lmemcached `mysql_config --libs`

# Let static link as default
# use `make STATICLIB=false' to build out shared lib.
ifeq ($(STATICLIB),false)
LDFLAGS += -lm -ldl -lz -lrt -static
endif

OBJS = $(SRC:.cc=.o)

all:passwdmgr

passwdmgr:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)


.cc.o:
	$(CC) $(CXXFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o passwdmgr a.out
	find . -name '*.o' -exec rm -rf {} ";"
