#ifndef CHAT_MSC_CLIENT_H
#define CHAT_MSC_CLIENT_H

#include "msc_common.h"

#include <glib.h>

#include <stdint.h>
#include <stddef.h>

typedef struct _msc_client msc_client;
typedef int MSC_CLIENT_ERR;

typedef void(*msc_client_message_callback)(MSC_BOOL is_srv_msg, MSC_BOOL is_priv_msg, GString *from_user, GString *from_host, GString *msg);

#define MSC_CLIENT_ERR_NO_ERROR        (0)
#define MSC_CLIENT_ERR_NOT_CONNECTED   (1)
#define MSC_CLIENT_ERR_CHATROOM_LEN    (2)
#define MSC_CLIENT_ERR_USERNAME_LEN    (3)
#define MSC_CLIENT_ERR_NOT_IMPLEMENTED (0xffff)

const char *msc_client_err_desc(MSC_CLIENT_ERR err);

msc_client *msc_client_create();
void msc_client_destroy(msc_client *c);

MSC_CLIENT_ERR msc_client_connect(msc_client *c, const char *hostname, uint16_t port);
void msc_client_disconnect(msc_client *c);
MSC_BOOL msc_client_pump_messages(msc_client *c, msc_client_message_callback msg_callback);

GString *msc_client_authenticated_user_name(msc_client *c);
GString *msc_client_chatroom(msc_client *c);

MSC_CLIENT_ERR msc_client_authenticate(msc_client *c, const GString *user, const GString *passwd);
MSC_CLIENT_ERR msc_client_join_chatroom(msc_client *c, const GString *chatroom);
MSC_CLIENT_ERR msc_client_list_chatrooms(msc_client *c);
MSC_CLIENT_ERR msc_client_list_users(msc_client *c);
MSC_CLIENT_ERR msc_client_say(msc_client *c, const GString *target, const GString *msg);

MSC_CLIENT_ERR msc_client_game_start(msc_client *c, const GString *target);
MSC_CLIENT_ERR msc_client_game_roll(msc_client *c);

#endif //CHAT_MSC_CLIENT_H
