#include "repl.h"
#include "config.h"
#include "../ui/prompt.h"
#include "../ui/display.h"
#include "../utils/mem.h"
#include "../utils/str_utils.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

/* ── Multiline continuation detection ────────────────────────────────── */

/*
 * Returns non-zero if we need more input to complete the command.
 * Checks:
 *  - line ends with '\'
 *  - line ends with '|', '&&', '||'
 *  - unmatched '{' '(' '['
 */
/* Count occurrences of a whole word in a shell line (skips quoted regions) */
static int count_whole_word(const char *line, const char *word) {
    int count  = 0;
    int in_sq  = 0, in_dq = 0;
    size_t wlen = strlen(word);
    for (const char *p = line; *p; p++) {
        if (in_sq)  { if (*p == '\'') in_sq = 0; continue; }
        if (in_dq)  { if (*p == '\\' && p[1]) { p++; continue; }
                      if (*p == '"') in_dq = 0; else continue; }
        if (*p == '\'') { in_sq = 1; continue; }
        if (*p == '"')  { in_dq = 1; continue; }
        if (*p == '#')  break;
        if (strncmp(p, word, wlen) == 0) {
            int before_ok = (p == line || (!isalnum((unsigned char)p[-1]) && p[-1] != '_'));
            int after_ok  = (!isalnum((unsigned char)p[wlen]) && p[wlen] != '_');
            if (before_ok && after_ok) { count++; p += wlen - 1; }
        }
    }
    return count;
}

/* Returns true if the line ends with the given whole word */
static int ends_with_word(const char *line, const char *word) {
    size_t llen = strlen(line), wlen = strlen(word);
    if (llen < wlen) return 0;
    const char *tail = line + llen - wlen;
    /* skip trailing whitespace */
    const char *end = line + llen - 1;
    while (end >= line && (*end == ' ' || *end == '\t' || *end == '\r')) end--;
    if ((size_t)(end - line + 1) < wlen) return 0;
    tail = end - wlen + 1;
    if (strncmp(tail, word, wlen) != 0) return 0;
    int before_ok = (tail == line || (!isalnum((unsigned char)tail[-1]) && tail[-1] != '_'));
    return before_ok;
}

static int needs_more_input(const char *line) {
    if (!line || !*line) return 0;

    size_t len = strlen(line);
    /* strip trailing whitespace for end-checks */
    int end = (int)len - 1;
    while (end >= 0 && (line[end] == ' ' || line[end] == '\t' || line[end] == '\r'))
        end--;

    if (end < 0) return 0;

    /* ends with backslash continuation */
    if (line[end] == '\\') return 1;

    /* ends with pipe */
    if (line[end] == '|') return 1;

    /* ends with && or || */
    if (end >= 1 && line[end] == '&' && line[end-1] == '&') return 1;
    if (end >= 1 && line[end] == '|' && line[end-1] == '|') return 1;

    /* count unmatched brackets (skip quoted strings) */
    int braces = 0, parens = 0, brackets = 0;
    int in_single = 0, in_double = 0;
    for (int i = 0; i <= end; i++) {
        char c = line[i];
        if (in_single) {
            if (c == '\'') in_single = 0;
            continue;
        }
        if (in_double) {
            if (c == '\\' && i + 1 <= end) { i++; continue; }
            if (c == '"') in_double = 0;
            continue;
        }
        if (c == '\'') { in_single = 1; continue; }
        if (c == '"')  { in_double = 1; continue; }
        if (c == '#')  break;
        if (c == '{')  braces++;
        else if (c == '}') braces--;
        else if (c == '(') parens++;
        else if (c == ')') parens--;
        else if (c == '[') brackets++;
        else if (c == ']') brackets--;
    }
    if (braces > 0 || parens > 0 || brackets > 0) return 1;

    /* Unmatched keyword blocks: for/while/if vs done/fi */
    int opens  = count_whole_word(line, "for")
               + count_whole_word(line, "while")
               + count_whole_word(line, "until")
               + count_whole_word(line, "if");
    int closes = count_whole_word(line, "done")
               + count_whole_word(line, "fi");
    if (opens > closes) return 1;

    /* Ends with a keyword that expects a body to follow */
    if (ends_with_word(line, "do"))   return 1;
    if (ends_with_word(line, "then")) return 1;
    if (ends_with_word(line, "else")) return 1;
    if (ends_with_word(line, "elif")) return 1;

    return 0;
}

/* ── REPL lifecycle ───────────────────────────────────────────────────── */

cfd_repl_t *cfd_repl_new(cfd_session_t *sess) {
    cfd_repl_t *r = CFD_NEW(cfd_repl_t);
    r->sess  = sess;
    r->input = cfd_input_new(sess);
    return r;
}

void cfd_repl_free(cfd_repl_t *repl) {
    if (!repl) return;
    cfd_input_free(repl->input);
    cfd_free(repl);
}

int cfd_repl_eval(cfd_repl_t *repl, const char *line) {
    if (!line) return 0;
    const char *trimmed = line;
    while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
    if (!*trimmed || *trimmed == '#') return 0;
    return cfd_session_exec_string(repl->sess, trimmed);
}

int cfd_repl_run(cfd_repl_t *repl) {
    while (repl->sess->running) {
        /* Build prompt */
        char *prompt = cfd_prompt_build(repl->sess);

        char *line = cfd_input_readline(repl->input, prompt);
        cfd_free(prompt);

        if (!line) {
            /* EOF / Ctrl-D */
            printf("\n");
            break;
        }

        /* ── multiline continuation ── */
        char *full_line = cfd_strdup(line);
        cfd_free(line);

        while (needs_more_input(full_line)) {
            /* strip trailing backslash if continuation */
            int flen = (int)strlen(full_line);
            if (flen > 0 && full_line[flen-1] == '\\') {
                full_line[flen-1] = ' ';
                full_line[flen]   = '\0';
            } else {
                /* append a space to separate from next line */
                char *tmp = cfd_sprintf("%s ", full_line);
                cfd_free(full_line);
                full_line = tmp;
            }

            char *more = cfd_input_readline(repl->input, "> ");
            if (!more) break;

            char *combined = cfd_sprintf("%s%s", full_line, more);
            cfd_free(full_line);
            cfd_free(more);
            full_line = combined;
        }

        /* ── execution with timer ── */
        clock_t t_start = clock();
        int ret = cfd_repl_eval(repl, full_line);
        clock_t t_end   = clock();
        cfd_free(full_line);

        double elapsed = (double)(t_end - t_start) / CLOCKS_PER_SEC;
        if (elapsed >= 5.0) {
            /* show elapsed time in dim style */
            fprintf(stdout, "\033[2m[%.1fs]\033[0m\n", elapsed);
            fflush(stdout);
        }

        (void)ret;
    }
    return repl->sess->exit_code;
}
