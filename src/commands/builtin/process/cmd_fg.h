#ifndef CMD_FG_H
#define CMD_FG_H
#include "../../../core/session.h"
#include "../../../commands/command.h"
int cmd_fg(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_fg;
#endif
