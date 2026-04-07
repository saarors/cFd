#ifndef CMD_TEE_H
#define CMD_TEE_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_tee(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_tee;

#endif /* CMD_TEE_H */
