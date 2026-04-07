#ifndef CMD_CP_H
#define CMD_CP_H
#include "../../../core/session.h"
int cmd_cp(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_cp;
#endif
