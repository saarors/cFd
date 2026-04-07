#ifndef CMD_SORT_H
#define CMD_SORT_H
#include "../../../core/session.h"
#include "../../../commands/command.h"
int cmd_sort(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_sort;
#endif
