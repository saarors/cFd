#ifndef CMD_HEAD_H
#define CMD_HEAD_H
#include "../../../core/session.h"
int cmd_head(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_head;
#endif
