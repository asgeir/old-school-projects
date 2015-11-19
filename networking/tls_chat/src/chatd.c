#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <signal.h>
#include <unistd.h>
#include <stdint.h>

#include "msc_server.h"
#include "msc_library.h"

void fatal_server_error(const char *msg, MSC_SERVER_ERR e)
{
	fprintf(stderr, "%s: %s\n", msg, msc_server_err_desc(e));
	exit(EXIT_FAILURE);
}

// To avoid data races only write to this variable from exit_signal_handler
static int running = 1;
void exit_signal_handler(int signo, siginfo_t *sinfo, void *context)
{
	(void)signo;
	(void)sinfo;
	(void)context;

	running = 0;
}

void setup_signal_handlers()
{
	struct sigaction act;

	memset(&act, 0, sizeof(struct sigaction));
	sigemptyset(&act.sa_mask);

	act.sa_sigaction = exit_signal_handler;
	act.sa_flags = SA_SIGINFO;

	if (sigaction(SIGTERM, &act, NULL) < 0) halt_os_error("Unable to set up SIGTERM handler");
	if (sigaction(SIGINT, &act, NULL) < 0) halt_os_error("Unable to set up SIGINT handler");
}

void print_usage()
{
	fprintf(stderr,
			"Usage:\n"
				"\tchatd <port> <certificate_file> <key_file>\n"
				"\tThe certificate file defaults to 'cert.pem'\n"
				"\tThe key file defaults to 'key.pem'\n");
}

int main(int argc, const char **argv)
{
	if ((argc < 2) || (argc > 4))
	{
		print_usage();
		exit(EXIT_FAILURE);
	}

	setup_signal_handlers();

	uint16_t port = (uint16_t)atoi(argv[1]);
	const char *cert_file = (argc >= 3) ? argv[2] : "cert.pem";
	const char *key_file = (argc >= 4) ? argv[3] : "key.pem";

	if (!msc_library_init(MSC_LIBRARY_SERVER))
	{
		fatal_error("Unable to initialize msc library");
		exit(EXIT_FAILURE);
	}

	msc_server *s = msc_server_create(port, cert_file, key_file, 500);
	MSC_SERVER_ERR e = msc_server_listen(s);
	if (e)
	{
		msc_server_destroy(s);
		msc_library_finalize();
		fatal_server_error("Unable to start listening", e);
	}

	printf("Listening...\n");
	fflush(stdout);

	while (running)
	{
		msc_server_wait(s);
	}

	printf("Shutting down...\n");
	fflush(stdout);

	msc_server_close(s);
	msc_server_destroy(s);
	msc_library_finalize();

	return 0;
}
