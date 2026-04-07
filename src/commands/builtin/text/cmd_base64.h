#ifndef CMD_BASE64_H
#define CMD_BASE64_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_base64(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_base64;

#endif /* CMD_BASE64_H */
