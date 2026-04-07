#include "cmd_ping.h"
#include "../../../utils/mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cmd_ping(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    if (argc < 2) {
        fprintf(stderr, "Usage: ping [-c count] [-i interval] <host>\n");
        return 1;
    }

    int   count    = 4;
    const char *host = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            count = atoi(argv[++i]);
            if (count < 1) count = 1;
        } else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            /* interval: accepted but ignored (system ping handles it) */
            i++;
        } else if (argv[i][0] != '-') {
            host = argv[i];
        }
    }

    if (!host) {
        fprintf(stderr, "ping: no host specified\n");
        return 1;
    }

    char cmd[512];
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "ping -n %d %s", count, host);
#else
    snprintf(cmd, sizeof(cmd), "ping -c %d %s", count, host);
#endif

    int ret = system(cmd);
    return (ret == 0) ? 0 : 1;
}

const cfd_command_t builtin_ping = {
    "ping",
    "ping [-c count] [-i interval] <host>",
    "Send ICMP echo requests to a host",
    "net",
    cmd_ping,
    1, -1
};
