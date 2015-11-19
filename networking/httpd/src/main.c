#include "httpd_common.h"
#include "httpd_server.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>

// To avoid data races only write to this variable from exit_signal_handler
static int running = 1;
void exit_signal_handler(int signo, siginfo_t *sinfo, void *context)
{
	(void)signo;
	(void)sinfo;
	(void)context;

	running = 0;
}

void print_usage()
{
	fprintf(stderr, "Usage:\n\thttpd <port>\n");
}

void setup_signal_handlers()
{
	struct sigaction act;

	memset(&act, 0, sizeof(struct sigaction));
	sigemptyset(&act.sa_mask);

	act.sa_sigaction = exit_signal_handler;
	act.sa_flags = SA_SIGINFO;

	if (sigaction(SIGTERM, &act, NULL) < 0) halt_error("Unable to set up SIGTERM handler");
	if (sigaction(SIGINT, &act, NULL) < 0) halt_error("Unable to set up SIGINT handler");
}

void halt_error(const char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, const char **argv)
{
	if (argc != 2)
	{
		print_usage();
		return 1;
	}

	setup_signal_handlers();

	server *s = server_create((uint16_t)atoi(argv[1]));
	server_listen(s);

	printf("Listening...\n");
	fflush(stdout);

	while (running)
	{
		server_wait(s);
	}

	printf("Shutting down...\n");
	fflush(stdout);

	server_close(s);
	server_destroy(s);

	return 0;
}
