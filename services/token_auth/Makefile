# Makefile for build token and test client code
#
CONFIG_SINGLE_APPTOKEN=true
CDSBASE = ../../common/cds_base/
CC = g++

SRC = token_main.cc \
      sql_operation.cc \
      mem_operation.cc \
      db_access.cc

CLIENT_TOKEN_SRC = client_token.c


CXXFLAGS = -I$(CDSBASE) `mysql_config --cflags` -I /usr/include/libxml2/ -std=gnu++0x

BUILD_NUMBER = $(shell date +%Y%m%d)
CXXFLAGS += -DBUILD_NUMBER=$(BUILD_NUMBER)

ifeq ($(CONFIG_SINGLE_APPTOKEN),true)
CXXFLAGS += -DCONFIG_SINGLE_APPTOKEN
endif

# toggle below two flags..
CXXFLAGS += -DDEBUG -g -O0
#CXXFLAGS += -O2

LDFLAGS = -L $(CDSBASE) -lcds -pthread -levent -lcrypto -lxml2 -lmemcached `mysql_config --libs`

# Let static link as default
# use `make STATICLIB=false' to build out shared lib.
ifneq ($(STATICLIB),false)
LDFLAGS += -lm -ldl -lz -lrt -static
endif

OBJS = $(SRC:.cc=.o)
#CLIENT_TOKEN_OBJS = $(CLIENT_TOKEN_SRC:.c=.o)

all:tauth

tauth:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.cc.o:
	$(CC) $(CXXFLAGS) -c $< -o $@

.PHONY:install
install:
	mkdir -p $(DESTDIR)/etc/
	cp ${CDSBASE}cds_cfg.xml $(DESTDIR)/etc/
	mkdir -p $(DESTDIR)/usr/bin/
	cp tauth $(DESTDIR)/usr/bin/

.PHONY:clean
clean:
	rm -rf *.o tauth ct
