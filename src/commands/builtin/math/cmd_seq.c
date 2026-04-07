#include "cmd_seq.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

int cmd_seq(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    if (argc < 2) {
        fprintf(stderr, "Usage: seq [first [increment]] last\n");
        fprintf(stderr, "  -s <sep>     Use separator (default: newline)\n");
        fprintf(stderr, "  -f <fmt>     Printf format string\n");
        fprintf(stderr, "  -w           Equal width (zero-pad)\n");
        return 1;
    }

    const char *sep     = "\n";
    const char *fmt     = NULL;
    bool        equal_w = false;

    double first = 1.0, incr = 1.0, last = 0.0;
    int    num_pos = 0; /* number of positional args so far */
    double pos[3];

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            sep = argv[++i];
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            fmt = argv[++i];
        } else if (strcmp(argv[i], "-w") == 0) {
            equal_w = true;
        } else if (argv[i][0] != '-') {
            if (num_pos < 3)
                pos[num_pos++] = atof(argv[i]);
        }
    }

    switch (num_pos) {
        case 1:
            first = 1.0; incr = 1.0; last = pos[0];
            break;
        case 2:
            first = pos[0]; incr = 1.0; last = pos[1];
            break;
        case 3:
            first = pos[0]; incr = pos[1]; last = pos[2];
            break;
        default:
            fprintf(stderr, "seq: invalid arguments\n");
            return 1;
    }

    if (incr == 0.0) {
        fprintf(stderr, "seq: increment must be non-zero\n");
        return 1;
    }

    /* Determine width for -w option */
    int width = 0;
    if (equal_w && !fmt) {
        /* Width based on largest number */
        char tbuf[64];
        snprintf(tbuf, sizeof(tbuf), "%.0f", last);
        width = (int)strlen(tbuf);
        if (first < 0) {
            snprintf(tbuf, sizeof(tbuf), "%.0f", first);
            int w2 = (int)strlen(tbuf);
            if (w2 > width) width = w2;
        }
    }

    bool first_item = true;
    double val = first;

    /* Iterate */
    while ((incr > 0 && val <= last + 1e-10) ||
           (incr < 0 && val >= last - 1e-10)) {

        if (!first_item)
            fputs(sep, stdout);
        first_item = false;

        if (fmt) {
            printf(fmt, val);
        } else {
            /* Check if all values are integers */
            bool is_int = (floor(val) == val &&
                           floor(first) == first &&
                           floor(incr) == incr);
            if (is_int) {
                if (equal_w && width > 0)
                    printf("%0*lld", width, (long long)val);
                else
                    printf("%lld", (long long)val);
            } else {
                /* Determine decimal places from increment */
                char inc_str[64];
                snprintf(inc_str, sizeof(inc_str), "%g", incr);
                int decimals = 0;
                char *dot = strchr(inc_str, '.');
                if (dot) decimals = (int)strlen(dot + 1);
                printf("%.*f", decimals, val);
            }
        }

        val += incr;

        /* Guard against floating point overshoot causing infinite loop */
        if (fabs(val - first) > fabs(last - first) + fabs(incr) * 2)
            break;
    }

    if (!first_item)
        putchar('\n');

    return 0;
}

const cfd_command_t builtin_seq = {
    "seq",
    "seq [-s sep] [-f fmt] [-w] [first [incr]] last",
    "Print a sequence of numbers",
    "math",
    cmd_seq,
    1, -1
};
