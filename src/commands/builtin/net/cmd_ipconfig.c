#include "cmd_ipconfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int cmd_ipconfig(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    char cmd[256];

#ifdef _WIN32
    bool all = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "/all") == 0 || strcmp(argv[i], "-all") == 0)
            all = true;
    }
    if (all)
        snprintf(cmd, sizeof(cmd), "ipconfig /all");
    else
        snprintf(cmd, sizeof(cmd), "ipconfig");
#else
    /* On Unix, prefer ip addr; fall back to ifconfig */
    snprintf(cmd, sizeof(cmd), "ifconfig 2>/dev/null || ip addr");
    (void)argc; (void)argv;
#endif

    int ret = system(cmd);
    return (ret == 0) ? 0 : 1;
}

const cfd_command_t builtin_ipconfig = {
    "ipconfig",
    "ipconfig [/all]",
    "Display network interface configuration",
    "net",
    cmd_ipconfig,
    0, -1
};
