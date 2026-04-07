#ifndef CMD_CALC_H
#define CMD_CALC_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_calc(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_calc;

#endif /* CMD_CALC_H */
