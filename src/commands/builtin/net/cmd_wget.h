#ifndef CMD_WGET_H
#define CMD_WGET_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_wget(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_wget;

#endif /* CMD_WGET_H */
