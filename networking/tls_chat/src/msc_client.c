#include "msc_client.h"
#include "msc_secure_socket.h"
#include "msc_srp_auth.h"

#include <stdlib.h>
#include <string.h>

#define min(x, y) (((x) < (y)) ? (x) : (y))

typedef struct _msc_client {
	msc_secure_socket *socket;

	GString *user;
	GString *chatroom;

	struct SRPUser *srp;
} msc_client;

const char *msc_client_err_desc(MSC_CLIENT_ERR err)
{
	switch (err)
	{
	case MSC_CLIENT_ERR_NO_ERROR:
		return "No error";

	case MSC_CLIENT_ERR_NOT_CONNECTED:
		return "Not connected";

	case MSC_CLIENT_ERR_NOT_IMPLEMENTED:
		return "Function not implemented";

	case MSC_CLIENT_ERR_CHATROOM_LEN:
		return "Chatroom name must be between 1 and 64 characters in length";

	case MSC_CLIENT_ERR_USERNAME_LEN:
		return "Username must be between 1 and 64 characters in length";

	default:
		return "Invalid client error";
	}
}

msc_client *msc_client_create()
{
	msc_client *c = (msc_client *)malloc(sizeof(msc_client));
	if (c)
	{
		c->socket = msc_secure_socket_create();
		c->user = NULL;
		c->chatroom = NULL;
		c->srp = NULL;
	}

	return c;
}

void msc_client_destroy(msc_client *c)
{
	if (!c) return;

	msc_client_disconnect(c);

	if (c->socket) msc_secure_socket_destroy(c->socket);
	c->socket = NULL;

	if (c->srp) srp_user_delete(c->srp);
	c->srp = NULL;

	free(c);
}

MSC_CLIENT_ERR msc_client_connect(msc_client *c, const char *hostname, uint16_t port)
{
	MSC_SOCKET_ERR e = msc_secure_socket_connect(c->socket, hostname, port);
	if (e)
	{
		return MSC_CLIENT_ERR_NOT_CONNECTED;
	}

	return MSC_CLIENT_ERR_NO_ERROR;
}

void msc_client_disconnect(msc_client *c)
{
	if (!c) return;

	GString *msg = g_string_new_len("\0", 1);
	msc_secure_socket_write(c->socket, msg);
	g_string_free(msg, TRUE);

	msc_secure_socket_close(c->socket);

	if (c->user) g_string_free(c->user, TRUE);
	c->user = NULL;

	if (c->chatroom) g_string_free(c->chatroom, TRUE);
	c->chatroom = NULL;

	if (c->srp) srp_user_delete(c->srp);
	c->srp = NULL;
}

static void msc_client_handle_auth1(msc_client *c, char *seed, int seed_len, GString *b)
{
	const unsigned char *m = NULL;
	int m_len = 0;
	srp_user_process_challenge(c->srp, (const unsigned char *)seed, seed_len, (const unsigned char *)b->str, (int)b->len, &m, &m_len);

	if (m)
	{
		char header_buf[] = { MSC_CLIENT_MSG_OPCODE_AUTH, 0x01, (char)(m_len & 0xff) };

		GString *buf = g_string_new_len(header_buf, 3);
		buf = g_string_append_len(buf, (char *)m, m_len);

		msc_secure_socket_write(c->socket, buf);

		g_string_free(buf, TRUE);
	}
}

static void msc_client_handle_auth2(msc_client *c, GString *hamk)
{
	srp_user_verify_session(c->srp, (const unsigned char *)hamk->str);

	if (srp_user_is_authenticated(c->srp))
	{
		if (c->user) g_string_free(c->user, TRUE);
		c->user = g_string_new(srp_user_get_username(c->srp));
	}
}

MSC_BOOL msc_client_pump_messages(msc_client *c, msc_client_message_callback msg_callback)
{
	int maxfd = msc_secure_socket_socket(c->socket);
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(msc_secure_socket_socket(c->socket), &rfds);

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1000;
	int status = select(maxfd + 1, &rfds, NULL, NULL, &tv);
	if ((status > 0) && FD_ISSET(msc_secure_socket_socket(c->socket), &rfds))
	{
		GString *buf = g_string_new("");
		if (msc_secure_socket_read(c->socket, buf) != MSC_SOCKET_ERR_EMPTY_READ)
		{
			GString *message = NULL;
			size_t message_len = 0;
			GString *from = NULL;
			size_t from_len = 0;
			GString *from_host = NULL;
			size_t from_host_len = 0;
			char seed[4] = { 0 };

			// parse message and call callback if required
			// also need to continue authentication handshake here when appropriate
			char *p = buf->str;
			char *end = buf->str + buf->len;

			char opcode = *(p++);
			++p; // ignore auth param

			// parse field lengths
			switch (opcode)
			{
			case MSC_SERVER_MSG_OPCODE_SERVER:
			case MSC_SERVER_MSG_OPCODE_CHATROOM:
			case MSC_SERVER_MSG_OPCODE_PRIVATE:
				message_len = (size_t)(*(p++) << 8);
				message_len = (size_t)(message_len | *(p++));
				message_len = (message_len > 512) ? 512 : message_len;
				message_len = (message_len > (end - p)) ? (end - p) : message_len;
				break;

			case MSC_SERVER_MSG_OPCODE_AUTH1:
				memcpy(seed, p, 4);
				p += 4;

				message_len = (size_t)(*(p++) << 8);
				message_len = (size_t)(message_len | *(p++));
				message_len = (message_len > 1024) ? 1024 : message_len;
				message_len = (message_len > (end - p)) ? (end - p) : message_len;
				break;

			case MSC_SERVER_MSG_OPCODE_AUTH2:
				message_len = (size_t)*(p++);
				message_len = (message_len > 64) ? 64 : message_len;
				break;

			default:
				return MSC_FALSE;
			}

			if ((opcode == MSC_SERVER_MSG_OPCODE_CHATROOM) || (opcode == MSC_SERVER_MSG_OPCODE_PRIVATE))
			{
				from_len = (size_t)*(p++);
				from_len = (from_len > 64) ? 64 : from_len;

				from_host_len = (size_t)*(p++);
			}

			// read packet fields
			switch (opcode)
			{
			case MSC_SERVER_MSG_OPCODE_CHATROOM:
			case MSC_SERVER_MSG_OPCODE_PRIVATE:
				from = g_string_new_len(p, from_len);
				p += from_len;

				from_host = g_string_new_len(p, from_host_len);
				p += from_host_len;

			case MSC_SERVER_MSG_OPCODE_SERVER:
			case MSC_SERVER_MSG_OPCODE_AUTH1:
			case MSC_SERVER_MSG_OPCODE_AUTH2:
				message = g_string_new_len(p, message_len);
				p += message_len;
				break;

			default:
				break;
			}

			// dispatch callbacks
			switch (opcode)
			{
			case MSC_SERVER_MSG_OPCODE_SERVER:
				msg_callback(MSC_TRUE, MSC_FALSE, NULL, NULL, message);
				break;

			case MSC_SERVER_MSG_OPCODE_CHATROOM:
				msg_callback(MSC_FALSE, MSC_FALSE, from, from_host, message);
				break;

			case MSC_SERVER_MSG_OPCODE_PRIVATE:
				msg_callback(MSC_FALSE, MSC_TRUE, from, from_host, message);
				break;

			case MSC_SERVER_MSG_OPCODE_AUTH1:
				if (c->srp) msc_client_handle_auth1(c, seed, 4, message);
				break;

			case MSC_SERVER_MSG_OPCODE_AUTH2:
				if (c->srp) msc_client_handle_auth2(c, message);
				break;

			default:
				break;
			}

			if (message) g_string_free(message, TRUE);
			if (from) g_string_free(from, TRUE);
			if (from_host) g_string_free(from_host, TRUE);
		}

		// might want to do while SSL_pending here

		g_string_free(buf, TRUE);
		return MSC_TRUE;
	}

	return MSC_FALSE;
}

GString *msc_client_authenticated_user_name(msc_client *c)
{
	return c->user;
}

GString *msc_client_chatroom(msc_client *c)
{
	return c->chatroom;
}

MSC_CLIENT_ERR msc_client_authenticate(msc_client *c, const GString *user, const GString *passwd)
{
	if ((user->len > 64) || (user->len < 1)) return MSC_CLIENT_ERR_NOT_IMPLEMENTED;
	if (c->srp) srp_user_delete(c->srp);

	c->srp = srp_user_new(SRP_SHA256, SRP_NG_2048, user->str, (const unsigned char *)passwd->str, (int)passwd->len, NULL, NULL);

	const char *ignore = NULL;
	const unsigned char *a = NULL;
	int a_len = 0;
	srp_user_start_authentication(c->srp, &ignore, &a, &a_len);

	char header_buf[] = { MSC_CLIENT_MSG_OPCODE_AUTH, 0x00, (char)(user->len & 0xff), (char)((a_len >> 8) & 0xff), (char)(a_len & 0xff) };

	GString *buf = g_string_new_len(header_buf, 5);
	buf = g_string_append_len(buf, user->str, user->len);
	buf = g_string_append_len(buf, (char *)a, a_len);

	msc_secure_socket_write(c->socket, buf);

	g_string_free(buf, TRUE);
	return MSC_CLIENT_ERR_NO_ERROR;
}

MSC_CLIENT_ERR msc_client_join_chatroom(msc_client *c, const GString *chatroom)
{
	if (chatroom->len < 1) return MSC_CLIENT_ERR_CHATROOM_LEN;
	int actual_len = (int)min(64, chatroom->len);

	if (c->chatroom) g_string_free(c->chatroom, TRUE);
	c->chatroom = g_string_new_len(chatroom->str, actual_len);

	char header_buf[] = { MSC_CLIENT_MSG_OPCODE_JOIN, (char)(actual_len & 0xff) };

	GString *buf = g_string_new_len(header_buf, 2);
	buf = g_string_append_len(buf, chatroom->str, actual_len);

	msc_secure_socket_write(c->socket, buf);

	g_string_free(buf, TRUE);
	return MSC_CLIENT_ERR_NO_ERROR;
}

MSC_CLIENT_ERR msc_client_list_chatrooms(msc_client *c)
{
	char buf[] = { MSC_CLIENT_MSG_OPCODE_LIST };
	GString *msg = g_string_new_len(buf, 1);
	msc_secure_socket_write(c->socket, msg);
	g_string_free(msg, TRUE);
	return MSC_CLIENT_ERR_NO_ERROR;
}

MSC_CLIENT_ERR msc_client_list_users(msc_client *c)
{
	char buf[] = { MSC_CLIENT_MSG_OPCODE_WHO };
	GString *msg = g_string_new_len(buf, 1);
	msc_secure_socket_write(c->socket, msg);
	g_string_free(msg, TRUE);
	return MSC_CLIENT_ERR_NO_ERROR;
}

MSC_CLIENT_ERR msc_client_say(msc_client *c, const GString *target, const GString *msg)
{
	char header_buf[] = { 0, 0, 0, 0 };

	GString *buf = NULL;
	if (target)
	{
		header_buf[0] = MSC_CLIENT_MSG_OPCODE_SAY;
		header_buf[1] = (char)(min(64, target->len));

		int transmit_len = (int)min(512, msg->len);
		header_buf[2] = (char)((transmit_len >> 8) & 0xff);
		header_buf[3] = (char)( transmit_len       & 0xff);

		buf = g_string_new_len(header_buf, 4);
		buf = g_string_append_len(buf, target->str, min(64, target->len));
	}
	else
	{
		header_buf[0] = MSC_CLIENT_MSG_OPCODE_MESSAGE;

		int transmit_len = (int)min(512, msg->len);
		header_buf[1] = (char)((transmit_len >> 8) & 0xff);
		header_buf[2] = (char)( transmit_len       & 0xff);

		buf = g_string_new_len(header_buf, 3);
	}
	buf = g_string_append_len(buf, msg->str, min(512, msg->len));

	MSC_SOCKET_ERR e = msc_secure_socket_write(c->socket, buf);
	g_string_free(buf, TRUE);

	if (e)
	{
		return MSC_CLIENT_ERR_NOT_IMPLEMENTED;
	}

	return MSC_CLIENT_ERR_NO_ERROR;
}

MSC_CLIENT_ERR msc_client_game_start(msc_client *c, const GString *target)
{
	if (target->len < 1) return MSC_CLIENT_ERR_USERNAME_LEN;
	int actual_len = (int)min(64, target->len);

	if (c->chatroom) g_string_free(c->chatroom, TRUE);
	c->chatroom = g_string_new_len(target->str, actual_len);

	char header_buf[] = { MSC_CLIENT_MSG_OPCODE_GAME, (char)(actual_len & 0xff) };

	GString *buf = g_string_new_len(header_buf, 2);
	buf = g_string_append_len(buf, target->str, actual_len);

	msc_secure_socket_write(c->socket, buf);

	g_string_free(buf, TRUE);
	return MSC_CLIENT_ERR_NO_ERROR;
}

MSC_CLIENT_ERR msc_client_game_roll(msc_client *c)
{
	char buf[] = { MSC_CLIENT_MSG_OPCODE_ROLL };
	GString *msg = g_string_new_len(buf, 1);
	msc_secure_socket_write(c->socket, msg);
	g_string_free(msg, TRUE);
	return MSC_CLIENT_ERR_NO_ERROR;
}
