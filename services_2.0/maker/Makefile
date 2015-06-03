# Makefile for Caredear Maker(cdmk) Customization System
#
DEBUG=true
THIRD_PARTY=../../3rd_party/
QINIU_SDK = $(THIRD_PARTY)qiniu-sdk/
CDSBASE=../../common/cds_base/
BASECLS=../

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
      $(BASECLS)Config.cc \
      $(BASECLS)Operation.cc \
      maker.cc \
      MakerMessage.pb.cc \
      MakerConfig.cc \
      MakerOperation.cc


#include ../../common/caredear_service_lib/common.mk
CXXFLAGS = -I$(CDSBASE) -I$(BASECLS) -I/usr/include/libxml2/ `mysql_config --cflags` -fpermissive -std=c++0x -W -Wall -Wno-unused-parameter

# Release should use -O2

ifeq ($(DEBUG),false)
CXXFLAGS += -DDEBUG -O2
else
CXXFLAGS += -DCHECK_MEM_LEAK -DDEBUG -g -O0
endif

LDFLAGS = -L . -L $(CDSBASE) -L $(THIRD_PARTY)libuuid/ -lcds -pthread -levent -lcurl -lssl -lcrypto -lxml2 -lprotobuf -lmemcached -luuid-static `mysql_config --libs`

# Let static link as default
# use `make STATICLIB=false' to build out shared lib.
ifeq ($(STATICLIB),false)
LDFLAGS += -lm -ldl -lz -lrt -static
endif

OBJS = $(SRC:.cc=.o)

all:cdmk

cdmk:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)


.cc.o:
	$(CC) $(CXXFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o cdmk a.out
	find . -name '*.o' -exec rm -rf {} ";"
