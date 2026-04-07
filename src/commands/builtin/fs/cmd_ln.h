#ifndef CMD_LN_H
#define CMD_LN_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_ln(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_ln;

#endif /* CMD_LN_H */
