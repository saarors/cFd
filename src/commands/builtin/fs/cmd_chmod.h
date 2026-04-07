#ifndef CMD_CHMOD_H
#define CMD_CHMOD_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_chmod(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_chmod;

#endif /* CMD_CHMOD_H */
