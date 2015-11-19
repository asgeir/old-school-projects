#include "httpd_req_handler.h"
#include "httpd_req_parser.h"
#include "httpd_connection.h"
#include "httpd_helpers.h"

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define min(x, y) (((x) < (y)) ? (x) : (y))

typedef struct _status_message {
	int number;
	const char *message;
} status_message;

static status_message status_messages[] = {
	{ 200, "200 OK" },
	{ 201, "201 Created" },
	{ 202, "202 Accepted" },
	{ 203, "203 Non-Authoritative Information" },
	{ 204, "204 No Content" },
	{ 205, "205 Reset Content" },
	{ 206, "206 Partial Content" },
	{ 300, "300 Multiple Choices" },
	{ 301, "301 Moved Permanently" },
	{ 302, "302 Found" },
	{ 303, "303 See Other" },
	{ 304, "304 Not Modified" },
	{ 305, "305 Use Proxy" },
	{ 307, "307 Temporary Redirect" },
	{ 400, "400 Bad Request" },
	{ 401, "401 Unauthorized" },
	{ 402, "402 Payment Required" },
	{ 403, "403 Forbidden" },
	{ 404, "404 Not Found" },
	{ 405, "405 Method Not Allowed" },
	{ 406, "406 Not Acceptable" },
	{ 407, "407 Proxy Authentication Required" },
	{ 408, "408 Request Timeout" },
	{ 409, "409 Conflict" },
	{ 410, "410 Gone" },
	{ 411, "411 Length Required" },
	{ 412, "412 Precondition Failed" },
	{ 413, "413 Request Entity Too Large" },
	{ 414, "414 Request-URI Too Long" },
	{ 415, "415 Unsupported Media Type" },
	{ 416, "416 Requested Range Not Satisfiable" },
	{ 417, "417 Expectation Failed" },
	{ 500, "500 Internal Server Error" },
	{ 501, "501 Not Implemented" },
	{ 502, "502 Bad Gateway" },
	{ 503, "503 Service Unavailable" },
	{ 504, "504 Gateway Timeout" },
	{ 505, "505 HTTP Version Not Supported" },
	{ 0, NULL }
};

typedef struct _req_handler
{
	connection *conn;
	req_parser *req;
} req_handler;

req_handler *req_handler_create(connection *c, req_parser *p)
{
	req_handler *r = (req_handler *) malloc(sizeof(req_handler));

	r->conn = c;
	r->req = p;

	return r;
}

void req_handler_destroy(req_handler *r)
{
	free(r);
}

typedef GString *(*BodyFunction)(gpointer user_data);

static gboolean format_header(gpointer key, gpointer value, gpointer data)
{
	GString *data_str = (GString *)data;
	g_string_append_printf(data_str, "%s: %s\r\n", (const char *)key, ((GString *)value)->str);
	return FALSE;
}

static void append_status(GString *reply, int status)
{
	status_message *i;
	for (i = status_messages; i->number; ++i)
	{
		if (i->number == status)
		{
			g_string_append_printf(reply, "HTTP/1.1 %s\r\n", i->message);
			return;
		}
	}

	g_string_append_printf(reply, "HTTP/1.1 500 Internal Server Error\r\n");
}

static void send_reply(req_handler *r, int status, GTree *headers, BodyFunction body_func, gpointer user_data)
{
	GString *reply = g_string_new(NULL);
	GString *body = NULL;
	gboolean free_headers = FALSE;

	append_status(reply, status);

	if (body_func)
	{
		body = body_func(user_data);
	}

	if (!headers)
	{
		headers = g_tree_new_full(quarkcmp, NULL, NULL, gstring_destroy_notify);
		free_headers = TRUE;
	}

	GString *length = g_string_new(NULL);
	if (body) g_string_append_printf(length, "%zu", body->len);
	else g_string_append_len(length, "0", 1);

	g_tree_insert(headers, (gpointer)g_intern_static_string("Content-Length"), length);
	if (connection_is_persistent(r->conn))
	{
		g_tree_insert(headers, (gpointer)g_intern_static_string("Connection"), g_string_new("keep-alive"));
	}
	else
	{
		g_tree_insert(headers, (gpointer)g_intern_static_string("Connection"), g_string_new("close"));
	}

	g_tree_foreach(headers, format_header, reply);

	if (free_headers)
	{
		g_tree_destroy(headers);
	}

	// terminate headers
	g_string_append_len(reply, "\r\n", 2);

	if (body && (req_parser_method(r->req) != REQ_PARSER_METHOD_HEAD))
	{
		g_string_append_len(reply, body->str, body->len);
	}

	if (body) g_string_free(body, TRUE);

	connection_send(r->conn, reply->str, reply->len);

	g_string_free(reply, TRUE);
}

typedef GString *(*Html5Function)(req_handler *handler);

typedef struct {
	req_handler *handler;
	Html5Function head;
	Html5Function body;
} html5_data;

static GString *html5_reply(gpointer user_data)
{
	html5_data *data = (html5_data *)user_data;

	GString *head = NULL;
	if (data->head) head = data->head(data->handler);
	else head = g_string_new(NULL);

	GString *body = NULL;
	if (data->body) body = data->body(data->handler);
	else body = g_string_new(NULL);

	GString *addr = connection_client_ip(data->handler->conn);

	GString *req_info = g_string_new(NULL);
	g_string_printf(req_info,
		"<ul>"
			"<li>Path: %s</li>"
			"<li>Client: %s:%d</li>"
		"</ul>", req_parser_path(data->handler->req)->str, addr->str, connection_client_port(data->handler->conn));

	GString *reply = g_string_new(NULL);
	g_string_printf(reply,
		"<!doctype html>"
		"<html>"
			"<head>"
				"<meta charset=\"UTF-8\">"
				"%s"
			"</head>"
			"<body>"
				"%s"
				"%s"
			"</body>"
		"</html>", head->str, body->str, req_info->str);

	g_string_free(head, TRUE);
	g_string_free(body, TRUE);
	g_string_free(req_info, TRUE);
	g_string_free(addr, TRUE);

	return reply;
}

static GString *html5_empty(req_handler *handler)
{
	(void)handler;
	return g_string_new(NULL);
}

static GString *html5_default_post_body(req_handler *handler)
{
	GString *req_body = req_parser_body(handler->req);
	return g_string_new_len(req_body->str, req_body->len);
}

static gboolean query_to_list(gpointer key, gpointer value, gpointer data)
{
	GString *body = (GString *)data;
	g_string_append_printf(body, "<li>%s: %s</li>", (const char *)key, ((GString *)value)->str);
	return FALSE;
}

static GString *html5_list_query_body(req_handler *handler)
{
	GString *body = g_string_new("<p>Query data:</p><ul>");
	g_tree_foreach(req_parser_query(handler->req), query_to_list, body);
	g_string_append_len(body, "</ul>", 5);
	return body;
}

static GString *html5_list_headers_body(req_handler *handler)
{
	GString *body = g_string_new("<p>Header data:</p><ul>");
	g_tree_foreach(req_parser_headers(handler->req), query_to_list, body);
	g_string_append_len(body, "</ul>", 5);
	return body;
}

static GString *color_from_req(req_parser *r)
{
	GString *color = NULL;
	if (req_parser_query(r))
	{
		color = (GString *)g_tree_lookup(req_parser_query(r), g_intern_static_string("bg"));
	}

	if (!color && req_parser_headers(r))
	{
		color = (GString *)g_tree_lookup(req_parser_headers(r), g_intern_static_string("Cookie"));
	}

	if (color) return g_string_new_len(color->str, color->len);

	return g_string_new("white");
}

static GString *html5_color_head(req_handler *handler)
{
	GString *body = g_string_new(NULL);
	GString *color = color_from_req(handler->req);
	if (!color)
	{
		color = g_string_new("white");
	}

	g_string_append_printf(body,
		"<style>"
			"body {"
				"background-color: %s;"
			"}"
		"</style>", color->str);

	g_string_free(color, TRUE);

	return body;
}

int req_handler_handle_request(req_handler *r)
{
	if (req_parser_status(r->req) == REQ_PARSER_PARSE_INVALID_REQUEST)
	{
		send_reply(r, HTTP_STATUS_BAD_REQUEST, NULL, NULL, NULL);
		return HTTP_STATUS_BAD_REQUEST;
	}

	html5_data default_handler = {
		.handler = r,
		.head = html5_empty,
		.body = html5_empty
	};

	html5_data default_post_handler = {
		.handler = r,
		.head = html5_empty,
		.body = html5_default_post_body
	};

	html5_data query_handler = {
		.handler = r,
		.head = html5_empty,
		.body = html5_list_query_body
	};

	html5_data header_handler = {
		.handler = r,
		.head = html5_empty,
		.body = html5_list_headers_body
	};

	html5_data color_handler = {
		.handler = r,
		.head = html5_color_head,
		.body = html5_empty
	};

	int method = req_parser_method(r->req);
	GString *path = req_parser_path(r->req);

	GTree *headers = g_tree_new_full(quarkcmp, NULL, NULL, gstring_destroy_notify);
	g_tree_insert(headers, (gpointer)g_intern_static_string("Server"), g_string_new("Homework HTTPD - asgeirb09"));
	g_tree_insert(headers, (gpointer)g_intern_static_string("Content-Type"), g_string_new("text/html"));

	if (strncmp(path->str, "/test", min(5, path->len)) == 0)
	{
		send_reply(r, HTTP_STATUS_OK, headers, html5_reply, &query_handler);
	}
	else if (strncmp(path->str, "/headers", min(8, path->len)) == 0)
	{
		send_reply(r, HTTP_STATUS_OK, headers, html5_reply, &header_handler);
	}
	else if (strncmp(path->str, "/color", min(6, path->len)) == 0)
	{
		GString *color = color_from_req(r->req);
		if (color)
		{
			g_tree_insert(headers, (gpointer)g_intern_static_string("Set-Cookie"), color);
		}
		send_reply(r, HTTP_STATUS_OK, headers, html5_reply, &color_handler);
	}
	else if (method == REQ_PARSER_METHOD_POST)
	{
		send_reply(r, HTTP_STATUS_OK, headers, html5_reply, &default_post_handler);
	}
	else
	{
		send_reply(r, HTTP_STATUS_OK, headers, html5_reply, &default_handler);
	}

	g_tree_destroy(headers);

	return HTTP_STATUS_OK;
}
