#ifndef HTTPD_HTTPD_SERVER_H
#define HTTPD_HTTPD_SERVER_H

#include <stdint.h>

typedef struct _server server;

server *server_create(uint16_t port);
void server_destroy(server *s);

void server_listen(server *s);
void server_close(server *s);
void server_wait(server *s);

#endif //HTTPD_HTTPD_SERVER_H
