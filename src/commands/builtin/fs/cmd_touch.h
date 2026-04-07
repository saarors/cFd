#ifndef CMD_TOUCH_H
#define CMD_TOUCH_H
#include "../../../core/session.h"
#include "../../../commands/command.h"
int cmd_touch(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_touch;
#endif
