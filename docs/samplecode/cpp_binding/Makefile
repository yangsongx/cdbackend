# Makefile for the cpp
#
CXX = g++
AR = ar

SRC = token_main.cc \
	TokenMessage.pb.cc

OBJS = $(SRC:.cc=.o)

LDFLAGS = -lprotobuf

.PHONY: all
all: thecpp

thecpp:$(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

.cc.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY:clean
clean:
	find . -name '*.o' -exec rm -rf {} ";"

