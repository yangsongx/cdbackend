# Makefile for NewUser Service (nus)
#
DEBUG=true

CC = g++
SRC = NewUserMessage.pb.cc \
      newusr.cc \
      newusr_db.cc


#include ../../common/caredear_service_lib/common.mk
CXXFLAGS = $(COMMON_CFLAGS) -I../../common/cds_base/ -I /usr/include/libxml2/ `mysql_config --cflags` -fpermissive

# Release should use -O2

ifeq ($(DEBUG),false)
CXXFLAGS += -DDEBUG -O2
else
CXXFLAGS += -DCHECK_MEM_LEAK -DDEBUG -g -O0
endif

LDFLAGS = -L ../../common/cds_base/ -lcds -pthread -levent -lcurl -lssl -lcrypto -lxml2 -lprotobuf `mysql_config --libs`

# Let static link as default
# use `make STATICLIB=false' to build out shared lib.
ifeq ($(STATICLIB),false)
LDFLAGS += -lldap_r -llber -lidn -lrtmp -lm -ldl -lz -lrt -static
endif

OBJS = $(SRC:.cc=.o)

all:nus

nus:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.cc.o:
	$(CC) $(CXXFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o nus a.out
	find $(QINIU_SDK) -name '*.o' -exec rm -rf {} ";"
