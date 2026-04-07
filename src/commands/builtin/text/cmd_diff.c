#include "cmd_diff.h"
#include "../../../utils/mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* ---- Simple LCS-based diff ---- */

static char **read_file_lines(const char *filename, int *count) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "diff: %s: No such file or directory\n", filename);
        *count = -1;
        return NULL;
    }

    int    cap = 64;
    char **lines = (char **)cfd_malloc((size_t)cap * sizeof(char *));
    if (!lines) { fclose(fp); *count = 0; return NULL; }

    int    n = 0;
    char   buf[4096];
    while (fgets(buf, sizeof(buf), fp)) {
        if (n >= cap) {
            cap *= 2;
            char **tmp = (char **)cfd_realloc(lines, (size_t)cap * sizeof(char *));
            if (!tmp) break;
            lines = tmp;
        }
        /* Keep newline for output, but strip for comparison */
        size_t len = strlen(buf);
        if (len > 0 && buf[len-1] == '\n') buf[--len] = '\0';
        lines[n++] = cfd_strdup(buf);
    }
    fclose(fp);
    *count = n;
    return lines;
}

static void free_lines(char **lines, int n) {
    for (int i = 0; i < n; i++) cfd_free(lines[i]);
    cfd_free(lines);
}

/* LCS DP table */
static int *lcs_build(char **a, int m, char **b, int n) {
    /* (m+1) x (n+1) table, row-major */
    int *dp = (int *)cfd_calloc((size_t)((m + 1) * (n + 1)), sizeof(int));
    if (!dp) return NULL;

    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            if (strcmp(a[i-1], b[j-1]) == 0)
                dp[i*(n+1)+j] = dp[(i-1)*(n+1)+(j-1)] + 1;
            else
                dp[i*(n+1)+j] = (dp[(i-1)*(n+1)+j] > dp[i*(n+1)+(j-1)])
                                  ? dp[(i-1)*(n+1)+j]
                                  : dp[i*(n+1)+(j-1)];
        }
    }
    return dp;
}

/* Diff output types */
typedef enum { DIFF_EQ, DIFF_ADD, DIFF_DEL } diff_op_t;

typedef struct { diff_op_t op; int a_line; int b_line; const char *text; } diff_hunk_t;

#define MAX_HUNKS 65536

static int hunk_count = 0;
static diff_hunk_t hunks[MAX_HUNKS];

static void collect_diff(int *dp, int n, char **a, int i, char **b, int j) {
    if (i > 0 && j > 0 && strcmp(a[i-1], b[j-1]) == 0) {
        collect_diff(dp, n, a, i-1, b, j-1);
        if (hunk_count < MAX_HUNKS)
            hunks[hunk_count++] = (diff_hunk_t){DIFF_EQ, i, j, a[i-1]};
    } else if (j > 0 && (i == 0 || dp[i*(n+1)+j-1] >= dp[(i-1)*(n+1)+j])) {
        collect_diff(dp, n, a, i, b, j-1);
        if (hunk_count < MAX_HUNKS)
            hunks[hunk_count++] = (diff_hunk_t){DIFF_ADD, i, j, b[j-1]};
    } else if (i > 0) {
        collect_diff(dp, n, a, i-1, b, j);
        if (hunk_count < MAX_HUNKS)
            hunks[hunk_count++] = (diff_hunk_t){DIFF_DEL, i, j, a[i-1]};
    }
}

int cmd_diff(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    bool unified = false;
    const char *file1 = NULL, *file2 = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-u") == 0) unified = true;
        else if (!file1)               file1 = argv[i];
        else if (!file2)               file2 = argv[i];
    }

    if (!file1 || !file2) {
        fprintf(stderr, "Usage: diff [-u] <file1> <file2>\n");
        return 1;
    }

    int m, n;
    char **a = read_file_lines(file1, &m);
    if (m < 0) return 1;
    char **b = read_file_lines(file2, &n);
    if (n < 0) { free_lines(a, m); return 1; }

    int *dp = lcs_build(a, m, b, n);
    if (!dp) { free_lines(a, m); free_lines(b, n); return 1; }

    hunk_count = 0;
    collect_diff(dp, n, a, m, b, n);
    cfd_free(dp);

    /* Print diff */
    if (unified) {
        printf("\033[34m--- %s\033[0m\n", file1);
        printf("\033[34m+++ %s\033[0m\n", file2);

        /* Find ranges of changes for unified headers */
        int context = 3;
        int i = 0;
        while (i < hunk_count) {
            if (hunks[i].op == DIFF_EQ) { i++; continue; }
            /* Found change; find extent */
            int start = i - context;
            if (start < 0) start = 0;
            int end = i + 1;
            while (end < hunk_count && (hunks[end].op != DIFF_EQ ||
                   (end - i) < context)) end++;
            if (end < hunk_count) end += context;
            if (end >= hunk_count) end = hunk_count;

            /* Count lines in each file for @@ header */
            int a_start = hunks[start].a_line, a_count = 0;
            int b_start = hunks[start].b_line, b_count = 0;
            for (int k = start; k < end; k++) {
                if (hunks[k].op != DIFF_ADD) a_count++;
                if (hunks[k].op != DIFF_DEL) b_count++;
            }
            printf("\033[36m@@ -%d,%d +%d,%d @@\033[0m\n",
                   a_start, a_count, b_start, b_count);

            for (int k = start; k < end; k++) {
                switch (hunks[k].op) {
                    case DIFF_EQ:  printf(" %s\n",        hunks[k].text); break;
                    case DIFF_DEL: printf("\033[31m-%s\033[0m\n", hunks[k].text); break;
                    case DIFF_ADD: printf("\033[32m+%s\033[0m\n", hunks[k].text); break;
                }
            }
            i = end;
        }
    } else {
        /* Normal diff output */
        for (int i = 0; i < hunk_count; i++) {
            switch (hunks[i].op) {
                case DIFF_EQ:  break;
                case DIFF_DEL: printf("\033[31m< %s\033[0m\n", hunks[i].text); break;
                case DIFF_ADD: printf("\033[32m> %s\033[0m\n", hunks[i].text); break;
            }
        }
    }

    int changed = 0;
    for (int i = 0; i < hunk_count; i++)
        if (hunks[i].op != DIFF_EQ) changed++;

    free_lines(a, m);
    free_lines(b, n);

    return changed > 0 ? 1 : 0;
}

const cfd_command_t builtin_diff = {
    "diff",
    "diff [-u] <file1> <file2>",
    "Compare files line by line",
    "text",
    cmd_diff,
    2, -1
};
