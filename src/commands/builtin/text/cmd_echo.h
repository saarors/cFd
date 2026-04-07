#ifndef CMD_ECHO_H
#define CMD_ECHO_H
#include "../../../core/session.h"
int cmd_echo(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_echo;
#endif
