#include "httpd_server.h"
#include "httpd_common.h"
#include "httpd_connection.h"

#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <glib.h>

#define CONNECTION_BACKLOG 10

#define TIMEOUT_SEC  0
#define TIMEOUT_USEC (500 * 1000)

typedef struct _server {
	int socket;
	struct sockaddr_in addr;

	GQueue *free_connections;
	GList *connections;
	FILE *log_file;
} server;

void server_destroy_connection(gpointer data)
{
	connection_close((connection *)data);
	connection_destroy((connection *)data);
}

server *server_create(uint16_t port)
{
	server *s = (server *)malloc(sizeof(server));
	if (!s)
	{
		halt_error("Unable to allocate server structure");
		return NULL;
	}

	// SOCK_STREAM|SOCK_NONBLOCK
	s->socket = socket(AF_INET, SOCK_STREAM, 0);
	if (s->socket < 0)
	{
		halt_error("Unable to create server socket");
		return NULL;
	}

	setsockopt(s->socket, SOL_SOCKET, SO_REUSEADDR, (void *)1, sizeof(int));
	setsockopt(s->socket, SOL_SOCKET, SO_REUSEPORT, (void *)1, sizeof(int));

	memset(&s->addr, 0, sizeof(s->addr));

	s->addr.sin_family = AF_INET;
	s->addr.sin_addr.s_addr = htonl(INADDR_ANY);
	s->addr.sin_port = htons(port);

	s->log_file = fopen("httpd.log", "w");
	if (s->log_file == NULL)
	{
		halt_error("Unable to create logfile 'httpd.log'");
	}

	s->free_connections = g_queue_new();
	int i;
	for (i = 0; i < CONNECTION_BACKLOG; ++i)
	{
		g_queue_push_tail(s->free_connections, connection_create(s->log_file));
	}
	s->connections = NULL;

	return s;
}

void server_destroy(server *s)
{
	if (!s) return;

	server_close(s);

	g_queue_free_full(s->free_connections, server_destroy_connection);
	s->free_connections = NULL;

	if (s->connections) g_list_free_full(s->connections, server_destroy_connection);
	s->connections = NULL;

	fclose(s->log_file);
	s->log_file = NULL;

	free(s);
}

void server_listen(server *s)
{
	if (bind(s->socket, (struct sockaddr*)&s->addr, sizeof(s->addr)) < 0)
	{
		halt_error("Could not bind to socket");
	}

	if (listen(s->socket, CONNECTION_BACKLOG) < 0)
	{
		halt_error("Could not start listening on socket");
	}
}

void server_close(server *s)
{
	if (!s) return;

	GList *i = s->connections;
	while (i != NULL)
	{
		GList *next = i->next;

		connection *c = (connection *)i->data;
		connection_close(c);
		i->data = NULL;

		s->connections = g_list_delete_link(s->connections, i);
		g_queue_push_tail(s->free_connections, c);

		i = next;
	}

	close(s->socket);
	s->socket = 0;

	memset(&s->addr, '0', sizeof(s->addr));
}

void server_wait(server *s)
{
	int maxfd = s->socket;
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(s->socket, &rfds);

	GList *i;
	for (i = s->connections; i != NULL; i = i->next)
	{
		connection *c = (connection *)i->data;
		int fd = connection_socket(c);
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
				connection *c = (connection *)g_queue_pop_head(s->free_connections);
				s->connections = g_list_append(s->connections, c);

				connection_accept(c, s->socket);
				if (connection_socket(c) == 0) halt_error("Socket accept really shouldn't fail");
			}
		}

		GList *j = s->connections;
		while (j != NULL)
		{
			GList *next = j->next;

			connection *c = (connection *)j->data;
			if (FD_ISSET(connection_socket(c), &rfds))
			{
				if (connection_receive(c))
				{
					j->data = NULL;
					s->connections = g_list_delete_link(s->connections, j);
					g_queue_push_tail(s->free_connections, c);
				}
			}

			j = next;
		}
	}
	else if (status == 0)
	{
		// connection timeout
		GList *j = s->connections;
		while (j != NULL)
		{
			GList *next = j->next;

			connection *c = (connection *)j->data;
			connection_timeout_increment(c, TIMEOUT_SEC + (((float)TIMEOUT_USEC) / 1000000.0f));

			if (connection_timedout(c))
			{
				j->data = NULL;
				s->connections = g_list_delete_link(s->connections, j);
				g_queue_push_tail(s->free_connections, c);
			}

			j = next;
		}
	}
	else
	{
		// TODO: Don't know if this is ever a fatal error
		//halt_error("Error in socket select");
		return;
	}
}
