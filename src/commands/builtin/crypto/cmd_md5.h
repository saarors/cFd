#ifndef CMD_MD5_H
#define CMD_MD5_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_md5(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_md5;

#endif /* CMD_MD5_H */
