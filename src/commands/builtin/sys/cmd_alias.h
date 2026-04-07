#ifndef CMD_ALIAS_H
#define CMD_ALIAS_H
#include "../../../core/session.h"
#include "../../../commands/command.h"
int cmd_alias(cfd_session_t *sess, int argc, char **argv);
int cmd_unalias(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_alias;
extern const cfd_command_t builtin_unalias;
#endif
