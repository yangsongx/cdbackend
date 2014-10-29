# Makefile for the cpp
#
CXX = g++
AR = ar

SRC = token_main.cc \
	tokendata.pb.cc

$(shell export CLASSPATH=protobuf-java-2.5.0.jar)

OBJS = $(SRC:.cc=.o)

LDFLAGS = -lprotobuf

.PHONY: all
all:
	javac -cp protobuf-java-2.5.0.jar TokenMain.java Tokendata.java
	mv *.class com/caredear/

.cc.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY:clean
clean:
	find . -name '*.o' -exec rm -rf {} ";"

