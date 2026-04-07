#ifndef CMD_TYPE_H
#define CMD_TYPE_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_type(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_type;

#endif /* CMD_TYPE_H */
