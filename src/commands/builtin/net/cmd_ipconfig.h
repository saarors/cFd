#ifndef CMD_IPCONFIG_H
#define CMD_IPCONFIG_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_ipconfig(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_ipconfig;

#endif /* CMD_IPCONFIG_H */
