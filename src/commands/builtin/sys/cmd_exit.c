#include "cmd_exit.h"
#include <stdlib.h>
int cmd_exit(cfd_session_t *sess, int argc, char **argv) {
    int code = (argc > 1) ? atoi(argv[1]) : sess->last_exit;
    sess->running = false;
    sess->exit_code = code;
    return code;
}
const cfd_command_t builtin_exit = {
    "exit","exit [code]","Exit the terminal","system",cmd_exit,0,1
};
