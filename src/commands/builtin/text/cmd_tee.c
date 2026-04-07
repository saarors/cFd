#include "cmd_tee.h"
#include "../../../utils/mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int cmd_tee(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    bool append   = false;
    int  file_start = 1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0) {
            append = true;
            file_start = i + 1;
        } else {
            file_start = i;
            break;
        }
    }

    int  file_count = argc - file_start;
    FILE **fps = NULL;

    if (file_count > 0) {
        fps = (FILE **)cfd_calloc((size_t)file_count, sizeof(FILE *));
        if (!fps) {
            fprintf(stderr, "tee: out of memory\n");
            return 1;
        }
        for (int i = 0; i < file_count; i++) {
            fps[i] = fopen(argv[file_start + i], append ? "a" : "w");
            if (!fps[i]) {
                fprintf(stderr, "tee: %s: cannot open file\n", argv[file_start + i]);
                /* Continue, but note the failure */
            }
        }
    }

    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), stdin)) > 0) {
        /* Write to stdout */
        fwrite(buf, 1, n, stdout);
        /* Write to each file */
        for (int i = 0; i < file_count; i++) {
            if (fps && fps[i])
                fwrite(buf, 1, n, fps[i]);
        }
    }

    for (int i = 0; i < file_count; i++) {
        if (fps && fps[i])
            fclose(fps[i]);
    }
    cfd_free(fps);

    return 0;
}

const cfd_command_t builtin_tee = {
    "tee",
    "tee [-a] [file...]",
    "Read stdin and write to stdout and files",
    "text",
    cmd_tee,
    0, -1
};
