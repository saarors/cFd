#include "cmd_netstat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int cmd_netstat(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    bool flag_a = false, flag_n = false, flag_t = false, flag_u = false;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            for (int j = 1; argv[i][j]; j++) {
                switch (argv[i][j]) {
                    case 'a': flag_a = true; break;
                    case 'n': flag_n = true; break;
                    case 't': flag_t = true; break;
                    case 'u': flag_u = true; break;
                    default: break;
                }
            }
        }
    }

    char cmd[256];

#ifdef _WIN32
    /* Build flags for Windows netstat */
    char flags[64] = "";
    if (flag_a) strncat(flags, "a", sizeof(flags) - strlen(flags) - 1);
    if (flag_n) strncat(flags, "n", sizeof(flags) - strlen(flags) - 1);
    if (flag_a || flag_n)
        snprintf(cmd, sizeof(cmd), "netstat -%so", flags);
    else
        snprintf(cmd, sizeof(cmd), "netstat -ano");
#else
    /* Build flags for Unix netstat / ss */
    char flags[64] = "-";
    if (flag_t) strncat(flags, "t", sizeof(flags) - strlen(flags) - 1);
    if (flag_u) strncat(flags, "u", sizeof(flags) - strlen(flags) - 1);
    if (flag_a) strncat(flags, "l", sizeof(flags) - strlen(flags) - 1);
    if (flag_n) strncat(flags, "n", sizeof(flags) - strlen(flags) - 1);
    /* default if nothing specified */
    if (strlen(flags) <= 1) strncat(flags, "tuln", sizeof(flags) - strlen(flags) - 1);
    snprintf(cmd, sizeof(cmd), "netstat %s 2>/dev/null || ss %s", flags, flags);
#endif

    int ret = system(cmd);
    return (ret == 0) ? 0 : 1;
}

const cfd_command_t builtin_netstat = {
    "netstat",
    "netstat [-a] [-n] [-t] [-u]",
    "Display network connections and statistics",
    "net",
    cmd_netstat,
    0, -1
};
