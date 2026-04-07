#ifndef CMD_SOURCE_H
#define CMD_SOURCE_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_source(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_source;

#endif /* CMD_SOURCE_H */
