##
# make                 (build with DEBUG info)
# make DEBUG=false     (build without DEBUG info, i.e, release version)
# make STATICLIB=false (build with so, otherwise is a static link version)
#
PROJECT_TOP_DIR=$(shell cd ./; pwd)
PROJECT_BIN_DIR=$(PROJECT_TOP_DIR)/bin
PROJECT_SRC_DIR=$(PROJECT_TOP_DIR)/src
PROJECT_INC_DIR=$(PROJECT_TOP_DIR)/inc
PROJECT_LIB_DIR=$(PROJECT_TOP_DIR)/lib
PROJECT_OBJ_DIR=$(PROJECT_BIN_DIR)/objs
MKDIR := mkdir -p

CC := gcc
TARGETS = auth

CFLAGS := -I$(PROJECT_INC_DIR) -Wall -I /usr/include/libxml2/
LDFLAG := -L$(PROJECT_LIB_DIR)  -lcrypto -lxml2 -std=c99

ifeq ($(DEBUG),false)
CFLAGS += -DDEBUG -O2
else
CFLAGS += -DDEBUG -g -O0
endif

ifneq ($(STATICLIB),false)
LDFLAG += -lm -ldl -lz -static
endif

src :=$(wildcard $(PROJECT_SRC_DIR)/*.c)
dir := $(notdir $(src))
PROJECT_OBJ := $(patsubst %.c,%.o,$(dir) )

PROJECT_ALL_OBJS := $(addprefix $(PROJECT_OBJ_DIR)/, $(PROJECT_OBJ))

all:$(PROJECT_SRC_DIR) $(PROJECT_ALL_OBJS)
	$(CC) $(CFLAGS) $(PROJECT_ALL_OBJS) -o $(PROJECT_BIN_DIR)/$(TARGETS) $(LDFLAG)

$(PROJECT_OBJ_DIR)/%.o : $(PROJECT_SRC_DIR)/%.c
	$(MKDIR) $(PROJECT_OBJ_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY:install
install:
	mkdir -p $(DESTDIR)/data/tool/xmpp
	mkdir -p $(DESTDIR)/etc/service/auth
	cp $(PROJECT_SRC_DIR)/cds_cfg.xml $(DESTDIR)/etc/
	cp $(PROJECT_BIN_DIR)/$(TARGETS) $(DESTDIR)/data/tool/xmpp/

clean:
	rm -fr $(PROJECT_OBJ_DIR)
	rm -fr $(TARGETS)

