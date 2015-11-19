#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "tftpd_server.h"
#include "tftpd_common.h"

#define BLOCK_SIZE        512
#define MAX_SIZE          (BLOCK_SIZE + 4)
#define MAX_TIMEOUT_COUNT 5

#define STATE_IDLE  0
#define STATE_READ  1
#define STATE_WRITE 2

#define MODE_UNDEFINED 0
#define MODE_TEXT      1
#define MODE_BINARY    2

#define OPCODE_UNDEFINED     0
#define OPCODE_READ_REQUEST  1
#define OPCODE_WRITE_REQUEST 2
#define OPCODE_DATA          3
#define OPCODE_ACK           4
#define OPCODE_ERROR         5

#define ERROR_UNDEFINED           0
#define ERROR_FILE_NOT_FOUND      1
#define ERROR_ACCESS_VIOLATION    2
#define ERROR_DISK_FULL           3
#define ERROR_ILLEGAL_OPERATION   4
#define ERROR_UNKNOWN_TRANSFER_ID 5
#define ERROR_FILE_EXISTS         6
#define ERROR_NO_SUCH_USER        7

#define TIMEOUT_SEC  0
#define TIMEOUT_USEC (500 * 1000)

#define min(x, y) (((x) < (y)) ? (x) : (y))

char *STD_ERR_MSG[] = {
	NULL,
	"File not found",
	"Access violation",
	"Disk full",
	"Illegal operation",
	"Unknown transfer id",
	"File exists",
	"No such user"
};

typedef struct _server {
	int socket;
	struct sockaddr_in addr;

	char basepath[PATH_MAX];
	size_t basepath_len;
	FILE *read_fp;
	char buf[BLOCK_SIZE];
	size_t buf_size;

	short block_id;
	int state;

	struct sockaddr_in client_addr;
	socklen_t client_len;
	int client_timeout_ctr;
} server;

typedef struct _packet {
	int opcode;
	int mode;
	int block_id;
	char filename[PATH_MAX];
} packet;

void server_transition_idle(server *s);
void server_handle_msg(server *s);

server *server_create(uint16_t port, const char *path)
{
	server *s = (server *)malloc(sizeof(server));
	if (!s)
	{
		halt_error("Unable to allocate server structure");
		return NULL;
	}

	s->socket = 0;
	s->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s->socket < 0)
	{
		halt_error("Unable to create server socket");
		return NULL;
	}

	setsockopt(s->socket, SOL_SOCKET, SO_REUSEADDR, (void *)1, sizeof(int));
	setsockopt(s->socket, SOL_SOCKET, SO_REUSEPORT, (void *)1, sizeof(int));

	memset(&s->addr, 0, sizeof(s->addr));

	s->addr.sin_family = AF_INET;
	s->addr.sin_addr.s_addr = htonl(INADDR_ANY);
	s->addr.sin_port = htons(port);

	if (!realpath(path, s->basepath))
	{
		free(s);
		halt_error("Invalid data path");
		return NULL;
	}
	s->basepath_len = strlen(s->basepath);

	s->read_fp = NULL;

	server_transition_idle(s);

	return s;
}

void server_destroy(server *s)
{
	server_close(s);
	free(s);
}

void server_listen(server *s)
{
	if (!s) return;

	if (bind(s->socket, (struct sockaddr*)&s->addr, sizeof(s->addr)) < 0)
	{
		halt_error("Could not bind to socket");
	}
}

void server_close(server *s)
{
	if (!s) return;

	close(s->socket);
	s->socket = 0;

	memset(&s->addr, '0', sizeof(s->addr));
}

void server_transition_idle(server *s)
{
	s->state = STATE_IDLE;

	if (s->read_fp) fclose(s->read_fp);
	s->read_fp = NULL;

	memset(&s->buf, 0, sizeof(s->buf));
	s->buf_size = 0;

	s->block_id = 0;
	s->state = STATE_IDLE;

	memset(&s->client_addr, 0, sizeof(s->client_addr));
	s->client_len = (socklen_t)sizeof(s->client_addr);
	s->client_timeout_ctr = 0;
}

int server_send(server *s, char *data, size_t n)
{
	size_t rem = min(n, MAX_SIZE);
	while (rem)
	{
		ssize_t sent = sendto(s->socket, data, rem, MSG_NOSIGNAL, (const struct sockaddr *)&s->client_addr, s->client_len);
		if (sent < 0) return 1;

		rem -= sent;
		data += sent;
	}

	return 0;
}

int server_send_data(server *s, short blkid, char *data, size_t n)
{
	char packet[MAX_SIZE];
	packet[0] = 0;
	packet[1] = OPCODE_DATA;
	packet[2] = (char)((blkid >> 8) & 0xff);
	packet[3] = (char)(blkid & 0xff);
	memcpy(packet + 4, data, min(n, BLOCK_SIZE));

	return server_send(s, packet, min(n, BLOCK_SIZE) + 4);
}

int server_send_error(server *s, short code, char *msg)
{
	char packet[MAX_SIZE];
	memset(packet, 0, MAX_SIZE);

	packet[0] = 0;
	packet[1] = OPCODE_ERROR;
	packet[2] = (char)((code >> 8) & 0xff);
	packet[3] = (char)(code & 0xff);

	if (!msg) msg = STD_ERR_MSG[code];
	size_t size = strnlen(msg, MAX_SIZE-6);
	memcpy(packet + 4, msg, size);

	return server_send(s, packet, size + 5);
}

int server_decode_packet(server *s, packet *p, char *buf, size_t max_size)
{
	if (max_size < 4)
	{
		return 1;
	}

	size_t mode_start = 0;
	char filename[PATH_MAX];
	char filepath[PATH_MAX];
	char filemode[20];

	p->opcode = (*buf << 8) | *(buf+1);
	switch (p->opcode)
	{
	case OPCODE_READ_REQUEST:
	case OPCODE_WRITE_REQUEST:
		if (max_size < 9)
		{
			return 1;
		}

		strncpy(filename, buf + 2, min(max_size - 2, PATH_MAX-1));
		mode_start = 3 + strnlen(filename, PATH_MAX);

		printf("file \"%s\" requested from %s:%u\n", filename, inet_ntoa(s->client_addr.sin_addr), s->client_addr.sin_port);

		snprintf(filepath, PATH_MAX-1, "%s/%s", s->basepath, filename);
		if (!realpath(filepath, p->filename))
		{
			server_send_error(s, ERROR_FILE_NOT_FOUND, NULL);
			server_transition_idle(s);
			return -1;
		}

		strncpy(filemode, buf + mode_start, min(max_size - mode_start, 20-1));
		if (strncasecmp(filemode, "netascii", 8) == 0)
		{
			p->mode = MODE_TEXT;
			return 1;
		}
		else if (strncasecmp(filemode, "octet", 5) == 0)
		{
			p->mode = MODE_BINARY;
		}
		else
		{
			p->mode = MODE_UNDEFINED;
			return 1;
		}
		break;

	case OPCODE_ACK:
		if (max_size < 4)
		{
			return 1;
		}

		p->block_id = ((*(buf + 2) & 0xff) << 8) | (*(buf + 3) & 0xff);
		break;

	default:
		return 1;
	}

	return 0;
}

int server_read_block(server *s)
{
	s->buf_size = fread(s->buf, sizeof(char), sizeof(s->buf) / sizeof(char), s->read_fp);
	if (!s->buf_size)
	{
		if (ferror(s->read_fp))
		{
			server_send_error(s, ERROR_UNDEFINED, "I/O error reading file");
			server_transition_idle(s);
			return 1;
		}

		return server_send_data(s, ++s->block_id, NULL, 0);
	}

	return server_send_data(s, ++s->block_id, s->buf, s->buf_size);
}

void server_init_read_request(server *s, packet *p)
{
	if (strncmp(p->filename, s->basepath, s->basepath_len) != 0)
	{
		server_send_error(s, ERROR_ACCESS_VIOLATION, NULL);
		server_transition_idle(s);
		return;
	}

	if (p->mode == MODE_TEXT)
	{
		s->read_fp = fopen(p->filename, "r");
	}
	else if (p->mode == MODE_BINARY)
	{
		s->read_fp = fopen(p->filename, "rb");
	}
	else
	{
		server_send_error(s, ERROR_ILLEGAL_OPERATION, "Unsupported file mode");
		server_transition_idle(s);
		return;
	}

	if (!s->read_fp)
	{
		server_send_error(s, ERROR_FILE_NOT_FOUND, NULL);
		server_transition_idle(s);
		return;
	}

	s->state = STATE_READ;
	if (server_read_block(s))
	{
		server_transition_idle(s);
	}
}

void server_continue_read(server *s, packet *p)
{
	if (p->block_id > s->block_id)
	{
		server_send_error(s, ERROR_ILLEGAL_OPERATION, "Future block acknowledged");
		server_transition_idle(s);
	}
	else if (p->block_id < s->block_id)
	{
		// duplicate ack
		return;
	}

	int is_final_ack = (p->block_id > 0) && (s->buf_size < BLOCK_SIZE);
	if (is_final_ack || server_read_block(s))
	{
		server_transition_idle(s);
	}
}

void server_wait(server *s)
{
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(s->socket, &rfds);

	struct timeval tv;
	tv.tv_sec = TIMEOUT_SEC;
	tv.tv_usec = TIMEOUT_USEC;
	int status = select(s->socket + 1, &rfds, NULL, NULL, &tv);

	if (status < 0)
	{
		server_transition_idle(s);
	}
	else if (status > 0)
	{
		if (!FD_ISSET(s->socket, &rfds))
		{
			halt_error("Select failed");
			return;
		}

		s->client_timeout_ctr = 0;
		server_handle_msg(s);
	}
	else
	{
		switch (s->state)
		{
		case STATE_IDLE:
			// Just waiting for a connection
			break;

		case STATE_READ:
			if (s->client_timeout_ctr++ < MAX_TIMEOUT_COUNT)
			{
				server_send_data(s, s->block_id, s->buf, s->buf_size);
			}
			else
			{
				server_transition_idle(s);
			}
			break;

		default:
			halt_error("Invalid server state");
		}
	}
}

void server_handle_msg(server *s)
{
	struct sockaddr_in client;
	socklen_t slen = (socklen_t)sizeof(client);
	char recv_buf[MAX_SIZE];

	memset(&client, 0, sizeof(client));
	if (recvfrom(s->socket, &recv_buf, MAX_SIZE, 0, (struct sockaddr *)&client, &slen) < 0)
	{
		server_transition_idle(s);
	}

	if (s->state != STATE_IDLE && memcmp(&s->client_addr, &client, min(s->client_len, slen)) != 0)
	{
		server_send_error(s, ERROR_UNKNOWN_TRANSFER_ID, "Another user is already using the service");
		return;
	}
	else
	{
		s->client_addr = client;
		s->client_len = slen;
	}

	packet p;
	int p_status = server_decode_packet(s, &p, recv_buf, MAX_SIZE);
	if (p_status > 0)
	{
		server_send_error(s, ERROR_ILLEGAL_OPERATION, NULL);
		server_transition_idle(s);
		return;
	}
	else if (p_status < 0)
	{
		return;
	}

	switch (s->state)
	{
	case STATE_IDLE:
		switch (p.opcode)
		{
		case OPCODE_READ_REQUEST:
			server_init_read_request(s, &p);
			break;

		default:
			server_send_error(s, ERROR_ILLEGAL_OPERATION, NULL);
			server_transition_idle(s);
			break;
		}
		break;

	case STATE_READ:
		switch (p.opcode)
		{
		case OPCODE_ACK:
			server_continue_read(s, &p);
			break;

		default:
			server_send_error(s, ERROR_ILLEGAL_OPERATION, NULL);
			server_transition_idle(s);
			break;
		}
		break;

	default:
		server_send_error(s, ERROR_ILLEGAL_OPERATION, NULL);
		server_transition_idle(s);
		break;
	}
}
