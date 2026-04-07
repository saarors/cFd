#ifndef CMD_SET_H
#define CMD_SET_H
#include "../../../core/session.h"
int cmd_set(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_set;
#endif
