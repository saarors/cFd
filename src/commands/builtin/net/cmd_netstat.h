#ifndef CMD_NETSTAT_H
#define CMD_NETSTAT_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_netstat(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_netstat;

#endif /* CMD_NETSTAT_H */
