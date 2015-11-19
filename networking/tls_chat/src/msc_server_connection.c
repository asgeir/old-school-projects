#include "msc_server_connection.h"

#include "msc_secure_socket.h"
#include "msc_common.h"
#include "msc_server.h"
#include "msc_srp_auth.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct _msc_server_connection {
	msc_server *server;
	msc_secure_socket *socket;
	GString *in_buf;

	GQuark chatroom;
	GString *user;

	struct SRPVerifier *srp;
	int bad_logins;
} msc_server_connection;

static void msc_server_connection_send_raw_message(msc_server_connection *c, int msg_type, const char *body, size_t body_len)
{
	char header_buf[] = { (char)msg_type, 0x12 };

	// TODO: Don't hardcode authentication group

	GString *buf = g_string_new_len(header_buf, 2);
	buf = g_string_append_len(buf, body, body_len);

	msc_secure_socket_write(c->socket, buf);

	g_string_free(buf, TRUE);
}

static MSC_BOOL msc_server_connection_send_message(msc_server_connection *c, const char *body, size_t body_len)
{
	if ((body_len > 512) || (body_len < 1)) return MSC_FALSE;

	char header_buf[] = { (char)((body_len >> 8) & 0xff), (char)(body_len & 0xff) };

	GString *buf = g_string_new_len(header_buf, 2);
	buf = g_string_append_len(buf, body, body_len);

	msc_server_connection_send_raw_message(c, MSC_SERVER_MSG_OPCODE_SERVER, buf->str, buf->len);

	g_string_free(buf, TRUE);

	return MSC_TRUE;
}

static MSC_BOOL msc_server_connection_send_user_message(msc_server_connection *c, MSC_BOOL is_private, msc_server_connection *from, const char *body, size_t body_len)
{
	if ((body_len > 512) || (body_len < 1)) return MSC_FALSE;
	if ((!from) || (from->user->len > 64) || (from->user->len < 1)) return MSC_FALSE;

	GString *hostname = msc_server_connection_remote_hostname(from);
	GString *hostinfo = g_string_new("");
	g_string_printf(hostinfo, "%s:%d", hostname->str, msc_server_connection_remote_port(from));

	g_string_free(hostname, TRUE);

	char header_buf[] = { (char)((body_len >> 8) & 0xff), (char)(body_len & 0xff), (char)(from->user->len & 0xff), (char)(hostinfo->len & 0xff) };

	GString *buf = g_string_new_len(header_buf, 4);
	buf = g_string_append_len(buf, from->user->str, from->user->len);
	buf = g_string_append_len(buf, hostinfo->str, hostinfo->len);
	buf = g_string_append_len(buf, body, body_len);

	int type = (is_private) ? MSC_SERVER_MSG_OPCODE_PRIVATE : MSC_SERVER_MSG_OPCODE_CHATROOM;
	msc_server_connection_send_raw_message(c, type, buf->str, buf->len);

	g_string_free(buf, TRUE);
	g_string_free(hostinfo, TRUE);

	return MSC_TRUE;
}

static MSC_BOOL msc_server_connection_send_auth1(msc_server_connection *c, char *seed, size_t seed_len, char *B, size_t B_len)
{
	if (seed_len != 4) return MSC_FALSE;
	if ((B_len > 1024) || (B_len < 1)) return MSC_FALSE;

	char header_buf[] = { *(seed), *(seed + 1), *(seed + 2), *(seed + 3), (char)((B_len >> 8) & 0xff), (char)(B_len & 0xff) };

	GString *buf = g_string_new_len(header_buf, 6);
	buf = g_string_append_len(buf, B, B_len);

	msc_server_connection_send_raw_message(c, MSC_SERVER_MSG_OPCODE_AUTH1, buf->str, buf->len);

	g_string_free(buf, TRUE);

	return MSC_TRUE;
}

static MSC_BOOL msc_server_connection_send_auth2(msc_server_connection *c, char *hamk, size_t hamk_len)
{
	if ((hamk_len > 64) || (hamk_len < 1)) return MSC_FALSE;

	char header_buf[] = { (char)(hamk_len & 0xff) };

	GString *buf = g_string_new_len(header_buf, 1);
	buf = g_string_append_len(buf, hamk, hamk_len);

	msc_server_connection_send_raw_message(c, MSC_SERVER_MSG_OPCODE_AUTH2, buf->str, buf->len);

	g_string_free(buf, TRUE);

	return MSC_TRUE;
}

msc_server_connection *msc_server_connection_create(msc_server *server)
{
	msc_server_connection *c = (msc_server_connection *)malloc(sizeof(msc_server_connection));
	if (c)
	{
		c->server = server;
		c->socket = msc_secure_socket_create();
		c->in_buf = g_string_new("");
		c->chatroom = 0;
		c->user = g_string_new("");
		c->srp = NULL;
		c->bad_logins = 0;
	}

	return c;
}

void msc_server_connection_destroy(msc_server_connection *c)
{
	if (!c) return;

	msc_server_connection_close(c);

	msc_secure_socket_destroy(c->socket);
	c->socket = NULL;

	g_string_free(c->in_buf, TRUE);
	c->in_buf = NULL;

	g_string_free(c->user, TRUE);
	c->user = NULL;

	if (c->srp) srp_verifier_delete(c->srp);
	c->srp = NULL;

	free(c);
}

msc_secure_socket *msc_server_connection_socket(msc_server_connection *c)
{
	return c->socket;
}

int msc_server_connection_socket_handle(msc_server_connection *c)
{
	return msc_secure_socket_socket(c->socket);
}

GString *msc_server_connection_remote_hostname(msc_server_connection *c)
{
	return msc_secure_socket_hostname(c->socket);
}

uint16_t msc_server_connection_remote_port(msc_server_connection *c)
{
	return msc_secure_socket_port(c->socket);
}

GString *msc_server_connection_user(msc_server_connection *c)
{
	return c->user;
}

void msc_server_connection_accept(msc_server_connection *c, int guestid, int listenfd)
{
	msc_server_connection_close(c);
	msc_secure_socket_accept(c->socket, listenfd);
	g_string_printf(c->user, "guest%03d", guestid);
	msc_server_connection_send_message(c, "*** Welcome", 11);
}

void msc_server_connection_close(msc_server_connection *c)
{
	msc_server_leave_chatroom(c->server, c->chatroom, c);
	msc_server_connection_send_message(c, "*** Goodbye", 11);
	msc_secure_socket_close(c->socket);
	g_string_truncate(c->in_buf, 0);
	c->chatroom = 0;
	c->bad_logins = 0;
}

typedef struct {
	msc_server_connection *from;
	const char *msg;
	size_t msg_len;
} msc_server_connection_chatroom_message;

static void msc_server_connection_deliver_message_to_chatroom(msc_server_connection *user, void *data)
{
	msc_server_connection_chatroom_message *d = (msc_server_connection_chatroom_message *)data;
	msc_server_connection_send_user_message(user, MSC_FALSE, d->from, d->msg, d->msg_len);
}

static void msc_server_connection_list_chatrooms(GQuark chatroom, void *data)
{
	msc_server_connection *c = (msc_server_connection *)data;

	const char *name = g_quark_to_string(chatroom);
	size_t len = strnlen(name, 64);
	msc_server_connection_send_message(c, name, len);
}

static void msc_server_connection_list_users(msc_server_connection *user, void *data)
{
	msc_server_connection *c = (msc_server_connection *)data;

	GString *hostname = msc_server_connection_remote_hostname(user);

	GString *tmp = g_string_new("");
	g_string_printf(tmp, "%s\t%s\t%d\t%s",
					user->user->str,
					hostname->str,
					msc_server_connection_remote_port(user),
					g_quark_to_string(user->chatroom));

	msc_server_connection_send_message(c, tmp->str, tmp->len);

	g_string_free(tmp, TRUE);
	g_string_free(hostname, TRUE);
}

static void msc_server_connection_handle_auth1(msc_server_connection *c, GString *username, const char *a, size_t a_len)
{
	msc_server_user_auth_info *info = msc_server_user_get_auth_info(c->server, username);

	if (info)
	{
		const unsigned char *b = NULL;
		int b_len = 0;

		if (c->srp) srp_verifier_delete(c->srp);
		c->srp = srp_verifier_new(
			SRP_SHA256, SRP_NG_2048,
			username->str,
			(const unsigned char *)(info->s->str), (int)(info->s->len),
			(const unsigned char *)(info->v->str), (int)(info->v->len),
			(const unsigned char *)a, (int)a_len,
			&b, &b_len,
			NULL, NULL);

		if (b)
		{
			msc_server_connection_send_auth1(c, info->s->str, info->s->len, (char *)b, (size_t)b_len);
		}
		else
		{
			// This is a failure of the SRP process so it won't be counted against the user
			msc_server_log_authentication_failed(c->server, c, srp_verifier_get_username(c->srp));

			srp_verifier_delete(c->srp);
			c->srp = NULL;
		}

		g_string_free(info->s, TRUE);
		g_string_free(info->v, TRUE);
		free(info);
	}
}

static void msc_server_connection_handle_auth2(msc_server_connection *c, const char *m)
{
	if (!c->srp) return;

	const unsigned char *hamk = NULL;
	srp_verifier_verify_session(c->srp, (const unsigned char *)m, &hamk);

	if (hamk)
	{
		if (srp_verifier_is_authenticated(c->srp))
		{
			msc_server_connection_send_auth2(c, (char *)hamk, (256 / 8));

			if (c->user) g_string_free(c->user, TRUE);
			c->user = g_string_new(srp_verifier_get_username(c->srp));

			msc_server_log_authenticated(c->server, c);
			c->bad_logins = 0;
		}
		else
		{
			++c->bad_logins;
			msc_server_log_authentication_failed(c->server, c, srp_verifier_get_username(c->srp));

			srp_verifier_delete(c->srp);
			c->srp = NULL;
		}
	}
	else
	{
		++c->bad_logins;
		msc_server_log_authentication_failed(c->server, c, srp_verifier_get_username(c->srp));

		srp_verifier_delete(c->srp);
		c->srp = NULL;
	}
}

MSC_BOOL msc_server_connection_receive(msc_server_connection *c)
{
	g_string_truncate(c->in_buf, 0);
	MSC_SOCKET_ERR e = msc_secure_socket_read(c->socket, c->in_buf);
	if (e)
	{
		// connection has been closed
		return MSC_TRUE;
	}

//	GString *message = NULL;
	size_t message_len = 0;
	GString *target = NULL;
	size_t target_len = 0;
	GQuark chatroom = 0;
	int phase = -1;
	msc_server_connection_chatroom_message chat_msg;
	msc_server_connection *target_user = NULL;

	char *p = c->in_buf->str;
	char *end = c->in_buf->str + c->in_buf->len;
	while (p < end)
	{
		switch (*(p++))
		{
		case MSC_CLIENT_MSG_OPCODE_BYE:
			return MSC_TRUE;

		case MSC_CLIENT_MSG_OPCODE_MESSAGE:
			message_len = (size_t)(*(p++) << 8);
			message_len = (size_t)(message_len | *(p++));
			message_len = (message_len > 512) ? 512 : message_len;
			message_len = (message_len > (end - p)) ? (end - p) : message_len;

			chat_msg.from = c;
			chat_msg.msg = p;
			chat_msg.msg_len = message_len;
			msc_server_foreach_user_in_chatroom(c->server, c->chatroom, msc_server_connection_deliver_message_to_chatroom, &chat_msg);

			p += message_len;
			break;

		case MSC_CLIENT_MSG_OPCODE_SAY:
			target_len = (size_t)(*(p++));
			target_len = (target_len > 64) ? 64 : target_len;
			target_len = (target_len > (end - p)) ? (end - p) : target_len;

			message_len = (size_t)(*(p++) << 8);
			message_len = (size_t)(message_len | *(p++));
			message_len = (message_len > 512) ? 512 : message_len;
			message_len = (message_len > (end - p)) ? (end - p) : message_len;

			target = g_string_new_len(p, target_len);
			p += target_len;

			target_user = msc_server_user(c->server, target);
			if (target_user)
			{
				msc_server_connection_send_user_message(target_user, MSC_TRUE, c, p, message_len);
			}

			p += message_len;
			g_string_free(target, TRUE);

			break;

		case MSC_CLIENT_MSG_OPCODE_WHO:
			msc_server_connection_send_message(c, "user\thost\tport\tchatroom", 23);
			msc_server_foreach_user(c->server, msc_server_connection_list_users, c);
			break;

		case MSC_CLIENT_MSG_OPCODE_LIST:
			msc_server_foreach_chatroom(c->server, msc_server_connection_list_chatrooms, c);
			break;

		case MSC_CLIENT_MSG_OPCODE_JOIN:
			target_len = (size_t)(*(p++));
			target_len = (target_len > 64) ? 64 : target_len;
			target_len = (target_len > (end - p)) ? (end - p) : target_len;

			chatroom = g_quark_from_string(p);
			msc_server_leave_chatroom(c->server, c->chatroom, c);
			msc_server_join_chatroom(c->server, chatroom, c);
			c->chatroom = chatroom;

			p += target_len;
			break;

		case MSC_CLIENT_MSG_OPCODE_AUTH:
			phase = *(p++);

			target_len = (size_t)(*(p++));
			target_len = (target_len > 64) ? 64 : target_len;
			target_len = (target_len > (end - p)) ? (end - p) : target_len;

			if (phase == 0)
			{
				message_len = (size_t)(*(p++) << 8);
				message_len = (size_t)(message_len | *(p++));
				message_len = (message_len > 1024) ? 1024 : message_len;
				message_len = (message_len > (end - p)) ? (end - p) : message_len;

				target = g_string_new_len(p, target_len);
				p += target_len;

				msc_server_connection_handle_auth1(c, target, p, message_len);

				p += message_len;
				g_string_free(target, TRUE);
			}
			else if (phase == 1)
			{
				msc_server_connection_handle_auth2(c, p);
				p += target_len;
			}
			else
			{
				p = end;
			}
			break;

		case MSC_CLIENT_MSG_OPCODE_GAME:
		case MSC_CLIENT_MSG_OPCODE_ROLL:
		default:
			// Invalid opcode. Don't try to resynchronize.
			p = end;
			break;
		}
	}

	if (c->bad_logins >= 3) return MSC_TRUE;

	return MSC_FALSE;
}
