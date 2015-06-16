THIRD_PARTY=../../3rd_party/

CC = gcc

PSRC = thep.c utils.c
CSRC = thec.c

CFLAGS += -I$(THIRD_PARTY)rabbitMQ/rabbitmq-c/librabbitmq/ -W -Wall -Wno-unused-parameter
LDFLAGS += -L$(THIRD_PARTY)rabbitMQ/rabbitmq-c/out_bin/ -lrabbitmq #-pthread -lssl -lcrypto -lrt -ldl -static

POBJS = $(PSRC:.c=.o)
COBJS = $(CSRC:.c=.o)

all:thep thec

thep:$(POBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

thec:$(COBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o thep thec
