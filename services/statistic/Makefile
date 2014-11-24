#
#

CC = gcc

SRC = gr.c

CFLAGS = -g -O0 `mysql_config --cflags` -I /usr/include/libxml2/
LDFLAGS = `mysql_config --libs` -lxml2 -lz -lm -static

OBJS = $(SRC:.c=.o)

all:gr

gr:$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o gr
