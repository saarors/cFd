#ifndef CMD_STAT_H
#define CMD_STAT_H
#include "../../../core/session.h"
int cmd_stat(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_stat;
#endif
