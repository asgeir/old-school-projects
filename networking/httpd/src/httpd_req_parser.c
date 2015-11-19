#include "httpd_req_parser.h"
#include "httpd_helpers.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define min(x, y) (((x) < (y)) ? (x) : (y))

typedef struct _req_parser {
	int status;
	int method;
	GString *path;
	GString *fragment;
	GString *body;
	GTree *query;
	GTree *headers;
} req_parser;

req_parser *req_parser_create()
{
	req_parser *r = (req_parser *)malloc(sizeof(req_parser));

	r->status = REQ_PARSER_PARSE_INVALID_REQUEST;
	r->method = REQ_PARSER_METHOD_INVALID;
	r->path = NULL;
	r->fragment = NULL;
	r->body = NULL;
	r->query = NULL;
	r->headers = NULL;

	return r;
}

void req_parser_destroy(req_parser *r)
{
	if (!r) return;

	req_parser_reset(r);

	free(r);
}

int req_parser_status(req_parser *r)
{
	return r->status;
}

int req_parser_method(req_parser *r)
{
	return r->method;
}

GString *req_parser_path(req_parser *r)
{
	return r->path;
}

GString *req_parser_fragment(req_parser *r)
{
	return r->fragment;
}

GString *req_parser_body(req_parser *r)
{
	return r->body;
}

GTree *req_parser_query(req_parser *r)
{
	return r->query;
}

GTree *req_parser_headers(req_parser *r)
{
	return r->headers;
}

void req_parser_reset(req_parser *r)
{
	r->status = REQ_PARSER_PARSE_INVALID_REQUEST;
	r->method = REQ_PARSER_METHOD_INVALID;

	if (r->path) g_string_free(r->path, TRUE);
	r->path = NULL;

	if (r->fragment) g_string_free(r->fragment, TRUE);
	r->fragment = NULL;

	if (r->body) g_string_free(r->body, TRUE);
	r->body = NULL;

	if (r->query) g_tree_destroy(r->query);
	r->query = NULL;

	if (r->headers) g_tree_destroy(r->headers);
	r->headers = NULL;
}

#define MODE_URI_PATH     0
#define MODE_URI_QUERY    1
#define MODE_URI_FRAGMENT 2

static void parse_uri(req_parser *r, const char *data, size_t len)
{
	int mode = MODE_URI_PATH;

	const char *end = data + len;
	const char *token_start = data;
	const char *i;
	for (i = data; (i < end) && *i; i = g_utf8_next_char(i))
	{
		gunichar c = g_utf8_get_char(i);

		if (token_start == NULL)
		{
			token_start = i;
		}

		switch (mode)
		{
		case MODE_URI_PATH:
			if (c == '?')
			{
				r->path = g_string_new_len(token_start, (i - token_start));
				token_start = NULL;
				mode = MODE_URI_QUERY;
			}
			break;

		case MODE_URI_QUERY:
			if (c == '#')
			{
				r->query = parse_query(token_start, (i - token_start));
				token_start = NULL;
				mode = MODE_URI_FRAGMENT;
				break;
			}
			break;
		}
	}

	switch (mode)
	{
	case MODE_URI_PATH:
		r->path = g_string_new_len(data, len);
		break;

	case MODE_URI_QUERY:
		r->query = parse_query(token_start, len - (token_start - data));
		break;

	case MODE_URI_FRAGMENT:
		i = g_utf8_next_char(i);
		r->fragment = g_string_new_len(token_start, len - (token_start - data));
		break;
	}
}

#define HEADER_BUF_SIZE 1024
#define MODE_REQUEST_LINE 0
#define MODE_HEADER_LINE  1
#define MODE_REQUEST_BODY 2

int req_parser_add_text(req_parser *r, const char *data, size_t len)
{
	const char *end;
	if (!g_utf8_validate(data, len, &end))
	{
		return REQ_PARSER_PARSE_INVALID_REQUEST;
	}

	int result = REQ_PARSER_PARSE_PARTIAL_REQUEST;

	int mode = (r->method == REQ_PARSER_METHOD_INVALID) ? MODE_REQUEST_LINE : MODE_HEADER_LINE;
	const gchar *headerQuark = NULL;

	char buf[HEADER_BUF_SIZE];
	const char *token_start = data;
	const char *i;
	for (i = data; (i < end) && *i; i = g_utf8_next_char(i))
	{
		result = REQ_PARSER_PARSE_PARTIAL_REQUEST;
		gunichar c = g_utf8_get_char(i);

		if (c == '\r')
		{
			i = g_utf8_next_char(i);
			c = g_utf8_get_char(i);
		}
		if ((c == '\n') && (*g_utf8_prev_char(i) != '\r'))
		{
			fprintf(stderr, "ERROR - Encountered a newline without a preceding carriage return.\n");
			req_parser_reset(r);
			return REQ_PARSER_PARSE_INVALID_REQUEST;
		}

		switch (mode)
		{
		case MODE_REQUEST_LINE:
			if (c == ' ')
			{
				if (r->method == REQ_PARSER_METHOD_INVALID)
				{
					if (strncmp("GET", token_start, 3) == 0) r->method = REQ_PARSER_METHOD_GET;
					else if (strncmp("HEAD", token_start, 4) == 0) r->method = REQ_PARSER_METHOD_HEAD;
					else if (strncmp("POST", token_start, 4) == 0) r->method = REQ_PARSER_METHOD_POST;
					else if (strncmp("OPTIONS", token_start, 7) == 0) r->method = REQ_PARSER_METHOD_OPTIONS;
					else
					{
						fprintf(stderr, "ERROR - Unknown method\n");
						req_parser_reset(r);
						return REQ_PARSER_PARSE_INVALID_REQUEST;
					}

					// skip over the separating space
					i = g_utf8_next_char(i);
					token_start = i;
				}
				else if (r->path == NULL)
				{
					parse_uri(r, token_start, (i - token_start));

					// skip over the separating space
					i = g_utf8_next_char(i);
					token_start = i;
				}
				else
				{
					fprintf(stderr, "ERROR - Too many spaces in request line\n");
					req_parser_reset(r);
					return REQ_PARSER_PARSE_INVALID_REQUEST;
				}
			}
			else if (c == '\n')
			{
				if (strncmp("HTTP/1.1", token_start, min(8, i - token_start)) != 0)
				{
					fprintf(stderr, "ERROR - Unsupported HTTP version\n");
					req_parser_reset(r);
					return REQ_PARSER_PARSE_INVALID_REQUEST;
				}
				token_start = NULL;
				mode = MODE_HEADER_LINE;
			}
			break;

		case MODE_HEADER_LINE:
			if (token_start == NULL)
			{
				token_start = i;
			}

			if ((token_start == i) && (c == '\n'))
			{
				result = REQ_PARSER_PARSE_OK;
				mode = MODE_REQUEST_BODY;
			}
			else if ((headerQuark == NULL) && (c == ':'))
			{
				memset(buf, 0, HEADER_BUF_SIZE);
				memcpy(buf, token_start, min((i - token_start), HEADER_BUF_SIZE - 1));
				headerQuark = g_intern_string(buf);
				token_start = NULL;

				// skip over the separating space
				i = g_utf8_next_char(i);
			}
			else if (c == '\n')
			{
				if (!r->headers)
				{
					r->headers = g_tree_new_full(quarkcmp, NULL, NULL, gstring_destroy_notify);
				}

				// Subtract one since we don't want the "\r" in the header
				g_tree_insert(r->headers, (gpointer)headerQuark, g_string_new_len(token_start, (i - token_start) - 1));
				token_start = NULL;
				headerQuark = NULL;
			}
			break;

		case MODE_REQUEST_BODY:
			if (r->body == NULL)
			{
				token_start = i;

				if (!r->headers)
				{
					r->headers = g_tree_new_full(quarkcmp, NULL, NULL, gstring_destroy_notify);
				}

				GString *length_header = g_tree_lookup(r->headers, g_intern_static_string("Content-Length"));
				if (length_header)
				{
					int length = atoi(length_header->str);
					size_t bytes_remaining = len - (data - i);

					if (bytes_remaining < (size_t)length)
					{
						fprintf(stderr, "ERROR - Body incomplete.\n");
						req_parser_reset(r);
						return REQ_PARSER_PARSE_INVALID_REQUEST;
					}

					r->body = g_string_new_len(i, length);
					i = i + length;
				}
				else
				{
					size_t length = len - (data - i);
					r->body = g_string_new_len(i, length);
					i = i + length;
				}
				result = REQ_PARSER_PARSE_OK;
			}
			else if (c == '\n')
			{
				result = REQ_PARSER_PARSE_OK;
			}
			break;
		}
	}

	r->status = result;
	return result;
}
