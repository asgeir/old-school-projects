#include "msc_server.h"
#include "msc_library.h"
#include "msc_srp_auth.h"

#include <sqlite3.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CREATE_TABLES "CREATE TABLE IF NOT EXISTS users (name VARCHAR PRIMARY KEY, s VARCHAR, v VARCHAR);"

void print_usage()
{
	fprintf(stderr,
			"Inserts a new user into the password database.\n\n"
			"Usage:\n"
				"\tchatd_user <user_name>\n");
}

void setup_db(sqlite3 *db)
{
	char *zErrMsg = NULL;
	if (sqlite3_exec(db, CREATE_TABLES, NULL, 0, &zErrMsg) != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
}

void add_user(sqlite3 *db, const char *username, const char *passwd)
{
	const unsigned char *s = NULL;
	const unsigned char *v = NULL;
	int s_len = 0;
	int v_len = 0;

	srp_create_salted_verification_key(
		SRP_SHA256, SRP_NG_2048,
		username,
		(unsigned char *)passwd, (int)strnlen(passwd, 48),
		&s, &s_len,
		&v, &v_len,
		NULL, NULL);

	gchar *s_b64 = g_base64_encode(s, (gsize)s_len);
	gchar *v_b64 = g_base64_encode(v, (gsize)v_len);

	sqlite3_stmt *stmt = NULL;
	const char *insert_query = "INSERT INTO users (name, s, v) VALUES (?, ?, ?);";
	if (sqlite3_prepare_v2(db, insert_query, (int)strlen(insert_query), &stmt, NULL) != SQLITE_OK)
	{
		fprintf(stderr, "Unable to prepare SQL statement.\n");
		g_free(s_b64);
		g_free(v_b64);
		goto cleanup;
	}

	sqlite3_bind_text(stmt, 1, username, (int)strnlen(username, 64), NULL);
	sqlite3_bind_text(stmt, 2, s_b64, (int)strnlen(s_b64, 64), g_free);
	sqlite3_bind_text(stmt, 3, v_b64, (int)strnlen(v_b64, 4096), g_free);

	if (sqlite3_step(stmt) != SQLITE_DONE)
	{
		fprintf(stderr, "Unable to insert user into database");
	}

	sqlite3_finalize(stmt);

	cleanup:
	free((void *)s);
	free((void *)v);
}

int main(int argc, const char **argv)
{
	if (argc != 2)
	{
		print_usage();
		exit(EXIT_FAILURE);
	}

	if (!msc_library_init(MSC_LIBRARY_SERVER))
	{
		fatal_error("Unable to initialize msc library");
		exit(EXIT_FAILURE);
	}

	sqlite3 *db;
	if (sqlite3_open("chatd_passwd.db", &db))
	{
		fatal_error("Unable to open password db");
	}

	setup_db(db);

	char passwd[48];
	getpasswd("Password: ", passwd, 48);

	add_user(db, argv[1], passwd);

	sqlite3_close(db);
	msc_library_finalize();

	return 0;
}
