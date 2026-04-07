#ifndef CMD_COLUMN_H
#define CMD_COLUMN_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_column(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_column;

#endif /* CMD_COLUMN_H */
