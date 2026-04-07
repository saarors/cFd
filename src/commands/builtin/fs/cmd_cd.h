#ifndef CMD_CD_H
#define CMD_CD_H
#include "../../../core/session.h"
int cmd_cd(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_cd;
#endif
