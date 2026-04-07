#include "cmd_watch.h"
#include "../../../platform/platform.h"
#include "../../../utils/mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <signal.h>

/* Signal flag for SIGINT */
static volatile int watch_interrupted = 0;

#ifndef _WIN32
static void watch_sigint(int sig) {
    (void)sig;
    watch_interrupted = 1;
}
#endif

int cmd_watch(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    if (argc < 2) {
        fprintf(stderr, "Usage: watch [-n seconds] <command...>\n");
        return 1;
    }

    double interval = 2.0;
    int    cmd_start = 1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            interval = atof(argv[++i]);
            if (interval < 0.1) interval = 0.1;
            cmd_start = i + 1;
        } else {
            cmd_start = i;
            break;
        }
    }

    if (cmd_start >= argc) {
        fprintf(stderr, "watch: no command specified\n");
        return 1;
    }

    /* Build command string */
    char cmd_str[2048];
    cmd_str[0] = '\0';
    for (int i = cmd_start; i < argc; i++) {
        if (i > cmd_start) strncat(cmd_str, " ", sizeof(cmd_str) - strlen(cmd_str) - 1);
        strncat(cmd_str, argv[i], sizeof(cmd_str) - strlen(cmd_str) - 1);
    }

    unsigned interval_ms = (unsigned)(interval * 1000.0);

#ifndef _WIN32
    signal(SIGINT, watch_sigint);
#endif

    while (!watch_interrupted) {
        cfd_platform_clear_screen();

        /* Print header */
        time_t now = time(NULL);
        char time_str[64];
        struct tm *tm_info = localtime(&now);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
        printf("Every %.1fs: %-40s  %s\n\n", interval, cmd_str, time_str);

        /* Run command */
        system(cmd_str);

        /* Sleep */
        unsigned slept = 0;
        unsigned chunk = 100; /* check interrupt every 100ms */
        while (!watch_interrupted && slept < interval_ms) {
            cfd_platform_sleep_ms(chunk);
            slept += chunk;
        }
    }

#ifndef _WIN32
    signal(SIGINT, SIG_DFL);
#endif
    watch_interrupted = 0;
    putchar('\n');
    return 0;
}

const cfd_command_t builtin_watch = {
    "watch",
    "watch [-n seconds] <command...>",
    "Execute a command repeatedly and show output",
    "system",
    cmd_watch,
    1, -1
};
