# Makefile for build token and test client code
#
CONFIG_SINGLE_APPTOKEN=true

CC = gcc

SRC = token_main.c \
      sql_operation.c

CLIENT_TOKEN_SRC = client_token.c


include ../../common/caredear_service_lib/common.mk
CFLAGS = $(COMMON_CFLAGS) -I../../common/caredear_service_lib/ `mysql_config --cflags` -I /usr/include/libxml2/

ifeq ($(CONFIG_SINGLE_APPTOKEN),true)
CFLAGS += -DCONFIG_SINGLE_APPTOKEN
endif

LDFLAGS = -L ../../common/caredear_service_lib/ -lcds -pthread -levent -lcrypto -lxml2 `mysql_config --libs`

# Let static link as default
# use `make STATICLIB=false' to build out shared lib.
ifneq ($(STATICLIB),false)
LDFLAGS += -lm -ldl -lz -lrt -static
endif

OBJS = $(SRC:.c=.o)
CLIENT_TOKEN_OBJS = $(CLIENT_TOKEN_SRC:.c=.o)

all:tauth ct

tauth:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

ct:$(CLIENT_TOKEN_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:install
install:
	mkdir -p $(DESTDIR)/etc/
	cp cds_cfg.xml $(DESTDIR)/etc/
	mkdir -p $(DESTDIR)/usr/bin/
	cp tauth $(DESTDIR)/usr/bin/

.PHONY:clean
clean:
	rm -rf *.o tauth ct
