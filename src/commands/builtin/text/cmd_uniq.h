#ifndef CMD_UNIQ_H
#define CMD_UNIQ_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_uniq(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_uniq;

#endif /* CMD_UNIQ_H */
