#ifndef CMD_DF_H
#define CMD_DF_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_df(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_df;

#endif /* CMD_DF_H */
