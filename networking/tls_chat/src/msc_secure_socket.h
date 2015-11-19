#ifndef CHAT_MSC_SECURE_SOCKET_H
#define CHAT_MSC_SECURE_SOCKET_H

#include "msc_common.h"

#include <stdint.h>
#include <glib.h>

typedef struct _msc_secure_socket msc_secure_socket;
typedef int MSC_SOCKET_ERR;

#define MSC_SOCKET_ERR_NO_ERROR               (0)
#define MSC_SOCKET_ERR_HOST_LOOKUP            (1)
#define MSC_SOCKET_ERR_SOCKET_CREATE          (2)
#define MSC_SOCKET_ERR_CONNECT                (3)
#define MSC_SOCKET_ERR_CONNECT_SSL            (4)
#define MSC_SOCKET_ERR_READ_BUF_NULL          (5)
#define MSC_SOCKET_ERR_EMPTY_READ             (6)
#define MSC_SOCKET_ERR_WRITE_BUF_NULL         (7)
#define MSC_SOCKET_ERR_WRITE_ERROR            (8)
#define MSC_SOCKET_ERR_NOT_IMPLEMENTED        (0xffff)

MSC_SOCKET_ERR msc_secure_socket_ctx_load_cert(const char *cert_file, const char *key_file);

msc_secure_socket *msc_secure_socket_create();
void msc_secure_socket_destroy(msc_secure_socket *s);

int msc_secure_socket_socket(msc_secure_socket *s);
GString *msc_secure_socket_hostname(msc_secure_socket *s);
uint16_t msc_secure_socket_port(msc_secure_socket *s);

void msc_secure_socket_accept(msc_secure_socket *s, int listenfd);
void msc_secure_socket_close(msc_secure_socket *s);
MSC_SOCKET_ERR msc_secure_socket_connect(msc_secure_socket *s, const char *hostname, uint16_t port);
MSC_SOCKET_ERR msc_secure_socket_read(msc_secure_socket *s, GString *buf);
MSC_SOCKET_ERR msc_secure_socket_write(msc_secure_socket *s, GString *buf);

MSC_BOOL msc_secure_socket_library_init(MSC_LIBRARY_MODE mode);
void msc_secure_socket_library_finalize();

#endif //CHAT_MSC_SECURE_SOCKET_H
