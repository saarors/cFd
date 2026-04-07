#ifndef CMD_HELP_H
#define CMD_HELP_H
#include "../../../core/session.h"
#include "../../../commands/command.h"
int cmd_help(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_help;
#endif
