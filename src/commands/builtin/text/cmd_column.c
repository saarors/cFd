#include "cmd_column.h"
#include "../../../utils/mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define COL_MAX_COLS 256
#define COL_MAX_ROWS 65536

static char **col_split(const char *line, char sep, int *count) {
    /* Split line by separator (whitespace if sep=0) */
    int    cap   = 16;
    char **parts = (char **)cfd_malloc((size_t)cap * sizeof(char *));
    if (!parts) { *count = 0; return NULL; }

    int    n    = 0;
    char   buf[4096];
    int    bpos = 0;
    bool   in_word = false;

    for (const char *p = line; *p; p++) {
        bool is_sep;
        if (sep == '\0')
            is_sep = isspace((unsigned char)*p);
        else
            is_sep = (*p == sep);

        if (is_sep) {
            if (sep != '\0' || in_word) {
                buf[bpos] = '\0';
                if (n >= cap) {
                    cap *= 2;
                    char **tmp = (char **)cfd_realloc(parts, (size_t)cap * sizeof(char *));
                    if (!tmp) break;
                    parts = tmp;
                }
                parts[n++] = cfd_strdup(buf);
                bpos = 0;
                in_word = false;
            }
        } else {
            if (bpos < (int)sizeof(buf) - 1) buf[bpos++] = *p;
            in_word = true;
        }
    }
    if (bpos > 0 || (sep != '\0')) {
        buf[bpos] = '\0';
        if (n < cap) parts[n++] = cfd_strdup(buf);
    }

    *count = n;
    return parts;
}

int cmd_column(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    bool table  = false;
    char sep    = '\0';
    const char *infile = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0) {
            table = true;
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            sep = argv[++i][0];
        } else if (argv[i][0] != '-') {
            infile = argv[i];
        }
    }

    FILE *fp = infile ? fopen(infile, "r") : stdin;
    if (!fp) {
        fprintf(stderr, "column: %s: No such file or directory\n", infile);
        return 1;
    }

    if (!table) {
        /* Without -t, just pass through */
        char buf[4096];
        while (fgets(buf, sizeof(buf), fp))
            fputs(buf, stdout);
        if (infile) fclose(fp);
        return 0;
    }

    /* Read all rows */
    char  **raw_lines = (char **)cfd_malloc(COL_MAX_ROWS * sizeof(char *));
    char ***rows      = (char ***)cfd_malloc(COL_MAX_ROWS * sizeof(char **));
    int    *row_cols  = (int *)cfd_calloc(COL_MAX_ROWS, sizeof(int));
    int     nrows     = 0;
    int     max_cols  = 0;

    if (!raw_lines || !rows || !row_cols) {
        if (infile) fclose(fp);
        cfd_free(raw_lines); cfd_free(rows); cfd_free(row_cols);
        return 1;
    }

    char buf[4096];
    while (fgets(buf, sizeof(buf), fp) && nrows < COL_MAX_ROWS) {
        size_t len = strlen(buf);
        while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) buf[--len] = '\0';
        raw_lines[nrows] = cfd_strdup(buf);
        int nc = 0;
        rows[nrows] = col_split(buf, sep, &nc);
        row_cols[nrows] = nc;
        if (nc > max_cols) max_cols = nc;
        nrows++;
    }
    if (infile) fclose(fp);

    if (max_cols == 0 || nrows == 0) {
        for (int r = 0; r < nrows; r++) {
            cfd_free(raw_lines[r]);
            if (rows[r]) {
                for (int c = 0; c < row_cols[r]; c++) cfd_free(rows[r][c]);
                cfd_free(rows[r]);
            }
        }
        cfd_free(raw_lines); cfd_free(rows); cfd_free(row_cols);
        return 0;
    }

    /* Compute column widths */
    int *widths = (int *)cfd_calloc((size_t)max_cols, sizeof(int));
    if (!widths) {
        for (int r = 0; r < nrows; r++) {
            cfd_free(raw_lines[r]);
            if (rows[r]) { for (int c = 0; c < row_cols[r]; c++) cfd_free(rows[r][c]); cfd_free(rows[r]); }
        }
        cfd_free(raw_lines); cfd_free(rows); cfd_free(row_cols);
        return 1;
    }

    for (int r = 0; r < nrows; r++) {
        for (int c = 0; c < row_cols[r]; c++) {
            int w = (int)strlen(rows[r][c]);
            if (w > widths[c]) widths[c] = w;
        }
    }

    /* Print aligned */
    for (int r = 0; r < nrows; r++) {
        for (int c = 0; c < row_cols[r]; c++) {
            if (c > 0) printf("  ");
            if (c < row_cols[r] - 1)
                printf("%-*s", widths[c], rows[r][c]);
            else
                printf("%s", rows[r][c]);
        }
        putchar('\n');

        for (int c = 0; c < row_cols[r]; c++) cfd_free(rows[r][c]);
        cfd_free(rows[r]);
        cfd_free(raw_lines[r]);
    }

    cfd_free(widths);
    cfd_free(raw_lines);
    cfd_free(rows);
    cfd_free(row_cols);
    return 0;
}

const cfd_command_t builtin_column = {
    "column",
    "column [-t] [-s sep] [file]",
    "Format input into columns",
    "text",
    cmd_column,
    0, -1
};
