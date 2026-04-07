#ifndef CMD_WATCH_H
#define CMD_WATCH_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_watch(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_watch;

#endif /* CMD_WATCH_H */
