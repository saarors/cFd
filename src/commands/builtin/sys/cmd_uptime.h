#ifndef CMD_UPTIME_H
#define CMD_UPTIME_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_uptime(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_uptime;

#endif /* CMD_UPTIME_H */
