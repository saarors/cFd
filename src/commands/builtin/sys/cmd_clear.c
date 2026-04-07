#include "cmd_clear.h"
#include "../../../platform/platform.h"
int cmd_clear(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;(void)argc;(void)argv;
    cfd_platform_clear_screen();
    return 0;
}
const cfd_command_t builtin_clear = {
    "clear","clear","Clear the terminal screen","system",cmd_clear,0,0
};
