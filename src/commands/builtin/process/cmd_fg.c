#include "cmd_fg.h"
#include <stdio.h>
int cmd_fg(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;(void)argc;(void)argv;
    printf("fg: no background jobs to bring forward\n");
    return 0;
}
const cfd_command_t builtin_fg = {
    "fg","fg [job]","Bring job to foreground","process",cmd_fg,0,1
};
