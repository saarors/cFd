#include "cmd_fold.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static void fold_line(const char *line, int width, bool break_at_space) {
    int len = (int)strlen(line);
    int pos = 0;

    while (pos < len) {
        int chunk = len - pos;
        if (chunk > width) chunk = width;

        if (break_at_space && pos + chunk < len) {
            /* Find last space within chunk */
            int last_sp = -1;
            for (int i = chunk - 1; i >= 0; i--) {
                if (line[pos + i] == ' ') { last_sp = i; break; }
            }
            if (last_sp > 0) chunk = last_sp + 1; /* include the space at end */
        }

        fwrite(line + pos, 1, (size_t)chunk, stdout);
        putchar('\n');
        pos += chunk;
    }
}

int cmd_fold(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    int         width  = 80;
    bool        spaces = false;
    const char *infile = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
            width = atoi(argv[++i]);
            if (width < 1) width = 1;
        } else if (argv[i][0] == '-' && argv[i][1] == 'w') {
            width = atoi(argv[i] + 2);
            if (width < 1) width = 1;
        } else if (strcmp(argv[i], "-s") == 0) {
            spaces = true;
        } else if (argv[i][0] != '-') {
            infile = argv[i];
        }
    }

    FILE *fp = infile ? fopen(infile, "r") : stdin;
    if (!fp) {
        fprintf(stderr, "fold: %s: No such file or directory\n", infile);
        return 1;
    }

    char buf[65536];
    while (fgets(buf, sizeof(buf), fp)) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len-1] == '\n') buf[--len] = '\0';
        if ((int)len <= width) {
            puts(buf);
        } else {
            fold_line(buf, width, spaces);
        }
    }

    if (infile) fclose(fp);
    return 0;
}

const cfd_command_t builtin_fold = {
    "fold",
    "fold [-w width] [-s] [file]",
    "Wrap lines at a specified width",
    "text",
    cmd_fold,
    0, -1
};
