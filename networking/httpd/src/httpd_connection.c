#include "httpd_connection.h"
#include "httpd_common.h"
#include "httpd_req_parser.h"
#include "httpd_req_handler.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <glib.h>

#define min(x, y) (((x) < (y)) ? (x) : (y))

#define RECEIVE_BUFFER_SIZE (5 * 1024 * 1024)
#define CONNECTION_TIMEOUT ((float)30.0)

typedef struct _connection {
	int socket;
	struct sockaddr_in client_addr;
	socklen_t client_len;

	float timeout;
	float timeout_threshold;
	gboolean persist_connection;
	req_parser *parser;
	req_handler *handler;
	FILE *log_file;
} connection;

int connection_send(connection *c, char *data, size_t n);

connection *connection_create(FILE *log_file)
{
	connection *c = (connection *)malloc(sizeof(connection));
	if (!c)
	{
		halt_error("Unable to allocate connection structure");
		return NULL;
	}

	c->socket = 0;
	memset(&c->client_addr, 0, sizeof(c->client_addr));
	c->client_len = sizeof(c->client_addr);

	c->timeout = 0;
	c->timeout_threshold = CONNECTION_TIMEOUT;
	c->persist_connection = TRUE;
	c->parser = req_parser_create();
	c->handler = req_handler_create(c, c->parser);
	c->log_file = log_file;

	return c;
}

void connection_destroy(connection *c)
{
	if (!c) return;

	connection_close(c);
	req_parser_destroy(c->parser);
	req_handler_destroy(c->handler);

	free(c);
}

int connection_socket(connection *c)
{
	return c->socket;
}

int connection_timedout(connection *c)
{
	return c->timeout >= c->timeout_threshold;
}

int connection_is_persistent(connection *c)
{
	return c->persist_connection;
}

GString *connection_client_ip(connection *c)
{
	char addr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(c->client_addr.sin_addr), addr, INET_ADDRSTRLEN);

	return g_string_new(addr);
}

int connection_client_port(connection *c)
{
	return ntohs(c->client_addr.sin_port);
}

void connection_accept(connection *c, int listenfd)
{
	connection_close(c);

	c->socket = accept(listenfd, (struct sockaddr *)&c->client_addr, &c->client_len);
	if (c->socket < 0)
	{
		if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
		{
			c->socket = 0;
			connection_close(c);
		}
		else
		{
			halt_error("Unable to accept connection");
		}
		return;
	}

	req_parser_reset(c->parser);
}

void connection_close(connection *c)
{
	if (!c) return;

	if (c->socket != 0) close(c->socket);
	c->socket = 0;

	memset(&c->client_addr, '0', sizeof(c->client_addr));
	c->client_len = sizeof(c->client_addr);

	c->timeout = 0;
}

static void set_persistence(connection *c)
{
	GString *connection_header = NULL;
	if (req_parser_headers(c->parser))
	{
		connection_header = g_tree_lookup(req_parser_headers(c->parser), g_intern_static_string("Connection"));
	}

	if (connection_header)
	{
		if (strncmp(connection_header->str, "close", min(connection_header->len, 5)) == 0)
			c->persist_connection = FALSE;
		else if (strncmp(connection_header->str, "keep-alive", min(connection_header->len, 10)) == 0)
			c->persist_connection = TRUE;
	}
}

static void write_logline(connection *c, int response)
{
	char timebuf[512];
	memset(timebuf, 0, 512);

	time_t t = time(NULL);
	struct tm *tmp = localtime(&t);
	strftime(timebuf, 511, "%Y-%m-%dT%H:%M:%S", tmp);

	char addr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(c->client_addr.sin_addr), addr, INET_ADDRSTRLEN);

	const char *method = NULL;
	switch (req_parser_method(c->parser))
	{
	case REQ_PARSER_METHOD_GET:
		method = "GET";
		break;

	case REQ_PARSER_METHOD_HEAD:
		method = "HEAD";
		break;

	case REQ_PARSER_METHOD_POST:
		method = "POST";
		break;

	case REQ_PARSER_METHOD_OPTIONS:
		break;

	default:
		method = "???";
		break;
	}

	GString *path = NULL;
	if (req_parser_path(c->parser))
	{
		GString *tmp = req_parser_path(c->parser);
		path = g_string_new_len(tmp->str, tmp->len);
	}
	else
	{
		path = g_string_new(NULL);
	}

	fprintf(c->log_file, "%s : %s:%d %s %s : %d\n", timebuf, addr, ntohs(c->client_addr.sin_port), method, path->str, response);

	g_string_free(path, TRUE);
}

int connection_receive(connection *c)
{
	c->timeout = 0;

	char buf[RECEIVE_BUFFER_SIZE];
	ssize_t buf_size = recvfrom(c->socket, buf, RECEIVE_BUFFER_SIZE, 0, (struct sockaddr *)&c->client_addr, &c->client_len);

	if (buf_size <= 0)
	{
		connection_close(c);
		return 1;
	}

	switch (req_parser_add_text(c->parser, buf, (size_t)buf_size))
	{
	case REQ_PARSER_PARSE_OK:
		// FALLTHROUGH
	case REQ_PARSER_PARSE_INVALID_REQUEST:
		set_persistence(c);

		write_logline(c, req_handler_handle_request(c->handler));
		req_parser_reset(c->parser);
		break;

	default:
		// Do I really want to do anything here?
		break;
	}

	if (!c->persist_connection)
	{
		connection_close(c);
		return 1;
	}

	return 0;
}

int connection_send(connection *c, char *data, size_t n)
{
	size_t rem = n;
	while (rem)
	{
		ssize_t sent = sendto(c->socket, data, rem, MSG_NOSIGNAL, (const struct sockaddr *)&c->client_addr, c->client_len);
		if (sent < 0) return 1;

		rem -= sent;
		data += sent;
	}

	return 0;
}

void connection_timeout_increment(connection *c, float seconds)
{
	c->timeout += seconds;
}
