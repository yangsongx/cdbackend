# Makefile for NetDisk Service
#
DEBUG=true
QINIU_SDK = ../../3rd_party/qiniu-sdk/

CC = g++
SRC = $(QINIU_SDK)qiniu/auth_mac.cc \
      $(QINIU_SDK)qiniu/base.cc \
      $(QINIU_SDK)qiniu/base_io.cc \
      $(QINIU_SDK)qiniu/http.cc \
      $(QINIU_SDK)qiniu/io.cc \
      $(QINIU_SDK)qiniu/conf.cc \
      $(QINIU_SDK)qiniu/resumable_io.cc \
      $(QINIU_SDK)qiniu/rs.cc \
      $(QINIU_SDK)b64/urlsafe_b64.cc \
      $(QINIU_SDK)cJSON/cJSON.cc \
      NetdiskMessage.pb.cc \
      nds_main.cc \
      nds_sql.cc


#include ../../common/caredear_service_lib/common.mk
CXXFLAGS = $(COMMON_CFLAGS) -I../../common/cds_base/ -I $(QINIU_SDK) -I /usr/include/libxml2/ `mysql_config --cflags` -fpermissive

# Release should use -O2

ifeq ($(DEBUG),false)
CXXFLAGS += -DDEBUG -O2
else
CXXFLAGS += -DCHECK_MEM_LEAK -DDEBUG -g -O0
endif

LDFLAGS = -L ../../common/cds_base/ -L $(QINIU_SDK) -lcds -pthread -levent -lcurl -lssl -lcrypto -lxml2 -lprotobuf `mysql_config --libs`

# Let static link as default
# use `make STATICLIB=false' to build out shared lib.
ifeq ($(STATICLIB),false)
LDFLAGS += -lldap_r -llber -lidn -lrtmp -lm -ldl -lz -lrt -static
endif

OBJS = $(SRC:.cc=.o)

all:nds

nds:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.cc.o:
	$(CC) $(CXXFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o nds a.out
	find $(QINIU_SDK) -name '*.o' -exec rm -rf {} ";"
