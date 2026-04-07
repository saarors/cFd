#ifndef CMD_HISTORY_H
#define CMD_HISTORY_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_history(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_history;

#endif /* CMD_HISTORY_H */
