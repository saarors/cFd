#ifndef CMD_FOLD_H
#define CMD_FOLD_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_fold(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_fold;

#endif /* CMD_FOLD_H */
