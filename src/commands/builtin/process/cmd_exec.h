#ifndef CMD_EXEC_H
#define CMD_EXEC_H
#include "../../../core/session.h"
int cmd_exec(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_exec;
#endif
