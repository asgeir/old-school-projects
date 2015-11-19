#include "msc_secure_socket.h"
#include "msc_common.h"

#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netdb.h>
#include <unistd.h>

typedef struct _msc_secure_socket {
	int socket;
	struct sockaddr_in remote_addr;
	socklen_t addr_len;
	SSL *ssl;
} msc_secure_socket;

SSL_CTX *g_ssl_ctx = NULL;

MSC_SOCKET_ERR msc_secure_socket_ctx_load_cert(const char *cert_file, const char *key_file)
{
	if (SSL_CTX_use_certificate_file(g_ssl_ctx, cert_file, SSL_FILETYPE_PEM) <= 0)
	{
		fprintf(stderr, "Unable to load server certificate from file\n");
		ERR_print_errors_fp(stderr);
		return MSC_FALSE;
	}
	if (SSL_CTX_use_PrivateKey_file(g_ssl_ctx, key_file, SSL_FILETYPE_PEM) <= 0)
	{
		fprintf(stderr, "Unable to load server key from file\n");
		ERR_print_errors_fp(stderr);
		return MSC_FALSE;
	}
	if (!SSL_CTX_check_private_key(g_ssl_ctx))
	{
		fprintf(stderr, "Server private key does not match server certificate\n");
		ERR_print_errors_fp(stderr);
		return MSC_FALSE;
	}

	return MSC_TRUE;
}

msc_secure_socket *msc_secure_socket_create()
{
	msc_secure_socket *s = (msc_secure_socket *)malloc(sizeof(msc_secure_socket));
	if (s)
	{
		s->socket = 0;
		memset(&s->remote_addr, 0, sizeof(s->remote_addr));
		s->addr_len = sizeof(s->remote_addr);

		s->ssl = NULL;
	}

	return s;
}

void msc_secure_socket_destroy(msc_secure_socket *s)
{
	if (!s) return;

	msc_secure_socket_close(s);
	free(s);
}

int msc_secure_socket_socket(msc_secure_socket *s)
{
	return s->socket;
}

GString *msc_secure_socket_hostname(msc_secure_socket *s)
{
	char addr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(s->remote_addr.sin_addr), addr, INET_ADDRSTRLEN);

	return g_string_new_len(addr, INET_ADDRSTRLEN);
}

uint16_t msc_secure_socket_port(msc_secure_socket *s)
{
	return ntohs(s->remote_addr.sin_port);
}

void msc_secure_socket_accept(msc_secure_socket *s, int listenfd)
{
	msc_secure_socket_close(s);

	s->socket = accept(listenfd, (struct sockaddr *)&s->remote_addr, &s->addr_len);
	if (s->socket < 0)
	{
		if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
		{
			s->socket = 0;
			msc_secure_socket_close(s);
		}
		else
		{
			halt_os_error("Unable to accept connection");
		}
		return;
	}

	s->ssl = SSL_new(g_ssl_ctx);
	SSL_set_fd(s->ssl, s->socket);

	if (SSL_accept(s->ssl) != 1)
	{
		fprintf(stderr, "Error accepting OpenSSL connection\n");
		ERR_print_errors_fp(stderr);

		msc_secure_socket_close(s);
	}
}

void msc_secure_socket_close(msc_secure_socket *s)
{
	if (s->ssl) SSL_free(s->ssl);
	s->ssl = NULL;

	if (s->socket != 0) close(s->socket);
	s->socket = 0;

	memset(&s->remote_addr, '0', sizeof(s->remote_addr));
	s->addr_len = sizeof(s->remote_addr);
}

MSC_SOCKET_ERR msc_secure_socket_connect(msc_secure_socket *s, const char *hostname, uint16_t port)
{
	msc_secure_socket_close(s);

	struct hostent *server = gethostbyname(hostname);
	if (!server)
	{
		return MSC_SOCKET_ERR_HOST_LOOKUP;
	}

	s->socket = socket(AF_INET, SOCK_STREAM, 0);
	if (s->socket < 0)
	{
		s->socket = 0;
		return MSC_SOCKET_ERR_SOCKET_CREATE;
	}

	s->remote_addr.sin_family = AF_INET;
	memcpy(&(s->remote_addr.sin_addr.s_addr), server->h_addr, (size_t)server->h_length);
	s->remote_addr.sin_port = htons(port);

	if (connect(s->socket, (const struct sockaddr *)&s->remote_addr, sizeof(s->remote_addr)) < 0)
	{
		msc_secure_socket_close(s);
		return MSC_SOCKET_ERR_CONNECT;
	}

	s->ssl = SSL_new(g_ssl_ctx);
	SSL_set_fd(s->ssl, s->socket);

	if (SSL_connect(s->ssl) != 1)
	{
		fprintf(stderr, "Error establishing OpenSSL connection\n");
		ERR_print_errors_fp(stderr);

		msc_secure_socket_close(s);
		return MSC_SOCKET_ERR_CONNECT_SSL;
	}

	return MSC_SOCKET_ERR_NO_ERROR;
}

MSC_SOCKET_ERR msc_secure_socket_read(msc_secure_socket *s, GString *buf)
{
	if (!buf) return MSC_SOCKET_ERR_READ_BUF_NULL;

	char tmp[4*1024];
	int bytes_read = SSL_read(s->ssl, tmp, sizeof(tmp));
	if (bytes_read > 0)
	{
		g_string_truncate(buf, 0);
		g_string_append_len(buf, tmp, bytes_read);
		return MSC_SOCKET_ERR_NO_ERROR;
	}

	return MSC_SOCKET_ERR_EMPTY_READ;
}

MSC_SOCKET_ERR msc_secure_socket_write(msc_secure_socket *s, GString *buf)
{
	if (!s->ssl) return MSC_SOCKET_ERR_WRITE_ERROR;
	if (!buf) return MSC_SOCKET_ERR_WRITE_BUF_NULL;

	char *data = buf->str;
	size_t bytes_remaining = buf->len;

	while (bytes_remaining)
	{
		int bytes_written = SSL_write(s->ssl, data, (int)bytes_remaining);
		if (bytes_written > 0)
		{
			bytes_remaining -= bytes_written;
			data += bytes_written;
		}
		else
		{
			fprintf(stderr, "Error writing to OpenSSL connection\n");
			ERR_print_errors_fp(stderr);

			msc_secure_socket_close(s);
			return MSC_SOCKET_ERR_WRITE_ERROR;
		}
	}

	return MSC_SOCKET_ERR_NO_ERROR;
}

MSC_BOOL msc_secure_socket_library_init(MSC_LIBRARY_MODE mode)
{
	if (g_ssl_ctx) return MSC_TRUE;

	SSL_library_init();
	SSL_load_error_strings();

	if (mode == MSC_LIBRARY_CLIENT)
	{
		g_ssl_ctx = SSL_CTX_new(TLSv1_client_method());
	}
	else
	{
		g_ssl_ctx = SSL_CTX_new(TLSv1_server_method());
	}

	if (!g_ssl_ctx)
	{
		fprintf(stderr, "Error initializing OpenSSL\n");
		ERR_print_errors_fp(stderr);
		return MSC_FALSE;
	}

	return MSC_TRUE;
}

void msc_secure_socket_library_finalize()
{
	if (g_ssl_ctx) SSL_CTX_free(g_ssl_ctx);
	g_ssl_ctx = NULL;
}
