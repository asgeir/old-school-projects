#include "msc_server.h"
#include "msc_secure_socket.h"
#include "msc_server_connection.h"

#include <sqlite3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define TIMEOUT_SEC  0
#define TIMEOUT_USEC (500 * 1000)

typedef struct _msc_server {
	int socket;
	struct sockaddr_in addr;
	int backlog;

	GQueue *free_connections;
	GList *connections;
	FILE *log_file;

	GTree *channels;
	int next_guest;

	sqlite3 *passwd_db;
} msc_server;

static gint msc_server_channel_tree_cmp(gconstpointer a, gconstpointer b)
{
	return a - b;
}

static void write_logline(msc_server *s, msc_server_connection *c, const char *msg)
{
	char timebuf[512];
	memset(timebuf, 0, 512);

	time_t t = time(NULL);
	struct tm *tmp = localtime(&t);
	strftime(timebuf, 511, "%Y-%m-%dT%H:%M:%S", tmp);

	GString *remote_host = msc_server_connection_remote_hostname(c);
	uint16_t port = msc_server_connection_remote_port(c);

	fprintf(s->log_file, "%s : %s:%d %s\n", timebuf, remote_host->str, port, msg);
	fprintf(stderr, "%s : %s:%d %s\n", timebuf, remote_host->str, port, msg);

	g_string_free(remote_host, TRUE);
}

static void msc_server_setup_db(sqlite3 *db)
{
	char *zErrMsg = NULL;
	if (sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS users (name VARCHAR PRIMARY KEY, s VARCHAR, v VARCHAR);", NULL, 0, &zErrMsg) != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
}

const char *msc_server_err_desc(MSC_SERVER_ERR err)
{
	switch (err)
	{
	case MSC_SERVER_ERR_NO_ERROR:
		return "No error";

	case MSC_SERVER_ERR_BIND:
		return "Error binding socket";

	case MSC_SERVER_ERR_LISTEN:
		return "Error listening to socket";

	case MSC_SERVER_ERR_NOT_IMPLEMENTED:
		return "Function not implemented";

	default:
		return "Invalid server error";
	}
}

msc_server *msc_server_create(uint16_t port, const char *cert_file, const char *key_file, int backlog)
{
	if (!msc_secure_socket_ctx_load_cert(cert_file, key_file)) return NULL;

	msc_server *s = (msc_server *)malloc(sizeof(msc_server));
	if (s)
	{
		s->socket = socket(AF_INET, SOCK_STREAM, 0);
		if (s->socket < 0)
		{
			free(s);
			halt_os_error("Unable to create socket");
			return NULL;
		}

		if (sqlite3_open("chatd_passwd.db", &s->passwd_db))
		{
			free(s);
			fatal_error("Unable to open password db");
		}

		msc_server_setup_db(s->passwd_db);

		setsockopt(s->socket, SOL_SOCKET, SO_REUSEADDR, (void *)1, sizeof(int));
		setsockopt(s->socket, SOL_SOCKET, SO_REUSEPORT, (void *)1, sizeof(int));

		memset(&s->addr, 0, sizeof(s->addr));

		s->addr.sin_family = AF_INET;
		s->addr.sin_addr.s_addr = htonl(INADDR_ANY);
		s->addr.sin_port = htons(port);

		s->backlog = backlog;

		s->log_file = fopen("chatd.log", "w");
		if (s->log_file == NULL)
		{
			halt_os_error("Unable to create logfile 'chatd.log'");
		}

		s->free_connections = g_queue_new();
		int i;
		for (i = 0; i < backlog; ++i)
		{
			g_queue_push_tail(s->free_connections, msc_server_connection_create(s));
		}
		s->connections = NULL;

		s->channels = g_tree_new(msc_server_channel_tree_cmp);
		s->next_guest = 0;
	}

	return s;
}

static void msc_server_destroy_connection(gpointer data)
{
	msc_server_connection_close((msc_server_connection *)data);
	msc_server_connection_destroy((msc_server_connection *)data);
}

static gboolean msc_server_free_channel_list(gpointer key, gpointer value, gpointer data)
{
	(void)key;   // key is a quark so glib manages it
	(void)data;  // data is not used

	// the connections aren't owned by the list so they don't need to be free'd
	g_list_free((GList *)value);
	return FALSE;
}

void msc_server_destroy(msc_server *s)
{
	if (!s) return;

	msc_server_close(s);

	if (s->channels)
	{
		g_tree_foreach(s->channels, msc_server_free_channel_list, NULL);
		g_tree_destroy(s->channels);
	}
	s->channels = NULL;

	if (s->free_connections) g_queue_free_full(s->free_connections, msc_server_destroy_connection);
	s->free_connections = NULL;

	if (s->connections) g_list_free_full(s->connections, msc_server_destroy_connection);
	s->connections = NULL;

	if (s->log_file) fclose(s->log_file);
	s->log_file = NULL;

	sqlite3_close(s->passwd_db);

	free(s);
}

msc_server_connection *msc_server_user(msc_server *s, GString *user)
{
	GList *i = s->connections;
	while (i != NULL)
	{
		if (g_string_equal(user, msc_server_connection_user((msc_server_connection *)i->data)))
		{
			return (msc_server_connection *)i->data;
		}
		i = i->next;
	}
	return NULL;
}

msc_server_user_auth_info *msc_server_user_get_auth_info(msc_server *s, GString *user)
{
	msc_server_user_auth_info *info = NULL;

	sqlite3_stmt *stmt = NULL;
	const char *select_query = "SELECT s, v FROM users WHERE name = ? LIMIT 1;";
	if (sqlite3_prepare_v2(s->passwd_db, select_query, (int)strlen(select_query), &stmt, NULL) != SQLITE_OK)
	{
		fprintf(stderr, "Unable to prepare SQL statement.\n");
	}
	else
	{
		sqlite3_bind_text(stmt, 1, user->str, (int)user->len, NULL);

		if (sqlite3_step(stmt) != SQLITE_ROW)
		{
			fprintf(stderr, "Unable to read user from database");
		}
		else
		{
			const unsigned char *s_b64 = sqlite3_column_text(stmt, 0);
			const unsigned char *v_b64 = sqlite3_column_text(stmt, 1);

			size_t s_len = 0;
			guchar *s_raw = g_base64_decode((const gchar *)s_b64, &s_len);

			size_t v_len = 0;
			guchar *v_raw = g_base64_decode((const gchar *)v_b64, &v_len);

			info = (msc_server_user_auth_info *)malloc(sizeof(msc_server_user_auth_info));
			info->s = g_string_new_len((char *)s_raw, s_len);
			info->v = g_string_new_len((char *)v_raw, v_len);

			g_free(s_raw);
			g_free(v_raw);
		}

		sqlite3_finalize(stmt);
	}

	return info;
}

typedef struct {
	msc_server_chatroom_callback cb;
	void *data;
} msc_server_traverse_data;

static gboolean msc_server_traverse_chatrooms(gpointer key, gpointer value, gpointer data)
{
	(void)value;
	msc_server_traverse_data *d = (msc_server_traverse_data *)data;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
	d->cb((GQuark)key, d->data);
#pragma GCC diagnostic pop

	return FALSE;
}

void msc_server_foreach_chatroom(msc_server *s, msc_server_chatroom_callback callback, void *data)
{
	msc_server_traverse_data tmp = { callback, data };
	g_tree_foreach(s->channels, msc_server_traverse_chatrooms, &tmp);
}

void msc_server_foreach_user(msc_server *s, msc_server_user_callback callback, void *data)
{
	GList *i = s->connections;
	while (i != NULL)
	{
		callback((msc_server_connection *)i->data, data);
		i = i->next;
	}
}

void msc_server_foreach_user_in_chatroom(msc_server *s, GQuark chatroom, msc_server_user_callback callback, void *data)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
	GList *user_list = (GList *)g_tree_lookup(s->channels, (gconstpointer)chatroom);
#pragma GCC diagnostic pop

	while (user_list != NULL)
	{
		callback((msc_server_connection *)user_list->data, data);
		user_list = user_list->next;
	}
}

void msc_server_join_chatroom(msc_server *s, GQuark chatroom, msc_server_connection *c)
{
	if (!chatroom || !c) return;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
	GList *user_list = (GList *)g_tree_lookup(s->channels, (gconstpointer)chatroom);
#pragma GCC diagnostic pop

	user_list = g_list_append(user_list, c);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
	g_tree_insert(s->channels, (gpointer)chatroom, user_list);
#pragma GCC diagnostic pop
}

void msc_server_leave_chatroom(msc_server *s, GQuark chatroom, msc_server_connection *c)
{
	if (!chatroom || !c) return;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
	GList *user_list = (GList *)g_tree_lookup(s->channels, (gconstpointer)chatroom);
#pragma GCC diagnostic pop

	user_list = g_list_remove_all(user_list, c);

	if (user_list)
	{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
		g_tree_insert(s->channels, (gpointer)chatroom, user_list);
#pragma GCC diagnostic pop
	}
	else
	{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
		g_tree_remove(s->channels, (gconstpointer)chatroom);
#pragma GCC diagnostic pop
	}
}

void msc_server_log_authenticated(msc_server *s, msc_server_connection *c)
{
	GString *tmp = g_string_new("");
	g_string_printf(tmp, "%s authenticated", msc_server_connection_user(c)->str);
	write_logline(s, c, tmp->str);
	g_string_free(tmp, TRUE);
}

void msc_server_log_authentication_failed(msc_server *s, msc_server_connection *c, const char *failed_user)
{
	GString *tmp = g_string_new("");
	g_string_printf(tmp, "%s authentication error", failed_user);
	write_logline(s, c, tmp->str);
	g_string_free(tmp, TRUE);
}

MSC_SERVER_ERR msc_server_listen(msc_server *s)
{
	if (bind(s->socket, (struct sockaddr*)&s->addr, sizeof(s->addr)) < 0)
	{
		return MSC_SERVER_ERR_BIND;
	}

	if (listen(s->socket, s->backlog) < 0)
	{
		return MSC_SERVER_ERR_LISTEN;
	}

	return MSC_SERVER_ERR_NO_ERROR;
}

void msc_server_close(msc_server *s)
{
	if (!s) return;

	GList *i = s->connections;
	while (i != NULL)
	{
		GList *next = i->next;

		msc_server_connection *c = (msc_server_connection *)i->data;
		msc_server_connection_close(c);
		i->data = NULL;

		s->connections = g_list_delete_link(s->connections, i);
		g_queue_push_tail(s->free_connections, c);

		i = next;
	}

	close(s->socket);
	s->socket = 0;

	memset(&s->addr, '0', sizeof(s->addr));
}

void msc_server_wait(msc_server *s)
{
	int maxfd = s->socket;
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(s->socket, &rfds);

	GList *i;
	for (i = s->connections; i != NULL; i = i->next)
	{
		msc_server_connection *c = (msc_server_connection *)i->data;
		int fd = msc_server_connection_socket_handle(c);
		if (fd > maxfd) maxfd = fd;
		FD_SET(fd, &rfds);
	}

	struct timeval tv;
	tv.tv_sec = TIMEOUT_SEC;
	tv.tv_usec = TIMEOUT_USEC;
	int status = select(maxfd + 1, &rfds, NULL, NULL, &tv);

	if (status > 0)
	{
		if (FD_ISSET(s->socket, &rfds))
		{
			if (!g_queue_is_empty(s->free_connections))
			{
				msc_server_connection *c = (msc_server_connection *)g_queue_pop_head(s->free_connections);
				s->connections = g_list_append(s->connections, c);

				msc_server_connection_accept(c, ++s->next_guest, s->socket);
				if (msc_server_connection_socket_handle(c) != 0)
				{
					write_logline(s, c, "connected");
				}
			}
		}

		GList *j = s->connections;
		while (j != NULL)
		{
			GList *next = j->next;

			msc_server_connection *c = (msc_server_connection *)j->data;
			if (msc_server_connection_socket_handle(c) == 0)
			{
				j->data = NULL;
				s->connections = g_list_delete_link(s->connections, j);
				g_queue_push_tail(s->free_connections, c);
			}
			else if (FD_ISSET(msc_server_connection_socket_handle(c), &rfds))
			{
				if (msc_server_connection_receive(c))
				{
					j->data = NULL;
					s->connections = g_list_delete_link(s->connections, j);
					g_queue_push_tail(s->free_connections, c);

					write_logline(s, c, "disconnected");
					msc_server_connection_close(c);
				}
			}

			j = next;
		}
	}
	else if (status == 0)
	{
//		// connection timeout
//		GList *j = s->connections;
//		while (j != NULL)
//		{
//			GList *next = j->next;
//
//			msc_server_connection *c = (msc_server_connection *)j->data;
//			msc_server_connection_timeout_increment(c, TIMEOUT_SEC + (((float)TIMEOUT_USEC) / 1000000.0f));
//
//			if (msc_server_connection_timedout(c))
//			{
//				j->data = NULL;
//				s->connections = g_list_delete_link(s->connections, j);
//				g_queue_push_tail(s->free_connections, c);
//			}
//
//			j = next;
//		}
	}
	else
	{
		// TODO: Don't know if this is ever a fatal error
		//halt_error("Error in socket select");
		return;
	}
}
