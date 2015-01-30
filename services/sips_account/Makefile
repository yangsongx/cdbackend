# Makefile for OpenSIPS account info
#
DEBUG=true

CDSBASE = ../../common/cds_base/
PREFIX_DIR=/opt/sips_account
DEB_DIR=$(DESTDIR)${PREFIX_DIR}

CC = gcc
SRC = sips_account.c

CFLAGS += -I /usr/include/libxml2/ -W -Wall -Wno-unused-parameter `mysql_config --cflags` -I../../common/cds_base/

# program running dir
CFLAGS += -DCONFIG_PREFIX=\"${PREFIX_DIR}\"

# Release should use -O2
ifeq ($(DEBUG),false)
CFLAGS += -O2
else
CFLAGS += -DCHECK_MEM_LEAK -DDEBUG -g -O0
endif

LDFLAGS = -L ../../common/cds_base/ -lcds -levent -lcrypto -pthread -lxml2 `mysql_config --libs`
ifneq ($(STATICLIB),false)
LDFLAGS += -lm -ldl -lz -lrt -static
endif

OBJS = $(SRC:.c=.o)

all:sipsaccount

sipsaccount:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:install
install:
	mkdir -p $(DEB_DIR)
	cp ${CDSBASE}cds_cfg.xml $(DEB_DIR)
	cp sipsaccount $(DEB_DIR)

.PHONY:clean
clean:
	rm -rf *.o sipsaccount
