#include "cmd_xargs.h"
#include "../../../utils/mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/* Read tokens from stdin (whitespace-delimited or by delimiter) */
static char **xargs_read_tokens(char delim, int *count) {
    int    cap    = 64;
    char **tokens = (char **)cfd_malloc((size_t)cap * sizeof(char *));
    if (!tokens) { *count = 0; return NULL; }

    int    n   = 0;
    char   buf[4096];
    int    bpos = 0;

    int c;
    while ((c = fgetc(stdin)) != EOF) {
        bool is_sep;
        if (delim == '\0')
            is_sep = isspace(c);
        else
            is_sep = (c == delim);

        if (is_sep) {
            if (bpos > 0) {
                buf[bpos] = '\0';
                if (n >= cap) {
                    cap *= 2;
                    char **tmp = (char **)cfd_realloc(tokens, (size_t)cap * sizeof(char *));
                    if (!tmp) break;
                    tokens = tmp;
                }
                tokens[n++] = cfd_strdup(buf);
                bpos = 0;
            }
        } else {
            if (bpos < (int)sizeof(buf) - 1)
                buf[bpos++] = (char)c;
        }
    }
    /* Last token */
    if (bpos > 0) {
        buf[bpos] = '\0';
        if (n < cap) tokens[n++] = cfd_strdup(buf);
    }

    *count = n;
    return tokens;
}

int cmd_xargs(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    int  max_args = 0; /* 0 = all */
    char delim    = '\0'; /* '\0' = whitespace */
    int  cmd_start = 1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            max_args = atoi(argv[++i]);
            cmd_start = i + 1;
        } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            delim = argv[++i][0];
            cmd_start = i + 1;
        } else {
            cmd_start = i;
            break;
        }
    }

    /* Base command */
    const char *base_cmd = (cmd_start < argc) ? argv[cmd_start] : "echo";
    int base_extra = (cmd_start < argc) ? (argc - cmd_start - 1) : 0;

    /* Read tokens from stdin */
    int    ntok = 0;
    char **tokens = xargs_read_tokens(delim, &ntok);

    if (ntok == 0) {
        cfd_free(tokens);
        return 0;
    }

    /* Run command with batches */
    int batch = (max_args > 0) ? max_args : ntok;
    int ret   = 0;

    for (int i = 0; i < ntok; i += batch) {
        int this_batch = ntok - i;
        if (this_batch > batch) this_batch = batch;

        /* Build command string */
        char cmd[8192];
        int clen = (int)strlen(base_cmd);
        if (clen >= (int)sizeof(cmd)) clen = (int)sizeof(cmd) - 1;
        strncpy(cmd, base_cmd, (size_t)clen);
        cmd[clen] = '\0';

        /* Add extra args from original command line */
        for (int k = 1; k <= base_extra; k++) {
            strncat(cmd, " ", sizeof(cmd) - strlen(cmd) - 1);
            strncat(cmd, argv[cmd_start + k], sizeof(cmd) - strlen(cmd) - 1);
        }

        /* Add stdin tokens */
        for (int j = 0; j < this_batch; j++) {
            strncat(cmd, " ", sizeof(cmd) - strlen(cmd) - 1);
            /* Quote tokens with spaces */
            bool needs_quote = strchr(tokens[i + j], ' ') != NULL;
            if (needs_quote) strncat(cmd, "\"", sizeof(cmd) - strlen(cmd) - 1);
            strncat(cmd, tokens[i + j], sizeof(cmd) - strlen(cmd) - 1);
            if (needs_quote) strncat(cmd, "\"", sizeof(cmd) - strlen(cmd) - 1);
        }

        int r = system(cmd);
        if (r != 0) ret = r;
    }

    for (int i = 0; i < ntok; i++) cfd_free(tokens[i]);
    cfd_free(tokens);

    return (ret == 0) ? 0 : 1;
}

const cfd_command_t builtin_xargs = {
    "xargs",
    "xargs [-n max_args] [-d delim] <command> [args...]",
    "Build and execute commands from stdin",
    "text",
    cmd_xargs,
    0, -1
};
