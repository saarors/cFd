#include "cmd_source.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int cmd_source(cfd_session_t *sess, int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: source <file>\n");
        return 1;
    }

    const char *filename = argv[1];
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "source: %s: No such file or directory\n", filename);
        return 1;
    }

    char line[4096];
    int  lineno = 0;
    int  ret    = 0;

    while (fgets(line, sizeof(line), fp)) {
        lineno++;

        /* Strip trailing newline */
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[--len] = '\0';

        /* Skip empty lines and comments */
        const char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0' || *p == '#') continue;

        /* Execute in current session */
        ret = cfd_session_exec_string(sess, line);

        /* Stop if session exits */
        if (!sess->running) break;
    }

    fclose(fp);
    return ret;
}

const cfd_command_t builtin_source = {
    "source",
    "source <file>",
    "Execute commands from a file in the current session",
    "system",
    cmd_source,
    1, 1
};
