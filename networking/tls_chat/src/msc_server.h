#ifndef CHAT_MSC_SERVER_H
#define CHAT_MSC_SERVER_H

#include "msc_common.h"

#include <glib.h>

#include <stdint.h>
#include <stddef.h>

typedef struct _msc_server msc_server;
typedef struct _msc_server_connection msc_server_connection;
typedef int MSC_SERVER_ERR;

typedef struct _msc_server_user_auth_info {
	GString *s;
	GString *v;
} msc_server_user_auth_info;

#define MSC_SERVER_ERR_NO_ERROR        (0)
#define MSC_SERVER_ERR_BIND            (1)
#define MSC_SERVER_ERR_LISTEN          (2)
#define MSC_SERVER_ERR_NOT_IMPLEMENTED (0xffff)

#define MSC_SERVER_DEFAULT_BACKLOG (10)

typedef void (*msc_server_chatroom_callback)(GQuark chatroom, void *data);
typedef void (*msc_server_user_callback)(msc_server_connection *user, void *data);

const char *msc_server_err_desc(MSC_SERVER_ERR err);

msc_server *msc_server_create(uint16_t port, const char *cert_file, const char *key_file, int backlog);
void msc_server_destroy(msc_server *s);

msc_server_connection *msc_server_user(msc_server *s, GString *user);
msc_server_user_auth_info *msc_server_user_get_auth_info(msc_server *s, GString *user);
void msc_server_foreach_chatroom(msc_server *s, msc_server_chatroom_callback callback, void *data);
void msc_server_foreach_user(msc_server *s, msc_server_user_callback callback, void *data);
void msc_server_foreach_user_in_chatroom(msc_server *s, GQuark chatroom, msc_server_user_callback callback, void *data);

void msc_server_join_chatroom(msc_server *s, GQuark chatroom, msc_server_connection *c);
void msc_server_leave_chatroom(msc_server *s, GQuark chatroom, msc_server_connection *c);
void msc_server_log_authenticated(msc_server *s, msc_server_connection *c);
void msc_server_log_authentication_failed(msc_server *s, msc_server_connection *c, const char *failed_user);

MSC_SERVER_ERR msc_server_listen(msc_server *s);
void msc_server_close(msc_server *s);
void msc_server_wait(msc_server *s);

#endif //CHAT_MSC_SERVER_H
