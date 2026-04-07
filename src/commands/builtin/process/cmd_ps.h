#ifndef CMD_PS_H
#define CMD_PS_H
#include "../../../core/session.h"
int cmd_ps(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_ps;
#endif
