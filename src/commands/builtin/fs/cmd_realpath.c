#include "cmd_realpath.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <limits.h>
#  include <stdlib.h>
#endif

int cmd_realpath(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    if (argc < 2) {
        fprintf(stderr, "Usage: realpath <path...>\n");
        return 1;
    }

    int ret = 0;
    for (int i = 1; i < argc; i++) {
#ifdef _WIN32
        char out[MAX_PATH];
        DWORD r = GetFullPathNameA(argv[i], sizeof(out), out, NULL);
        if (r == 0 || r >= sizeof(out)) {
            fprintf(stderr, "realpath: %s: cannot resolve path\n", argv[i]);
            ret = 1;
        } else {
            printf("%s\n", out);
        }
#else
        char out[PATH_MAX];
        if (!realpath(argv[i], out)) {
            fprintf(stderr, "realpath: %s: %m\n", argv[i]);
            ret = 1;
        } else {
            printf("%s\n", out);
        }
#endif
    }
    return ret;
}

const cfd_command_t builtin_realpath = {
    "realpath",
    "realpath <path...>",
    "Print the resolved absolute path",
    "fs",
    cmd_realpath,
    1, -1
};
