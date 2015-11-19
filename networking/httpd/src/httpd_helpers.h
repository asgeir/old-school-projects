#ifndef HTTPD_HTTPD_HELPERS_H
#define HTTPD_HTTPD_HELPERS_H

#include <glib.h>

gint quarkcmp(gconstpointer a, gconstpointer b, gpointer user_data);
void gstring_destroy_notify(gpointer data);
GTree *parse_query(const char *data, size_t len);

#endif //HTTPD_HTTPD_HELPERS_H
