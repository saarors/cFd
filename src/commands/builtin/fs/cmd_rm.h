#ifndef CMD_RM_H
#define CMD_RM_H
#include "../../../core/session.h"
int cmd_rm(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_rm;
#endif
