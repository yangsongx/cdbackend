COMMON_CFLAGS = -W -Wall -Wno-unused-parameter -Wno-old-style-declaration
ifeq ($(DEBUG),false)
COMMON_CFLAGS += -DDEBUG -O2
else
COMMON_CFLAGS += -DDEBUG -g -O0 -DCHECK_MEM_LEAK
endif

BUILD_NUMBER = $(shell date +%Y%m%d)
COMMON_CFLAGS += -DBUILD_NUMBER=$(BUILD_NUMBER)
