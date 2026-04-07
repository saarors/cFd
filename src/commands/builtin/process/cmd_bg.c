#include "cmd_bg.h"
#include <stdio.h>
#include <stdlib.h>
int cmd_bg(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;(void)argc;(void)argv;
    printf("bg: background jobs not fully supported on this platform\n");
    return 0;
}
const cfd_command_t builtin_bg = {
    "bg","bg [job]","Resume job in background","process",cmd_bg,0,1
};
