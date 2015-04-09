# Makefile for User Profile Modification
#
DEBUG=true
CDSBASE = ../../common/cds_base/
THIRD_PARTY=../../3rd_party/
BASECLS=../

CC = g++

SRC = verify_code_service.cc \
      VerifyCode.pb.cc \
      VerifyCodeConfig.cc \
      $(BASECLS)Config.cc \
      $(BASECLS)Operation.cc \
      VerifyCodeOperation.cc

#include ../../common/caredear_service_lib/common.mk
CXXFLAGS = -I$(CDSBASE) -I$(BASECLS) -I/usr/include/libxml2/ `mysql_config --cflags` -fpermissive -std=c++0x

# Release should use -O2

ifeq ($(DEBUG),false)
CXXFLAGS += -DDEBUG -O2
else
CXXFLAGS += -DCHECK_MEM_LEAK -DDEBUG -g -O0
endif

LDFLAGS = -L . -L /usr/local/lib/ -L $(CDSBASE) -L $(THIRD_PARTY)libuuid -lcds -pthread -levent -lssl -lcrypto -lxml2 -lprotobuf -lmemcached `mysql_config --libs` -luuid-static

# Let static link as default
# use `make STATICLIB=false' to build out shared lib.
ifeq ($(STATICLIB),false)
LDFLAGS += -lm -ldl -lz -lrt -static
endif

OBJS = $(SRC:.cc=.o)

all:vcs

vcs:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.cc.o:
	$(CC) $(CXXFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o vcs a.out
	find . -name '*.o' -exec rm -rf {} ";"
