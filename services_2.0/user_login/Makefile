# Makefile for User Login Service(ULS)
#
DEBUG=true
CDSBASE = ../../common/cds_base/

CC = g++

SRC = usr_login_service.cc \
      CommonUserCenter.pb.cc \
      usr_login_db.cc \
      UserLogin.pb.cc \
      UserLoginConfig.cc \
      LoginOperation.cc


#include ../../common/caredear_service_lib/common.mk
CXXFLAGS = -I$(CDSBASE) -I/usr/include/libxml2/ `mysql_config --cflags` -fpermissive -std=c++0x

# Release should use -O2

ifeq ($(DEBUG),false)
CXXFLAGS += -DDEBUG -O2
else
CXXFLAGS += -DCHECK_MEM_LEAK -DDEBUG -g -O0
endif

LDFLAGS = -L . -L $(CDSBASE) -lcds -pthread -levent -lssl -lcrypto -lxml2 -lprotobuf -lmemcached `mysql_config --libs` -luuid-static

# Let static link as default
# use `make STATICLIB=false' to build out shared lib.
ifeq ($(STATICLIB),false)
LDFLAGS += -lm -ldl -lz -lrt -static
endif

OBJS = $(SRC:.cc=.o)

all:uls

uls:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.cc.o:
	$(CC) $(CXXFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o uls a.out
	find . -name '*.o' -exec rm -rf {} ";"
