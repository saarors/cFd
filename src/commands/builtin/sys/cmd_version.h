#ifndef CMD_VERSION_H
#define CMD_VERSION_H
#include "../../../core/session.h"
int cmd_version(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_version;
#endif
