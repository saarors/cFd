#ifndef CMD_GREP_H
#define CMD_GREP_H
#include "../../../core/session.h"
#include "../../../commands/command.h"
int cmd_grep(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_grep;
#endif
