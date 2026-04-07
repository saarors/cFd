#ifndef CMD_KILL_H
#define CMD_KILL_H
#include "../../../core/session.h"
int cmd_kill(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_kill;
#endif
