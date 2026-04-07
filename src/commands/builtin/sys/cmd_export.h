#ifndef CMD_EXPORT_H
#define CMD_EXPORT_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_export(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_export;

#endif /* CMD_EXPORT_H */
