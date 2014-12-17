# Makefile for Circle Management Service
#
CC = gcc

SRC = cms_main.c cms_sql.c cms_auth.c


include ../../common/cds_base/common.mk
COMMON_CFLAGS = -I ../../common/cds_base/ `mysql_config --cflags`
CFLAGS = $(COMMON_CFLAGS) -I /usr/include/libxml2/

# Release should use -O2

ifeq ($(DEBUG),false)
CFLAGS += -DDEBUG -O2
else
CFLAGS += -DCHECK_MEM_LEAK -DDEBUG -g -O0
endif

LDFLAGS = -L ../../common/cds_base/ -lcds -pthread -levent -lcrypto -lxml2 `mysql_config --libs`

# Let static link as default
# use `make STATICLIB=false' to build out shared lib.
ifeq ($(STATICLIB),true)
LDFLAGS += -lm -ldl -lz -lrt -static
endif

OBJS = $(SRC:.c=.o)

all:cms

cms:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o cms a.out
