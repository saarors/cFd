#ifndef CMD_FIND_H
#define CMD_FIND_H
#include "../../../core/session.h"
#include "../../../commands/command.h"
int cmd_find(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_find;
#endif
