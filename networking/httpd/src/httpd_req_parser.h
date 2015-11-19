#ifndef HTTPD_HTTPD_REQ_PARSER_H
#define HTTPD_HTTPD_REQ_PARSER_H

#include <glib.h>

#define REQ_PARSER_PARSE_OK              0
#define REQ_PARSER_PARSE_PARTIAL_REQUEST 1
#define REQ_PARSER_PARSE_INVALID_REQUEST 2

#define REQ_PARSER_METHOD_INVALID -1
#define REQ_PARSER_METHOD_GET      0
#define REQ_PARSER_METHOD_HEAD     1
#define REQ_PARSER_METHOD_POST     2
#define REQ_PARSER_METHOD_OPTIONS  3

typedef struct _req_parser req_parser;

req_parser *req_parser_create();
void        req_parser_destroy(req_parser *r);

int      req_parser_status(req_parser *r);
int      req_parser_method(req_parser *r);
GString *req_parser_path(req_parser *r);
GString *req_parser_fragment(req_parser *r);
GString *req_parser_body(req_parser *r);
GTree   *req_parser_query(req_parser *r);
GTree   *req_parser_headers(req_parser *r);

void req_parser_reset(req_parser *r);
int  req_parser_add_text(req_parser *r, const char *data, size_t len);

#endif //HTTPD_HTTPD_REQ_PARSER_H
