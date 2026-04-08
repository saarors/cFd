#include "control.h"
#include "variable.h"
#include "../core/session.h"
#include "../utils/mem.h"
#include "../utils/str_utils.h"
#include "../../include/config.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef _WIN32
#  include <glob.h>
#else
#  include <windows.h>
#endif

/* Forward: execute an AST node via the session's executor */
extern int cfd_session_exec_node(cfd_session_t *sess, cfd_ast_node_t *node);

/* ── Glob expansion ─────────────────────────────────────────────────── */
/* Expand a single pattern into a list of matching file paths.
   If the pattern has no wildcards, returns {pattern} unchanged.
   Caller must free each string and the array itself. */
static char **expand_glob_pattern(const char *pattern, int *count) {
    *count = 0;

    int has_wildcard = (strchr(pattern, '*') || strchr(pattern, '?') ||
                        strchr(pattern, '['));
    if (!has_wildcard) {
        char **r = cfd_malloc(sizeof(char *));
        r[0] = cfd_strdup(pattern);
        *count = 1;
        return r;
    }

#ifndef _WIN32
    glob_t g;
    if (glob(pattern, GLOB_TILDE | GLOB_NOCHECK, NULL, &g) == 0) {
        char **r = cfd_malloc(g.gl_pathc * sizeof(char *));
        for (size_t i = 0; i < g.gl_pathc; i++)
            r[i] = cfd_strdup(g.gl_pathv[i]);
        *count = (int)g.gl_pathc;
        globfree(&g);
        return r;
    }
    /* fallback */
    char **r = cfd_malloc(sizeof(char *));
    r[0] = cfd_strdup(pattern);
    *count = 1;
    return r;

#else /* Windows */
    WIN32_FIND_DATAA ffd;
    HANDLE hf = FindFirstFileA(pattern, &ffd);
    if (hf == INVALID_HANDLE_VALUE) {
        char **r = cfd_malloc(sizeof(char *));
        r[0] = cfd_strdup(pattern);
        *count = 1;
        return r;
    }

    /* Determine directory prefix from pattern */
    char dir_prefix[CFD_MAX_PATH] = "";
    const char *last_sep = strrchr(pattern, '/');
    if (!last_sep) last_sep = strrchr(pattern, '\\');
    if (last_sep) {
        size_t plen = (size_t)(last_sep - pattern) + 1;
        if (plen < CFD_MAX_PATH) {
            strncpy(dir_prefix, pattern, plen);
            dir_prefix[plen] = '\0';
        }
    }

    /* Collect matches (up to 4096) */
    char **matches = cfd_malloc(4096 * sizeof(char *));
    int n = 0;
    do {
        if (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0)
            continue;
        char full[CFD_MAX_PATH];
        snprintf(full, sizeof(full), "%s%s", dir_prefix, ffd.cFileName);
        if (n < 4096)
            matches[n++] = cfd_strdup(full);
    } while (FindNextFileA(hf, &ffd) && n < 4096);
    FindClose(hf);

    if (n == 0) {
        cfd_free(matches);
        char **r = cfd_malloc(sizeof(char *));
        r[0] = cfd_strdup(pattern);
        *count = 1;
        return r;
    }

    *count = n;
    return matches;
#endif
}

/* ── if/then/elif/else/fi ───────────────────────────────────────────── */
int cfd_ctrl_eval_if(cfd_session_t *sess, cfd_ast_node_t *node,
                     cfd_ctrl_state_t *ctrl) {
    if (!node) return 0;
    int ret = 0;
    if (node->condition) {
        ret = cfd_session_exec_node(sess, node->condition);
        if (ctrl->signal != CFD_CTRL_NORMAL) return ret;
    }
    if (ret == 0 && node->body) {
        ret = cfd_session_exec_node(sess, node->body);
    } else if (ret != 0 && node->else_body) {
        ret = cfd_session_exec_node(sess, node->else_body);
    }
    return ret;
}

/* ── while/until loop ───────────────────────────────────────────────── */
int cfd_ctrl_eval_while(cfd_session_t *sess, cfd_ast_node_t *node,
                        cfd_ctrl_state_t *ctrl) {
    if (!node || !node->condition || !node->body) return 0;
    int until = node->background; /* 'until' is flagged via background field */
    int ret = 0;
    while (true) {
        ret = cfd_session_exec_node(sess, node->condition);
        if (ctrl->signal != CFD_CTRL_NORMAL) break;
        /* 'while' continues when ret==0; 'until' continues when ret!=0 */
        int should_continue = until ? (ret != 0) : (ret == 0);
        if (!should_continue) break;
        ret = cfd_session_exec_node(sess, node->body);
        if (ctrl->signal == CFD_CTRL_BREAK)    { ctrl->signal = CFD_CTRL_NORMAL; break; }
        if (ctrl->signal == CFD_CTRL_CONTINUE) { ctrl->signal = CFD_CTRL_NORMAL; continue; }
        if (ctrl->signal != CFD_CTRL_NORMAL)   break;
    }
    return ret;
}

/* ── for VAR in LIST; do BODY; done ────────────────────────────────── */
int cfd_ctrl_eval_for(cfd_session_t *sess, cfd_ast_node_t *node,
                      cfd_ctrl_state_t *ctrl) {
    if (!node || !node->var_name || !node->body) return 0;
    int ret = 0;

    /* Build final word list: variable-expand each word, then glob-expand */
    char **words = NULL;
    int    nwords = 0;

    for (int i = 0; i < node->var_list_len; i++) {
        /* Variable expansion */
        char *expanded = cfd_var_expand(sess->vars, node->var_list[i]);

        /* Glob expansion */
        int  nglob;
        char **globs = expand_glob_pattern(expanded, &nglob);
        cfd_free(expanded);

        words = realloc(words, (nwords + nglob) * sizeof(char *));
        for (int j = 0; j < nglob; j++)
            words[nwords++] = globs[j];
        cfd_free(globs);
    }

    /* Iterate */
    for (int i = 0; i < nwords; i++) {
        cfd_session_set_var(sess, node->var_name, words[i]);
        ret = cfd_session_exec_node(sess, node->body);
        if (ctrl->signal == CFD_CTRL_BREAK)    { ctrl->signal = CFD_CTRL_NORMAL; break; }
        if (ctrl->signal == CFD_CTRL_CONTINUE) { ctrl->signal = CFD_CTRL_NORMAL; continue; }
        if (ctrl->signal != CFD_CTRL_NORMAL)   break;
    }

    for (int i = 0; i < nwords; i++) cfd_free(words[i]);
    cfd_free(words);
    return ret;
}
