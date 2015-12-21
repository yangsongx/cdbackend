# For image processing study
#
# make
#
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

LDFLAGS = -lxml2 -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs -lpng


OBJS = $(SRC:.cc=.o)
TESTOBJS = $(TESTSRC:.cc=.o)

.PHONY:all
all:extrimg

testimg:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
.PHONY:extrimg
extrimg:
	$(CC) $(CXXFLAGS) -c extractimg.cc -o extrimg.o 
	$(CC) -o extrimg extrimg.o $(LDFLAGS)
.PHONY:cutimg
cutimg:
	$(CC) $(CXXFLAGS) -c cutimg.cc -o cutimg.o 
	$(CC) -o cutimg cutimg.o $(LDFLAGS)
.PHONY:enimg
enimg:
	$(CC) $(CXXFLAGS) -c enhanceimg.cc -o enimg.o 
	$(CC) -o enimg enimg.o $(LDFLAGS)

.cc.o:
	$(CC) $(CXXFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o testimg
	find . -name '*.o' -exec rm -rf {} ";"
