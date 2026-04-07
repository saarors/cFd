#include "cmd_sleep.h"
#include "../../../platform/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cmd_sleep(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    if (argc < 2) {
        fprintf(stderr, "Usage: sleep <seconds>\n");
        fprintf(stderr, "  Supports decimal values, e.g. sleep 0.5\n");
        return 1;
    }

    double total_secs = 0.0;
    for (int i = 1; i < argc; i++) {
        char *end;
        double val = strtod(argv[i], &end);
        if (end == argv[i] || val < 0.0) {
            fprintf(stderr, "sleep: invalid time interval '%s'\n", argv[i]);
            return 1;
        }
        /* Handle optional suffix: s, m, h, d */
        if (*end == 's' || *end == '\0') total_secs += val;
        else if (*end == 'm') total_secs += val * 60.0;
        else if (*end == 'h') total_secs += val * 3600.0;
        else if (*end == 'd') total_secs += val * 86400.0;
        else {
            fprintf(stderr, "sleep: invalid suffix '%c'\n", *end);
            return 1;
        }
    }

    unsigned ms = (unsigned)(total_secs * 1000.0);
    cfd_platform_sleep_ms(ms);
    return 0;
}

const cfd_command_t builtin_sleep = {
    "sleep",
    "sleep <seconds>",
    "Pause execution for the specified number of seconds",
    "system",
    cmd_sleep,
    1, -1
};
