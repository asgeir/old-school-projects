#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <unistd.h>

#include "msc_client.h"
#include "msc_library.h"

void fatal_client_error(const char *msg, MSC_CLIENT_ERR e)
{
	fprintf(stderr, "%s: %s\n", msg, msc_client_err_desc(e));
	exit(EXIT_FAILURE);
}

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

static msc_client *client = NULL;

/*
 * This variable shall point to the name of the user. The initial value
 * is NULL. Set this variable to the username once the user managed to be
 * authenticated
 */
static GString *user = NULL;

/*
 * This variable shall point to the name of the chatroom. The initial
 * value is NULL (not member of a chat room). Set this variable whenever
 * the user changed the chat room successfully.
 */
static GString *chatroom = NULL;

/*
 * This prompt is used by the readline library to ask the user for
 * input. It is good style to indicate the name of the user and the
 * chat room he is in as part of the prompt.
 */
static GString *prompt = NULL;

void update_prompt()
{
	g_string_truncate(prompt, 0);

	if (chatroom->len && user->len)
	{
		g_string_printf(prompt, "%s# %s> ", chatroom->str, user->str);
	}
	else if (chatroom->len)
	{
		g_string_printf(prompt, "%s# > ", chatroom->str);
	}
	else if (user->len)
	{
		g_string_printf(prompt, "%s> ", user->str);
	}
	else
	{
		g_string_printf(prompt, "> ");
	}

	rl_set_prompt(prompt->str);
}

/*
 * When a line is entered using the readline library, this function
 * gets called to handle the entered line. Implement the code to
 * handle the user requests in this function. The client handles the
 * server messages in the loop in main().
 */
void readline_callback(char *line)
{
	if (NULL == line)
	{
		rl_callback_handler_remove();
		running = 0;
		return;
	}

	if (strlen(line) > 0)
	{
		add_history(line);
	}

	if ((strncmp("/bye", line, 4) == 0) || (strncmp("/quit", line, 5) == 0))
	{
		msc_client_disconnect(client);
		rl_callback_handler_remove();
		running = 0;
		return;
	}

	if (strncmp("/game", line, 5) == 0)
	{
		/* Skip whitespace */
		int i = 4;
		while (line[i] != '\0' && isspace(line[i])) i++;
		if (line[i] == '\0')
		{
			write(STDOUT_FILENO, "Usage: /game username\n", 29);
			fsync(STDOUT_FILENO);
			rl_redisplay();
			return;
		}
		GString *username = g_string_new(&(line[i]));
		msc_client_game_start(client, username);
		g_string_free(username, TRUE);
		return;
	}

	if (strncmp("/join", line, 5) == 0)
	{
		int i = 5;
		/* Skip whitespace */
		while (line[i] != '\0' && isspace(line[i])) i++;
		if (line[i] == '\0')
		{
			write(STDOUT_FILENO, "Usage: /join chatroom\n", 22);
			fsync(STDOUT_FILENO);
			rl_redisplay();
			return;
		}
		GString *dest_room = g_string_new(&(line[i]));
		msc_client_join_chatroom(client, dest_room);

		if (chatroom) g_string_free(chatroom, TRUE);
		chatroom = dest_room;

		update_prompt();
		return;
	}

	if (strncmp("/list", line, 5) == 0)
	{
		msc_client_list_chatrooms(client);
		return;
	}

	if (strncmp("/roll", line, 5) == 0)
	{
		msc_client_game_roll(client);
		return;
	}

	if (strncmp("/say", line, 4) == 0)
	{
		/* Skip whitespace */
		int i = 4;
		while (line[i] != '\0' && isspace(line[i])) i++;
		if (line[i] == '\0')
		{
			write(STDOUT_FILENO, "Usage: /say username message\n", 29);
			fsync(STDOUT_FILENO);
			rl_redisplay();
			return;
		}
		/* Skip username */
		int j = i+1;
		while (line[j] != '\0' && isgraph(line[j])) j++;
		int i_end = j;
		/* Skip whitespace */
		while (line[j] != '\0' && isspace(line[j])) j++;
		if (line[j] == '\0')
		{
			write(STDOUT_FILENO, "Usage: /say username message\n", 29);
			fsync(STDOUT_FILENO);
			rl_redisplay();
			return;
		}
		GString *receiver = g_string_new_len(&(line[i]), i_end - i);
		GString *message = g_string_new(&(line[j]));

		msc_client_say(client, receiver, message);

		GString *display_line = g_string_new("");
		g_string_printf(display_line, "You whisper to %s: %s\n", receiver->str, message->str);

		write(STDOUT_FILENO, display_line->str, display_line->len);
		fsync(STDOUT_FILENO);
		rl_redisplay();

		g_string_free(display_line, TRUE);
		g_string_free(receiver, TRUE);
		g_string_free(message, TRUE);

		return;
	}

	if (strncmp("/user", line, 5) == 0) {
		int i = 5;
		/* Skip whitespace */
		while (line[i] != '\0' && isspace(line[i])) i++;
		if (line[i] == '\0')
		{
			write(STDOUT_FILENO, "Usage: /user username\n", 22);
			fsync(STDOUT_FILENO);
			rl_redisplay();
			return;
		}
		GString *new_user = g_string_new(&(line[i]));

		char tmp[48];
		getpasswd("Password: ", tmp, 48);
		GString *passwd = g_string_new(tmp);

		msc_client_authenticate(client, new_user, passwd);

		g_string_free(new_user, TRUE);
		g_string_free(passwd, TRUE);
		return;
	}

	if (strncmp("/who", line, 4) == 0)
	{
		msc_client_list_users(client);
		return;
	}

	GString *line_tmp = g_string_new(line);
	if (line_tmp->len)
	{
		if (chatroom->len)
		{
			msc_client_say(client, NULL, line_tmp);
		}
		else
		{
			write(STDOUT_FILENO, "ERROR: You have to join a channel before sending chat messages.\n", 64);
			fsync(STDOUT_FILENO);
		}
	}

	g_string_free(line_tmp, TRUE);
	rl_redisplay();
}

void server_message_callback(MSC_BOOL is_srv_msg, MSC_BOOL is_priv_msg, GString *from_user, GString *from_host, GString *msg)
{
	// https://github.com/thejoshwolfe/consoline/blob/master/consoline.c#L36
	char* saved_line = rl_copy_text(0, rl_end);
	int saved_point = rl_point;

	rl_set_prompt("");
	rl_replace_line("", 0);
	rl_redisplay();

	if (is_srv_msg)
	{
		if (msc_client_authenticated_user_name(client) || msc_client_chatroom(client))
		{
			update_prompt();
		}

		write(STDOUT_FILENO, msg->str, msg->len);
		write(STDOUT_FILENO, "\n", 1);
		fsync(STDOUT_FILENO);
	}
	else if (is_priv_msg)
	{
		GString *line = g_string_new("");
		g_string_printf(line, "%s(%s) whispers: %s\n", from_user->str, from_host->str, msg->str);

		write(STDOUT_FILENO, line->str, line->len);
		fsync(STDOUT_FILENO);

		g_string_free(line, TRUE);
	}
	else
	{
		GString *line = g_string_new("");
		g_string_printf(line, "%s# %s(%s): %s\n", chatroom->str, from_user->str, from_host->str, msg->str);

		write(STDOUT_FILENO, line->str, line->len);
		fsync(STDOUT_FILENO);

		g_string_free(line, TRUE);
	}

	rl_set_prompt(prompt->str);
	rl_replace_line(saved_line, 0);
	rl_point = saved_point;
	rl_redisplay();

	free(saved_line);
}

void print_usage()
{
	fprintf(stderr, "Usage:\n\tchat <hostname> <port>\n");
}

int main(int argc, const char **argv)
{
	if (argc != 3)
	{
		print_usage();
		exit(EXIT_FAILURE);
	}

	setup_signal_handlers();

	const char *hostname = argv[1];
	uint16_t port = (uint16_t)atoi(argv[2]);

	if (!msc_library_init(MSC_LIBRARY_CLIENT))
	{
		fatal_error("Unable to initialize msc library");
		exit(EXIT_FAILURE);
	}

	client = msc_client_create();
	MSC_CLIENT_ERR e = msc_client_connect(client, hostname, port);
	if (e)
	{
		msc_client_destroy(client);
		client = NULL;
		msc_library_finalize();
		fatal_client_error("Unable to connect to server", e);
	}

	user = g_string_new("");
	chatroom = g_string_new("");
	prompt = g_string_new("> ");
	rl_callback_handler_install(prompt->str, readline_callback);
	while (running)
	{
		fd_set rfds;
		struct timeval timeout;

		FD_ZERO(&rfds);
		FD_SET(STDIN_FILENO, &rfds);
		timeout.tv_sec = 0;
		timeout.tv_usec = 1000;

		int r = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &timeout);
		if (r < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			/* Not interrupted, maybe nothing we can do? */
			perror("select()");
			break;
		}

		if (FD_ISSET(STDIN_FILENO, &rfds))
		{
			rl_callback_read_char();
		}

		if (msc_client_pump_messages(client, server_message_callback))
		{
			rl_redisplay();
		}

		GString *authenticated_user = msc_client_authenticated_user_name(client);
		if (authenticated_user && !g_string_equal(authenticated_user, user))
		{
			g_string_truncate(user, 0);
			g_string_append_len(user, authenticated_user->str, authenticated_user->len);
		}
	}

	msc_client_disconnect(client);
	msc_client_destroy(client);
	client = NULL;
	msc_library_finalize();

	g_string_free(user, TRUE);
	g_string_free(chatroom, TRUE);
	g_string_free(prompt, TRUE);

	return 0;
}
