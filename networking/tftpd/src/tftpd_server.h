#ifndef TSAM_TFTPD_TFTPD_SERVER_H
#define TSAM_TFTPD_TFTPD_SERVER_H

typedef struct _server server;

server *server_create(uint16_t port, const char *path);
void server_destroy(server *s);

void server_listen(server *s);
void server_close(server *s);
void server_wait(server *s);

#endif //TSAM_TFTPD_TFTPD_SERVER_H
