#include "cmd_uptime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <time.h>
#endif

int cmd_uptime(cfd_session_t *sess, int argc, char **argv) {
    (void)sess; (void)argc; (void)argv;

    unsigned long long seconds = 0;

#ifdef _WIN32
    ULONGLONG ms = GetTickCount64();
    seconds = ms / 1000ULL;
#else
    FILE *fp = fopen("/proc/uptime", "r");
    if (fp) {
        double up;
        if (fscanf(fp, "%lf", &up) == 1)
            seconds = (unsigned long long)up;
        fclose(fp);
    } else {
        /* Fallback: use clock() (not accurate for system uptime) */
        seconds = (unsigned long long)clock() / (unsigned long long)CLOCKS_PER_SEC;
    }
#endif

    unsigned long long days  = seconds / 86400ULL;
    unsigned long long hours = (seconds % 86400ULL) / 3600ULL;
    unsigned long long mins  = (seconds % 3600ULL) / 60ULL;

    if (days > 0)
        printf("up %llu day%s, %02llu:%02llu\n",
               days, days == 1 ? "" : "s", hours, mins);
    else
        printf("up %02llu:%02llu\n", hours, mins);

    return 0;
}

const cfd_command_t builtin_uptime = {
    "uptime",
    "uptime",
    "Show how long the system has been running",
    "system",
    cmd_uptime,
    0, 0
};
