CC = gcc
CPPFLAGS =
CFLAGS = -O2 -g -Wall -Wextra -Wformat=2

CFLAGS += `pkg-config --cflags glib-2.0`
LDLIBS = `pkg-config --libs glib-2.0`

SOURCES = httpd_connection.c httpd_helpers.c httpd_req_handler.c httpd_req_parser.c httpd_server.c main.c


all: httpd

httpd: $(SOURCES)
	gcc -o httpd $(CFLAGS) $(SOURCES) $(LDLIBS)

clean:
	rm -f *.o *~

distclean: clean
	rm -f httpd
