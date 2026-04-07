#ifndef CMD_DIFF_H
#define CMD_DIFF_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_diff(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_diff;

#endif /* CMD_DIFF_H */
