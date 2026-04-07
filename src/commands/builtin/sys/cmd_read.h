#ifndef CMD_READ_H
#define CMD_READ_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_read(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_read;

#endif /* CMD_READ_H */
