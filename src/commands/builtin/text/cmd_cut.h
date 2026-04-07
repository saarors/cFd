#ifndef CMD_CUT_H
#define CMD_CUT_H
#include "../../../core/session.h"
#include "../../../commands/command.h"
int cmd_cut(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_cut;
#endif
