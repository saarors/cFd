#include "cmd_hostname.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#endif

int cmd_hostname(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    bool short_name = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) short_name = true;
    }

    char buf[512];

#ifdef _WIN32
    DWORD sz = sizeof(buf);
    if (!GetComputerNameA(buf, &sz)) {
        const char *h = getenv("COMPUTERNAME");
        if (h) strncpy(buf, h, sizeof(buf) - 1);
        else   strncpy(buf, "unknown", sizeof(buf) - 1);
    }
#else
    if (gethostname(buf, sizeof(buf)) != 0) {
        const char *h = getenv("HOSTNAME");
        if (h) strncpy(buf, h, sizeof(buf) - 1);
        else   strncpy(buf, "unknown", sizeof(buf) - 1);
    }
#endif

    buf[sizeof(buf) - 1] = '\0';

    if (short_name) {
        /* Strip domain: truncate at first '.' */
        char *dot = strchr(buf, '.');
        if (dot) *dot = '\0';
    }

    printf("%s\n", buf);
    return 0;
}

const cfd_command_t builtin_hostname = {
    "hostname",
    "hostname [-s]",
    "Print the system hostname",
    "system",
    cmd_hostname,
    0, -1
};
