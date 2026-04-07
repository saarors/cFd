#ifndef CMD_REALPATH_H
#define CMD_REALPATH_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_realpath(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_realpath;

#endif /* CMD_REALPATH_H */
