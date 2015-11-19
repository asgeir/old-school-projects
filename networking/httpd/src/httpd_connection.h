#ifndef HTTPD_HTTPD_CONNECTION_H
#define HTTPD_HTTPD_CONNECTION_H

#include <glib.h>
#include <stddef.h>
#include <stdio.h>

typedef struct _connection connection;

connection *connection_create(FILE *log_file);
void connection_destroy(connection *c);

int      connection_socket(connection *c);
int      connection_timedout(connection *c);
int      connection_is_persistent(connection *c);
GString *connection_client_ip(connection *c);
int      connection_client_port(connection *c);

void connection_accept(connection *c, int listenfd);
void connection_close(connection *c);
int connection_receive(connection *c);
int connection_send(connection *c, char *data, size_t n);
void connection_timeout_increment(connection *c, float seconds);

#endif //HTTPD_HTTPD_CONNECTION_H
