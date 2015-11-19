#ifndef CHAT_MSC_SERVER_CONNECTION_H
#define CHAT_MSC_SERVER_CONNECTION_H

#include "msc_common.h"

#include <glib.h>
#include <stdint.h>

typedef struct _msc_server msc_server;
typedef struct _msc_server_connection msc_server_connection;
typedef struct _msc_secure_socket msc_secure_socket;

msc_server_connection *msc_server_connection_create(msc_server *server);
void msc_server_connection_destroy(msc_server_connection *c);

msc_secure_socket *msc_server_connection_socket(msc_server_connection *c);
int msc_server_connection_socket_handle(msc_server_connection *c);
GString *msc_server_connection_remote_hostname(msc_server_connection *c);
uint16_t msc_server_connection_remote_port(msc_server_connection *c);
GString *msc_server_connection_user(msc_server_connection *c);

void msc_server_connection_accept(msc_server_connection *c, int guestid, int listenfd);
void msc_server_connection_close(msc_server_connection *c);
MSC_BOOL msc_server_connection_receive(msc_server_connection *c);


#endif //CHAT_MSC_SERVER_CONNECTION_H
