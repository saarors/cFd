#ifndef CMD_XARGS_H
#define CMD_XARGS_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_xargs(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_xargs;

#endif /* CMD_XARGS_H */
