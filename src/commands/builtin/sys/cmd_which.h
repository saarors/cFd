#ifndef CMD_WHICH_H
#define CMD_WHICH_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_which(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_which;

#endif /* CMD_WHICH_H */
