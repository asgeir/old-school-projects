#ifndef CHAT_MSC_COMMON_H
#define CHAT_MSC_COMMON_H

typedef int MSC_BOOL;
typedef int MSC_LIBRARY_MODE;

#define MSC_FALSE (0)
#define MSC_TRUE (!MSC_FALSE)

#define MSC_LIBRARY_CLIENT (0)
#define MSC_LIBRARY_SERVER (1)

#define MSC_CLIENT_MSG_OPCODE_BYE     (0x00)
#define MSC_CLIENT_MSG_OPCODE_MESSAGE (0x01)
#define MSC_CLIENT_MSG_OPCODE_SAY     (0x02)
#define MSC_CLIENT_MSG_OPCODE_WHO     (0x03)
#define MSC_CLIENT_MSG_OPCODE_LIST    (0x04)
#define MSC_CLIENT_MSG_OPCODE_JOIN    (0x05)
#define MSC_CLIENT_MSG_OPCODE_AUTH    (0x06)
#define MSC_CLIENT_MSG_OPCODE_GAME    (0x71)
#define MSC_CLIENT_MSG_OPCODE_ROLL    (0x72)

#define MSC_SERVER_MSG_OPCODE_SERVER   (0)
#define MSC_SERVER_MSG_OPCODE_CHATROOM (1)
#define MSC_SERVER_MSG_OPCODE_PRIVATE  (2)
#define MSC_SERVER_MSG_OPCODE_AUTH1    (3)
#define MSC_SERVER_MSG_OPCODE_AUTH2    (4)

void halt_os_error(const char *msg);
void fatal_error(const char *msg);

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
int getpasswd(const char *prompt, char *passwd, int size);

#endif //CHAT_MSC_COMMON_H
