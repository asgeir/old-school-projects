#include "httpd_helpers.h"

gint quarkcmp(gconstpointer a, gconstpointer b, gpointer user_data)
{
	(void)user_data;
	return (gint)(a - b);
}

void gstring_destroy_notify(gpointer data)
{
	g_string_free((GString *)data, TRUE);
}

static char *unescape_string(const char *data, size_t len)
{
	GString *tmp = g_string_new_len(data, len);
	char *result = g_uri_unescape_string(tmp->str, NULL);
	g_string_free(tmp, TRUE);

	return result;
}

#define MODE_QUERY_FIELD 0
#define MODE_QUERY_VALUE 1

GTree *parse_query(const char *data, size_t len)
{
	int mode = MODE_QUERY_FIELD;

	GTree *result = g_tree_new_full(quarkcmp, NULL, NULL, gstring_destroy_notify);
	const gchar *fieldQuark = NULL;

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
		case MODE_QUERY_FIELD:
			if (c == '=')
			{
				char *tmp = unescape_string(token_start, (i - token_start));
				fieldQuark = g_intern_string(tmp);
				g_free(tmp);
				token_start = NULL;
				mode = MODE_QUERY_VALUE;
			}
			break;

		case MODE_QUERY_VALUE:
			if (c == '&')
			{
				char *tmp = unescape_string(token_start, (i - token_start));
				g_tree_insert(result, (gpointer)fieldQuark, g_string_new(tmp));
				g_free(tmp);
				fieldQuark = NULL;
				token_start = NULL;
				mode = MODE_QUERY_FIELD;
			}
			break;
		}
	}

	if (mode == MODE_QUERY_VALUE)
	{
		char *tmp = unescape_string(token_start, len - (token_start - data));
		g_tree_insert(result, (gpointer)fieldQuark, g_string_new(tmp));
		g_free(tmp);
	}

	return result;
}
