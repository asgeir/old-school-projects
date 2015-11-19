#include "msc_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <termios.h>

void halt_os_error(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

void fatal_error(const char *msg)
{
	fprintf(stderr, "%s", msg);
	exit(EXIT_FAILURE);
}

/* To read a password without echoing it to the console.
 *
 * We assume that stdin is not redirected to a pipe and we won't
 * access tty directly. It does not make much sense for this program
 * to redirect input and output.
 *
 * This function is not safe to termination. If the program
 * crashes during getpasswd or gets terminated, then echoing
 * may remain disabled for the shell (that depends on shell,
 * operating system and C library). To restore echoing,
 * type 'reset' into the sell and press enter.
 */
int getpasswd(const char *prompt, char *passwd, int size)
{
	struct termios old_flags;
	struct termios new_flags;

	if (size < 0)
	{
		return -1;
	}

	/* Clear out the buffer content. */
	memset(passwd, 0, (size_t)size);

	/* Disable echo. */
	tcgetattr(fileno(stdin), &old_flags);
	memcpy(&new_flags, &old_flags, sizeof(old_flags));
	new_flags.c_lflag &= ~ECHO;
	new_flags.c_lflag |= ECHONL;

	if (tcsetattr(fileno(stdin), TCSANOW, &new_flags) != 0)
	{
		halt_os_error("Could not disable echo");
		return -1;
	}

	printf("%s", prompt);
	fgets(passwd, size, stdin);

	/* The result in passwd is '\0' terminated and may contain a final
	 * '\n'. If it exists, we remove it.
	 */
	if (passwd[strlen(passwd) - 1] == '\n')
	{
		passwd[strlen(passwd) - 1] = '\0';
	}

	/* Restore the terminal */
	if (tcsetattr(fileno(stdin), TCSANOW, &old_flags) != 0)
	{
		halt_os_error("Could not restore terminal attributes");
		return -1;
	}

	return 0;
}
