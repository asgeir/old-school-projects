CC = gcc
CPPFLAGS =
CFLAGS = -O2 -g -Wall -Wextra -Wformat=2

SOURCES = tftpd.c tftpd_server.c

all: tftpd

tftpd: ${SOURCES}
	${CC} -o tftpd ${CPPFLAGS} ${CFLAGS} ${SOURCES}

clean:
	rm -f *.o *~ tftpd

distclean: clean
	rm -f tftpd
