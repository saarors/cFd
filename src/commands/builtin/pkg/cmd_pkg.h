#ifndef CMD_PKG_H
#define CMD_PKG_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_pkg(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_pkg;

#endif /* CMD_PKG_H */
