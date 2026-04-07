#ifndef CMD_SHA256_H
#define CMD_SHA256_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_sha256(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_sha256;

#endif /* CMD_SHA256_H */
