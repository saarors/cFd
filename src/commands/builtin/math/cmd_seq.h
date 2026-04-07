#ifndef CMD_SEQ_H
#define CMD_SEQ_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_seq(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_seq;

#endif /* CMD_SEQ_H */
