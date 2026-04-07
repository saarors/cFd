#ifndef CMD_BG_H
#define CMD_BG_H
#include "../../../core/session.h"
#include "../../../commands/command.h"
int cmd_bg(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_bg;
#endif
