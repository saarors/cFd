#include "completion.h"
#include "../utils/mem.h"
#include "../utils/str_utils.h"
#include "../utils/path.h"
#include "../core/session.h"
#include "../commands/registry.h"
#include "../../include/config.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <dirent.h>
#endif

static char  *g_extra_words[CFD_MAX_COMPLETION];
static int    g_extra_count = 0;

void cfd_completion_init(cfd_session_t *sess) {
    (void)sess;
    g_extra_count = 0;
}

void cfd_completion_add_word(const char *word) {
    if (g_extra_count < CFD_MAX_COMPLETION)
        g_extra_words[g_extra_count++] = cfd_strdup(word);
}

char **cfd_complete_filenames(const char *prefix, int *count) {
    *count = 0;
    char **results = cfd_malloc(CFD_MAX_COMPLETION * sizeof(char *));
    int n = 0;

    char *dir_part  = cfd_path_dirname(prefix);
    char *base_part = cfd_path_basename(prefix);
    const char *dir = (strcmp(dir_part, ".") == 0 && prefix[0] != '.') ? "." : dir_part;

#ifdef _WIN32
    char pattern[CFD_MAX_PATH];
    snprintf(pattern, sizeof(pattern), "%s\\*", dir);
    WIN32_FIND_DATAA fd;
    HANDLE hf = FindFirstFileA(pattern, &fd);
    if (hf != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) continue;
            if (strncmp(fd.cFileName, base_part, strlen(base_part)) == 0) {
                char full[CFD_MAX_PATH];
                if (strcmp(dir, ".") == 0) snprintf(full, sizeof(full), "%s", fd.cFileName);
                else snprintf(full, sizeof(full), "%s/%s", dir, fd.cFileName);
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    char tmp[CFD_MAX_PATH];
                    snprintf(tmp, sizeof(tmp), "%s/", full);
                    results[n++] = cfd_strdup(tmp);
                } else {
                    results[n++] = cfd_strdup(full);
                }
                if (n >= CFD_MAX_COMPLETION) break;
            }
        } while (FindNextFileA(hf, &fd));
        FindClose(hf);
    }
#else
    DIR *d = opendir(dir);
    if (d) {
        struct dirent *ent;
        while ((ent = readdir(d)) && n < CFD_MAX_COMPLETION) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
            if (strncmp(ent->d_name, base_part, strlen(base_part)) == 0) {
                char full[CFD_MAX_PATH];
                if (strcmp(dir, ".") == 0) snprintf(full, sizeof(full), "%s", ent->d_name);
                else snprintf(full, sizeof(full), "%s/%s", dir, ent->d_name);
                results[n++] = cfd_strdup(full);
            }
        }
        closedir(d);
    }
#endif

    cfd_free(dir_part);
    cfd_free(base_part);
    results[n] = NULL;
    *count = n;
    return results;
}

char **cfd_complete_commands(cfd_session_t *sess, const char *prefix, int *count) {
    (void)sess;
    *count = 0;
    char **results = cfd_malloc(CFD_MAX_COMPLETION * sizeof(char *));
    int n = 0;

    /* Get registered commands */
    int cmd_count = 0;
    const char **cmds = cfd_registry_list_names(&cmd_count);
    if (cmds) {
        for (int i = 0; i < cmd_count && n < CFD_MAX_COMPLETION; i++) {
            if (strncmp(cmds[i], prefix, strlen(prefix)) == 0)
                results[n++] = cfd_strdup(cmds[i]);
        }
        cfd_free((void*)cmds);
    }

    /* Extra words */
    for (int i = 0; i < g_extra_count && n < CFD_MAX_COMPLETION; i++) {
        if (strncmp(g_extra_words[i], prefix, strlen(prefix)) == 0)
            results[n++] = cfd_strdup(g_extra_words[i]);
    }

    results[n] = NULL;
    *count = n;
    return results;
}

cfd_completion_result_t cfd_completion_get(cfd_session_t *sess,
                                            const char *line, int cursor) {
    cfd_completion_result_t res = {0};
    if (!line || cursor < 0) return res;

    /* Find the word being completed */
    int start = cursor;
    while (start > 0 && line[start-1] != ' ' && line[start-1] != '\t') start--;
    char word[CFD_MAX_INPUT];
    int wlen = cursor - start;
    if (wlen < 0) wlen = 0;
    strncpy(word, line + start, wlen);
    word[wlen] = '\0';

    int n = 0;
    char **matches;

    /* If first word on line, complete commands; else filenames */
    bool is_first = true;
    for (int i = 0; i < start; i++) {
        if (line[i] != ' ' && line[i] != '\t') { is_first = false; break; }
    }

    if (is_first)
        matches = cfd_complete_commands(sess, word, &n);
    else
        matches = cfd_complete_filenames(word, &n);

    res.matches = matches;
    res.count   = n;

    /* Compute common prefix length */
    if (n > 0) {
        int cpl = (int)strlen(matches[0]);
        for (int i = 1; i < n; i++) {
            int j = 0;
            while (j < cpl && matches[0][j] == matches[i][j]) j++;
            cpl = j;
        }
        res.common_prefix_len = cpl;
    }
    return res;
}

void cfd_completion_result_free(cfd_completion_result_t *r) {
    if (!r || !r->matches) return;
    for (int i = 0; i < r->count; i++) cfd_free(r->matches[i]);
    cfd_free(r->matches);
    r->matches = NULL;
    r->count   = 0;
}
