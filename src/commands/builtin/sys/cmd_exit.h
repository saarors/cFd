#ifndef CMD_EXIT_H
#define CMD_EXIT_H
#include "../../../core/session.h"
int cmd_exit(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_exit;
#endif
