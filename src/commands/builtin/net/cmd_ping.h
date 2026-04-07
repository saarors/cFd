#ifndef CMD_PING_H
#define CMD_PING_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_ping(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_ping;

#endif /* CMD_PING_H */
