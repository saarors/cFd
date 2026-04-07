#ifndef CMD_LS_H
#define CMD_LS_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_ls(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_ls;

#endif
