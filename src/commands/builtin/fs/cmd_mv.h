#ifndef CMD_MV_H
#define CMD_MV_H
#include "../../../core/session.h"
int cmd_mv(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_mv;
#endif
