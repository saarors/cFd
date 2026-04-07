#ifndef CMD_WAIT_H
#define CMD_WAIT_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_wait(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_wait;

#endif /* CMD_WAIT_H */
