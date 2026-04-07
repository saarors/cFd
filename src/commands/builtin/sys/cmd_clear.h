#ifndef CMD_CLEAR_H
#define CMD_CLEAR_H
#include "../../../core/session.h"
#include "../../../commands/command.h"
int cmd_clear(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_clear;
#endif
