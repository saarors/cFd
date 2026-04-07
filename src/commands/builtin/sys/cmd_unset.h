#ifndef CMD_UNSET_H
#define CMD_UNSET_H
#include "../../../core/session.h"
#include "../../../commands/command.h"
int cmd_unset(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_unset;
#endif
