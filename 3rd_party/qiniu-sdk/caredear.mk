# This is caredear Makfile
# which aims to build out Linux .so/.a library based on
# qiniu's SDK code

CC = gcc
AR = ar


SRC = b64/urlsafe_b64.c \
      cJSON/cJSON.c \
      qiniu/auth_mac.c \
      qiniu/base.c \
      qiniu/base_io.c \
      qiniu/conf.c \
      qiniu/http.c \
      qiniu/io.c \
      qiniu/resumable_io.c \
      qiniu/rs.c

OBJS = $(SRC:.c=.o)

CFLAGS = -I qiniu/ -I b64/ -fPIC

LDFLAGS = -shared -fPIC

#STATIC_LDFLAGS = -lm -ldl -lz -lrt -static -fPIC

.PHONY: all
all:qn qnstatic

qn:$(OBJS)
	$(CC) -o libqiniu.so $^ $(LDFLAGS)

qnstatic:$(OBJS)
	$(AR) -rc libqiniu.a $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -f *.so *.a
	find . -name '*.o' -exec rm -rf {} ";"
