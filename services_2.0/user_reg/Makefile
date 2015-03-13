# Makefile for User Registration Service(URS)
#
DEBUG=true
CDSBASE = ../../common/cds_base/
LOGINBASE=../user_login/
ACTIVATEBASE=../user_activate/
AUTHBASE=../user_auth/
#base class location
BASECLS=../

CC = g++

SRC = usr_reg_service.cc \
      data_access.cc \
      UserRegister.pb.cc \
      RegOperation.cc \
      UserRegConfig.cc \
      $(BASECLS)CommonUserCenter.pb.cc \
      $(BASECLS)Config.cc \
      $(BASECLS)Operation.cc


TESTSRC = test_client.cc UserRegister.pb.cc \
          $(BASECLS)CommonUserCenter.pb.cc \
          $(LOGINBASE)UserLogin.pb.cc \
          $(ACTIVATEBASE)UserActivation.pb.cc \
          $(AUTHBASE)UserAuth.pb.cc

#include ../../common/caredear_service_lib/common.mk
CXXFLAGS = -I$(BASECLS) -I$(AUTHBASE) -I$(LOGINBASE) -I$(ACTIVATEBASE) -I$(CDSBASE) -I/usr/include/libxml2/ `mysql_config --cflags` -fpermissive -std=c++0x -W -Wall -Wno-unused-parameter

# Release should use -O2

ifeq ($(DEBUG),false)
CXXFLAGS += -DDEBUG -O2
else
CXXFLAGS += -DCHECK_MEM_LEAK -DDEBUG -g -O0
endif

LDFLAGS = -L . -L $(CDSBASE) -lcds -pthread -levent -lssl -lcrypto -lxml2 -lprotobuf -lmemcached `mysql_config --libs` -luuid-static
TESTLDFLAGS = -lprotobuf -lxml2 `mysql_config --libs` -lssl -lcrypto

# Let static link as default
# use `make STATICLIB=false' to build out shared lib.
ifeq ($(STATICLIB),false)
LDFLAGS += -lm -ldl -lz -lrt -static
endif

OBJS = $(SRC:.cc=.o)
TESTOBJS = $(TESTSRC:.cc=.o)

all:urs test_reg

urs:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

test_reg:$(TESTOBJS)
	$(CC) -o $@ $^ $(TESTLDFLAGS)

.cc.o:
	$(CC) $(CXXFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o urs test_reg a.out
	find . -name '*.o' -exec rm -rf {} ";"
