# Makefile for the server framework code
# let's call it CDS (CareDear Service)
#
# check doc/tutorial.pdf for the details of this package.
#
# [Usage]
# make cds   - `build a binary program(main entry)'
# make cs    - `build a .so shared library'
# make ca    - `build a .a static library'
# make all   - `build all of above'
#
CC = gcc
AR = ar

SRC = cache.c \
	memcached.c \
	daemon.c \
	stats.c \
	thread.c \
	parse_cfgfile.c \
	util.c

OBJS = $(SRC:.c=.o)

include common.mk
CFLAGS = $(COMMON_CFLAGS) -std=c99 -I /usr/include/libxml2/

# keep the log output to file
CFLAGS += -DCONFIG_LOG2FILE

LDFLAGS = -levent -pthread -lcrypto -lxml2

# Let static link as default
# use `make STATICLIB=false' to build out shared lib.
ifneq ($(STATICLIB),false)
LDFLAGS += -lm -ldl -lz -lrt -static
endif


SO_OBJS = s_cache.o s_memcached.o \
	  s_daemon.o  s_parse_cfgfile.o \
	  s_stats.o s_thread.o s_util.o

# USE_LIB used to bypass main entry() in src
SO_CFLAGS = $(CFLAGS) -std=c99 -fPIC -DUSE_LIB

ifeq ($(DEBUG),false)
SO_CFLAGS += -DDEBUG -O2
else
SO_CFLAGS += -DDEBUG -g -O0
endif

SO_LDFLAGS = -shared -fPIC

.PHONY: all
all: cds cs ca testcode token circle nds nus ka

cds:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

cs:$(SO_OBJS)
	$(CC) $(SO_LDFLAGS) -o libcds.so $^

s_cache.o : cache.c
	$(CC) $(SO_CFLAGS) -c $< -o $@
s_memcached.o : memcached.c
	$(CC) $(SO_CFLAGS) -c $< -o $@
s_daemon.o : daemon.c
	$(CC) $(SO_CFLAGS) -c $< -o $@
s_stats.o : stats.c
	$(CC) $(SO_CFLAGS) -c $< -o $@
s_thread.o : thread.c
	$(CC) $(SO_CFLAGS) -c $< -o $@
s_parse_cfgfile.o : parse_cfgfile.c
	$(CC) $(SO_CFLAGS) -c $< -o $@
s_util.o: util.c
	$(CC) $(SO_CFLAGS) -c $< -o $@

ca:$(SO_OBJS)
	$(AR) -rc libcds.a $(SO_OBJS)

token:
	cd ../../services/token_auth/ && $(MAKE)

ka:
	cd ../../services/keep_alive/ && $(MAKE)

circle:
	cd ../../services/circle_mgr/ && $(MAKE)

nds:
	cd ../../services/netdisk/ && $(MAKE)

nus:
	cd ../../services/new_user/ && $(MAKE)

testcode:
	cd test/ && $(MAKE)

# FIXME - Should I call ldconfig for below?
.PHONY:install
install:
	mkdir -p $(DESTDIR)/usr/lib/
	cp libcds.so $(DESTDIR)/usr/lib/libcds.so

.PHONY:clean
clean:
	rm -f cds libcds.so libcds.a test/cdemo
	find . -name '*.o' -exec rm -rf {} ";"
	find ../../services/token_auth/ -name '*.o' -exec rm -rf {} ";"
	find ../../services/keep_alive/ -name '*.o' -exec rm -rf {} ";"
	find ../../services/netdisk/ -name '*.o' -exec rm -rf {} ";"
	find ../../services/new_user/ -name '*.o' -exec rm -rf {} ";"
	find ../../services/circle_mgr/ -name '*.o' -exec rm -rf {} ";"

