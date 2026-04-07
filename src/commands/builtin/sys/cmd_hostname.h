#ifndef CMD_HOSTNAME_H
#define CMD_HOSTNAME_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_hostname(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_hostname;

#endif /* CMD_HOSTNAME_H */
