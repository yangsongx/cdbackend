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
CC = g++

SSRC = servercode.cc rpcdata.pb.cc
CSRC = clientcode.cc rpcdata.pb.cc

SOBJS = $(SSRC:.cc=.o)
COBJS = $(CSRC:.cc=.o)

CXXFLAGS = -I/usr/local/include/ -g -O0

LDFLAGS = -lprotobuf

.PHONY:all
all:c s

.cc.o:
	$(CC) $(CXXFLAGS) -c $< -o $@

c:$(COBJS)
	$(CC) -o c $^ $(LDFLAGS)

s::$(SOBJS)
	$(CC)  -o s $^ $(LDFLAGS)

.PHONY:clean
clean:
	rm -f c s
	find . -name '*.o' -exec rm -rf {} ";"

