#ifndef CMD_RMDIR_H
#define CMD_RMDIR_H
#include "../../../core/session.h"
#include "../../../commands/command.h"
int cmd_rmdir(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_rmdir;
#endif
