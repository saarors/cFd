#ifndef CMD_TR_H
#define CMD_TR_H
#include "../../../core/session.h"
#include "../../../commands/command.h"
int cmd_tr(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_tr;
#endif
