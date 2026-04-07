#include "cmd_uniq.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int cmd_uniq(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    bool count_flag    = false; /* -c: prefix with count */
    bool dup_only      = false; /* -d: only print duplicates */
    bool unique_only   = false; /* -u: only print unique lines */
    const char *infile = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0)      count_flag = true;
        else if (strcmp(argv[i], "-d") == 0) dup_only   = true;
        else if (strcmp(argv[i], "-u") == 0) unique_only = true;
        else if (argv[i][0] != '-')          infile      = argv[i];
    }

    FILE *fp = infile ? fopen(infile, "r") : stdin;
    if (!fp) {
        fprintf(stderr, "uniq: %s: No such file or directory\n", infile);
        return 1;
    }

    char prev[4096] = "";
    char curr[4096];
    int  run = 0;
    bool have_prev = false;

    while (fgets(curr, sizeof(curr), fp)) {
        /* Normalize: remove trailing newline for comparison */
        size_t len = strlen(curr);
        if (len > 0 && curr[len-1] == '\n') curr[--len] = '\0';

        if (!have_prev) {
            strncpy(prev, curr, sizeof(prev) - 1);
            run      = 1;
            have_prev = true;
            continue;
        }

        if (strcmp(curr, prev) == 0) {
            run++;
        } else {
            /* Flush prev */
            bool should_print = false;
            if (dup_only)        should_print = (run > 1);
            else if (unique_only) should_print = (run == 1);
            else                 should_print = true;

            if (should_print) {
                if (count_flag) printf("%7d %s\n", run, prev);
                else            printf("%s\n", prev);
            }

            strncpy(prev, curr, sizeof(prev) - 1);
            run = 1;
        }
    }

    /* Flush last */
    if (have_prev) {
        bool should_print = false;
        if (dup_only)         should_print = (run > 1);
        else if (unique_only) should_print = (run == 1);
        else                  should_print = true;

        if (should_print) {
            if (count_flag) printf("%7d %s\n", run, prev);
            else            printf("%s\n", prev);
        }
    }

    if (infile) fclose(fp);
    return 0;
}

const cfd_command_t builtin_uniq = {
    "uniq",
    "uniq [-c] [-d] [-u] [file]",
    "Filter adjacent duplicate lines",
    "text",
    cmd_uniq,
    0, -1
};
