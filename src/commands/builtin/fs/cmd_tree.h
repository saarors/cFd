#ifndef CMD_TREE_H
#define CMD_TREE_H

#include "../../../core/session.h"
#include "../../../commands/command.h"

int cmd_tree(cfd_session_t *sess, int argc, char **argv);
extern const cfd_command_t builtin_tree;

#endif /* CMD_TREE_H */
