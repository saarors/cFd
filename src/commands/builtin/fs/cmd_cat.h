#ifndef CMD_CAT_H
#define CMD_CAT_H
#include "../../../core/session.h"
int cmd_cat(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_cat;
#endif
