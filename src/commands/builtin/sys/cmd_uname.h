#ifndef CMD_UNAME_H
#define CMD_UNAME_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_uname(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_uname;

#endif /* CMD_UNAME_H */
