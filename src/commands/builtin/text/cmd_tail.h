#ifndef CMD_TAIL_H
#define CMD_TAIL_H
#include "../../../core/session.h"
#include "../../../commands/command.h"
int cmd_tail(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_tail;
#endif
