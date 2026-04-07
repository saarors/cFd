#ifndef CMD_WC_H
#define CMD_WC_H
#include "../../../core/session.h"
int cmd_wc(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_wc;
#endif
