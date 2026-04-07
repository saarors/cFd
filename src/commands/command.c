#include "command.h"
#include <stdio.h>

int cfd_cmd_check_args(const cfd_command_t *cmd, int argc) {
    if (!cmd) return -1;
    /* argc includes argv[0] (command name) */
    int n = argc - 1;
    if (n < cmd->min_args) {
        fprintf(stderr, "%s: too few arguments\nUsage: %s\n", cmd->name, cmd->usage);
        return -1;
    }
    if (cmd->max_args >= 0 && n > cmd->max_args) {
        fprintf(stderr, "%s: too many arguments\nUsage: %s\n", cmd->name, cmd->usage);
        return -1;
    }
    return 0;
}
