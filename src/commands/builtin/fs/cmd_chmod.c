#include "cmd_chmod.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#ifdef _WIN32
#  include <sys/stat.h>
#  include <io.h>
#else
#  include <sys/stat.h>
#  include <sys/types.h>
#endif

/* Parse numeric mode like "755" */
static int parse_numeric_mode(const char *mode_str, mode_t *out) {
    char *end;
    long val = strtol(mode_str, &end, 8);
    if (*end != '\0' || val < 0 || val > 07777) return -1;
    *out = (mode_t)val;
    return 0;
}

#ifndef _WIN32
/* Parse symbolic mode like "u+x", "a-w", "go+r" */
static int apply_symbolic_mode(const char *mode_str, const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1;

    mode_t m = st.st_mode;
    const char *p = mode_str;

    while (*p) {
        /* Parse who: u, g, o, a */
        int who = 0; /* bitmask: 4=u, 2=g, 1=o */
        while (*p == 'u' || *p == 'g' || *p == 'o' || *p == 'a') {
            if (*p == 'u') who |= 4;
            else if (*p == 'g') who |= 2;
            else if (*p == 'o') who |= 1;
            else if (*p == 'a') who = 7;
            p++;
        }
        if (!who) who = 7; /* default: all */

        /* Operator */
        char op = *p;
        if (op != '+' && op != '-' && op != '=') {
            return -1;
        }
        p++;

        /* Permissions */
        mode_t perms = 0;
        while (*p && *p != ',' && *p != '+' && *p != '-' && *p != '=') {
            switch (*p) {
                case 'r': perms |= 04; break;
                case 'w': perms |= 02; break;
                case 'x': perms |= 01; break;
                default:  break;
            }
            p++;
        }

        /* Apply */
        for (int b = 0; b < 3; b++) {
            if (!(who & (4 >> b))) continue; /* u=4,g=2,o=1 maps to shifts 6,3,0 */
            int shift = (2 - b) * 3;
            mode_t mask = perms << shift;

            if (op == '+') m |= mask;
            else if (op == '-') m &= ~mask;
            else { /* = */ m = (m & ~(07 << shift)) | mask; }
        }

        if (*p == ',') p++;
    }

    return chmod(path, m);
}
#endif

int cmd_chmod(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    if (argc < 3) {
        fprintf(stderr, "Usage: chmod <mode> <file...>\n");
        fprintf(stderr, "  mode: numeric (755) or symbolic (u+x, a-w)\n");
        return 1;
    }

    const char *mode_str = argv[1];
    int ret = 0;

#ifdef _WIN32
    /* On Windows, only support read-only bit via _chmod */
    printf("chmod: limited support on Windows (can only set read-only attribute)\n");

    for (int i = 2; i < argc; i++) {
        mode_t m = 0644;
        if (parse_numeric_mode(mode_str, &m) == 0) {
            /* _chmod only supports _S_IREAD and _S_IWRITE */
            int win_mode = 0;
            if (m & 0400) win_mode |= _S_IREAD;
            if (m & 0200) win_mode |= _S_IWRITE;
            if (_chmod(argv[i], win_mode) != 0) {
                fprintf(stderr, "chmod: %s: cannot change permissions\n", argv[i]);
                ret = 1;
            }
        }
    }
#else
    for (int i = 2; i < argc; i++) {
        /* Try numeric first */
        mode_t m = 0;
        if (isdigit((unsigned char)mode_str[0])) {
            if (parse_numeric_mode(mode_str, &m) != 0) {
                fprintf(stderr, "chmod: invalid mode: %s\n", mode_str);
                ret = 1;
                continue;
            }
            if (chmod(argv[i], m) != 0) {
                perror("chmod");
                ret = 1;
            }
        } else {
            /* Symbolic mode */
            if (apply_symbolic_mode(mode_str, argv[i]) != 0) {
                fprintf(stderr, "chmod: cannot change mode of '%s'\n", argv[i]);
                ret = 1;
            }
        }
    }
#endif

    return ret;
}

const cfd_command_t builtin_chmod = {
    "chmod",
    "chmod <mode> <file...>",
    "Change file mode bits",
    "fs",
    cmd_chmod,
    2, -1
};
