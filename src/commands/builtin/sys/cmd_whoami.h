#ifndef CMD_WHOAMI_H
#define CMD_WHOAMI_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_whoami(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_whoami;

#endif /* CMD_WHOAMI_H */
