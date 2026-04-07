#ifndef CMD_EXPR_H
#define CMD_EXPR_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_expr(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_expr;

#endif /* CMD_EXPR_H */
