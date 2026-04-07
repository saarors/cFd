#ifndef CMD_CURL_H
#define CMD_CURL_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_curl(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_curl;

#endif /* CMD_CURL_H */
