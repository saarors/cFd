#ifndef CMD_NANO_H
#define CMD_NANO_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_nano(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_nano;

#endif /* CMD_NANO_H */
