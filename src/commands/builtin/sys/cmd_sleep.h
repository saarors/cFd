#ifndef CMD_SLEEP_H
#define CMD_SLEEP_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_sleep(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_sleep;

#endif /* CMD_SLEEP_H */
