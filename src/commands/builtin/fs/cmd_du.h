#ifndef CMD_DU_H
#define CMD_DU_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_du(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_du;

#endif /* CMD_DU_H */
