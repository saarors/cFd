#ifndef CMD_ENV_H
#define CMD_ENV_H
#include "../../../core/session.h"
#include "../../../commands/command.h"
int cmd_env(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_env;
#endif
