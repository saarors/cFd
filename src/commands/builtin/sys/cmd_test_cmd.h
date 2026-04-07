#ifndef CMD_TEST_CMD_H
#define CMD_TEST_CMD_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_test_cmd(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_test;

#endif /* CMD_TEST_CMD_H */
