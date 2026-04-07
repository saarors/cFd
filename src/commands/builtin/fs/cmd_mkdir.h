#ifndef CMD_MKDIR_H
#define CMD_MKDIR_H
#include "../../../core/session.h"
int cmd_mkdir(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_mkdir;
#endif
