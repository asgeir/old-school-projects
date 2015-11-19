CC = gcc
CFLAGS = -std=gnu11 -O2 -g -Wall -Wextra -Wformat=2 $(shell pkg-config glib-2.0 openssl sqlite3 --cflags)

MSC_SOURCES := $(wildcard msc_*.c)
MSC_OBJS := $(patsubst %.c, %.o, $(MSC_SOURCES))

all: chatd chatd_user chat

chat: chat.o $(MSC_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lreadline $(shell pkg-config glib-2.0 openssl sqlite3 --libs)

chatd: chatd.o $(MSC_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(shell pkg-config glib-2.0 openssl sqlite3 --libs)

chatd_user: chatd_user.o $(MSC_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(shell pkg-config glib-2.0 openssl sqlite3 --libs)

clean:
	rm -f *.o *~

distclean: clean
	rm -f chatd chatd_user chat

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%: %.c
	$(CC) $(CFLAGS) -o $@ $<
