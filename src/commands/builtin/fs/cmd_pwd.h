#ifndef CMD_PWD_H
#define CMD_PWD_H
#include "../../../core/session.h"
int cmd_pwd(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_pwd;
#endif
