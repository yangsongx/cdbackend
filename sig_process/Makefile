# For image processing study
DEBUG=true

CC = g++

SRC = main.cc

#include ../../common/caredear_service_lib/common.mk
CXXFLAGS = -I/usr/include/libxml2/ -fpermissive -std=c++0x -W -Wall -Wno-unused-parameter

# Release should use -O2

ifeq ($(DEBUG),false)
CXXFLAGS += -DDEBUG -O2
else
CXXFLAGS += -DCHECK_MEM_LEAK -DDEBUG -g -O0
endif

LDFLAGS = -lxml2 -lopencv_core -lopencv_highgui -lopencv_imgproc -lpng


OBJS = $(SRC:.cc=.o)
TESTOBJS = $(TESTSRC:.cc=.o)

all:testimg

testimg:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.cc.o:
	$(CC) $(CXXFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o testimg
	find . -name '*.o' -exec rm -rf {} ";"
