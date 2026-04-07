#include "cmd_env.h"
#include "../../../platform/platform.h"
#include <stdio.h>
#include <string.h>
int cmd_env(cfd_session_t *sess, int argc, char **argv) {
    (void)sess; (void)argc; (void)argv;
    char **env = cfd_platform_environ();
    if (!env) return 0;
    for (int i=0; env[i]; i++) puts(env[i]);
    return 0;
}
const cfd_command_t builtin_env = {
    "env","env","Print environment variables","system",cmd_env,0,0
};
