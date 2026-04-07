#include "readline.h"
#include "keybind.h"
#include "completion.h"
#include "../utils/mem.h"
#include "../utils/str_utils.h"
#include "../ui/display.h"
#include "../platform/platform.h"
#include "../../include/config.h"
#include "../commands/registry.h"
#include "../utils/path.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* ── ANSI color helpers ───────────────────────────────────────────────── */
#define AC_RESET    "\033[0m"
#define AC_GREEN    "\033[32m"
#define AC_YELLOW   "\033[33m"
#define AC_CYAN     "\033[36m"
#define AC_MAGENTA  "\033[35m"
#define AC_DGRAY    "\033[90m"
#define AC_DIM      "\033[2m"
#define AC_DIMGRAY  "\033[2;37m"

/* ── Forward declarations ─────────────────────────────────────────────── */
static void rl_refresh(cfd_readline_t *rl, const char *prompt);

/* ── Constructor / destructor ─────────────────────────────────────────── */

cfd_readline_t *cfd_readline_new(cfd_session_t *sess, cfd_history_t *history) {
    cfd_readline_t *rl = CFD_NEW(cfd_readline_t);
    rl->sess    = sess;
    rl->history = history;
    return rl;
}

void cfd_readline_free(cfd_readline_t *rl) {
    cfd_free(rl);
}

/* ── Check if a command exists in PATH ───────────────────────────────── */
static int cmd_in_path(const char *name) {
    if (!name || !*name) return 0;
    char *path_env = cfd_platform_getenv("PATH");
    if (!path_env) return 0;
    int n;
    char **dirs = cfd_path_split_dirs(path_env, &n);
    cfd_free(path_env);
    int found = 0;
    for (int i = 0; i < n && !found; i++) {
        char *full = cfd_path_join(dirs[i], name);
#ifdef _WIN32
        char *fullexe = cfd_sprintf("%s.exe", full);
        if (cfd_path_exists(fullexe)) found = 1;
        cfd_free(fullexe);
        if (!found) {
            char *fullcmd = cfd_sprintf("%s.cmd", full);
            if (cfd_path_exists(fullcmd)) found = 1;
            cfd_free(fullcmd);
        }
#endif
        if (!found && cfd_path_exists(full)) found = 1;
        cfd_free(full);
    }
    cfd_strfreev(dirs);
    return found;
}

/* ── Syntax-highlighted display of buf ───────────────────────────────── */
/*
 * Writes colorized version of buf[0..len) to stdout.
 * Does NOT change buf itself.
 */
static void rl_print_highlighted(cfd_readline_t *rl) {
    const char *s = rl->buf;
    int len = rl->len;
    if (len <= 0) return;

    int i = 0;
    int word_idx = 0; /* which word we're currently in (0 = command) */

    while (i < len) {
        /* skip leading whitespace between words */
        if (s[i] == ' ' || s[i] == '\t') {
            putchar(s[i]);
            i++;
            continue;
        }

        /* detect special single chars: | > < */
        if (s[i] == '|') {
            fputs(AC_CYAN "|" AC_RESET, stdout);
            i++;
            word_idx++;
            continue;
        }
        if (s[i] == '>' || s[i] == '<') {
            /* could be >> or 2> */
            fputs(AC_CYAN, stdout);
            putchar(s[i++]);
            if (i < len && s[i] == '>') putchar(s[i++]);
            fputs(AC_RESET, stdout);
            word_idx++;
            continue;
        }
        /* comment after first word */
        if (s[i] == '#' && word_idx > 0) {
            fputs(AC_DGRAY, stdout);
            while (i < len) putchar(s[i++]);
            fputs(AC_RESET, stdout);
            break;
        }
        /* string literal */
        if (s[i] == '"' || s[i] == '\'') {
            char quote = s[i];
            fputs(AC_YELLOW, stdout);
            putchar(s[i++]);
            while (i < len && s[i] != quote) {
                if (s[i] == '\\' && i+1 < len) { putchar(s[i++]); }
                putchar(s[i++]);
            }
            if (i < len) putchar(s[i++]); /* closing quote */
            fputs(AC_RESET, stdout);
            word_idx++;
            continue;
        }
        /* variable $VAR */
        if (s[i] == '$') {
            fputs(AC_MAGENTA, stdout);
            putchar(s[i++]);
            while (i < len && (isalnum((unsigned char)s[i]) || s[i] == '_'))
                putchar(s[i++]);
            fputs(AC_RESET, stdout);
            continue;
        }

        /* collect a word token */
        int wstart = i;
        while (i < len && s[i] != ' ' && s[i] != '\t' &&
               s[i] != '|' && s[i] != '>' && s[i] != '<' &&
               s[i] != '"' && s[i] != '\'' && s[i] != '$') {
            i++;
        }
        int wlen = i - wstart;
        char word[CFD_MAX_INPUT];
        if (wlen >= CFD_MAX_INPUT) wlen = CFD_MAX_INPUT - 1;
        memcpy(word, s + wstart, wlen);
        word[wlen] = '\0';

        if (word_idx == 0) {
            /* command name: green if known builtin or in PATH, else white */
            const cfd_command_t *cmd = cfd_registry_find(g_registry, word);
            int known = (cmd != NULL) || cmd_in_path(word);
            if (known)
                fputs(AC_GREEN, stdout);
            fwrite(word, 1, wlen, stdout);
            if (known)
                fputs(AC_RESET, stdout);
        } else if (word[0] == '-') {
            /* flag */
            fputs(AC_DIM, stdout);
            fwrite(word, 1, wlen, stdout);
            fputs(AC_RESET, stdout);
        } else {
            fwrite(word, 1, wlen, stdout);
        }
        word_idx++;
    }
}

/* ── Auto-suggest: find history entry starting with buf ──────────────── */
static void rl_update_suggest(cfd_readline_t *rl) {
    rl->suggest_buf[0] = '\0';
    if (!rl->history || rl->len == 0) return;
    int cnt = cfd_history_count(rl->history);
    /* search from newest to oldest */
    for (int i = cnt - 1; i >= 0; i--) {
        const char *entry = cfd_history_get(rl->history, i);
        if (!entry) continue;
        if (strncmp(entry, rl->buf, rl->len) == 0 && (int)strlen(entry) > rl->len) {
            strncpy(rl->suggest_buf, entry + rl->len, CFD_MAX_INPUT - 1);
            rl->suggest_buf[CFD_MAX_INPUT - 1] = '\0';
            return;
        }
    }
}

/* ── Main refresh ─────────────────────────────────────────────────────── */
static void rl_refresh(cfd_readline_t *rl, const char *prompt) {
    /* Go to start of line, erase to end */
    fputs("\r\033[K", stdout);

    if (rl->search_mode) {
        /* show reverse-i-search prompt */
        fprintf(stdout, "(reverse-i-search)'%s': ", rl->search_buf);
        /* show match */
        fputs(rl->buf, stdout);
        fflush(stdout);
        return;
    }

    fputs(prompt, stdout);

    /* syntax-highlighted buf */
    rl_print_highlighted(rl);

    /* auto-suggest ghost text */
    rl_update_suggest(rl);
    if (rl->suggest_buf[0]) {
        fputs(AC_DIMGRAY, stdout);
        fputs(rl->suggest_buf, stdout);
        fputs(AC_RESET, stdout);
    }

    /* reposition cursor */
    int prompt_vis_len = 0;
    {
        /* count visible chars in prompt (skip escape sequences) */
        const char *p = prompt;
        while (*p) {
            if (*p == '\033') {
                while (*p && *p != 'm') p++;
                if (*p) p++;
            } else {
                prompt_vis_len++;
                p++;
            }
        }
    }
    int chars_after_cursor = (rl->len - rl->cursor) + (int)strlen(rl->suggest_buf);
    if (chars_after_cursor > 0) {
        fprintf(stdout, "\033[%dD", chars_after_cursor);
    }

    fflush(stdout);
}

/* ── Edit operations ──────────────────────────────────────────────────── */

static void rl_insert(cfd_readline_t *rl, char c) {
    if (rl->len >= CFD_MAX_INPUT - 1) return;
    memmove(rl->buf + rl->cursor + 1, rl->buf + rl->cursor, rl->len - rl->cursor);
    rl->buf[rl->cursor++] = c;
    rl->len++;
    rl->buf[rl->len] = '\0';
}

static void rl_delete_back(cfd_readline_t *rl) {
    if (rl->cursor == 0) return;
    memmove(rl->buf + rl->cursor - 1, rl->buf + rl->cursor, rl->len - rl->cursor);
    rl->cursor--;
    rl->len--;
    rl->buf[rl->len] = '\0';
}

static void rl_delete_fwd(cfd_readline_t *rl) {
    if (rl->cursor >= rl->len) return;
    memmove(rl->buf + rl->cursor, rl->buf + rl->cursor + 1, rl->len - rl->cursor - 1);
    rl->len--;
    rl->buf[rl->len] = '\0';
}

static void rl_kill_to_end(cfd_readline_t *rl) {
    int kill_len = rl->len - rl->cursor;
    if (kill_len <= 0) return;
    strncpy(rl->kill_ring, rl->buf + rl->cursor, kill_len);
    rl->kill_ring[kill_len] = '\0';
    rl->buf[rl->cursor] = '\0';
    rl->len = rl->cursor;
}

static void rl_yank(cfd_readline_t *rl) {
    int klen = (int)strlen(rl->kill_ring);
    if (klen == 0 || rl->len + klen >= CFD_MAX_INPUT) return;
    memmove(rl->buf + rl->cursor + klen, rl->buf + rl->cursor, rl->len - rl->cursor);
    memcpy(rl->buf + rl->cursor, rl->kill_ring, klen);
    rl->cursor += klen;
    rl->len    += klen;
    rl->buf[rl->len] = '\0';
}

/* ── Tab completion ───────────────────────────────────────────────────── */
static void rl_handle_tab(cfd_readline_t *rl, const char *prompt) {
    cfd_completion_result_t cr = cfd_completion_get(rl->sess, rl->buf, rl->cursor);
    if (cr.count == 0) {
        putchar('\a'); fflush(stdout);
    } else if (cr.count == 1) {
        int start = rl->cursor;
        while (start > 0 && rl->buf[start-1] != ' ' && rl->buf[start-1] != '\t') start--;
        char *match = cr.matches[0];
        int match_len = (int)strlen(match);
        int tail_len  = rl->len - rl->cursor;
        memmove(rl->buf + start + match_len, rl->buf + rl->cursor, tail_len);
        memcpy(rl->buf + start, match, match_len);
        rl->len    = start + match_len + tail_len;
        rl->cursor = start + match_len;
        rl->buf[rl->len] = '\0';
    } else {
        printf("\n");
        for (int i = 0; i < cr.count; i++) {
            printf("  %s", cr.matches[i]);
            if ((i + 1) % 4 == 0 || i == cr.count - 1) printf("\n");
        }
        rl_refresh(rl, prompt);
    }
    cfd_completion_result_free(&cr);
}

/* ── Ctrl+R search helpers ────────────────────────────────────────────── */
static void rl_search_find(cfd_readline_t *rl) {
    /* Search history backward from search_hist_idx for search_buf */
    if (!rl->history) return;
    int cnt = cfd_history_count(rl->history);
    if (cnt == 0) return;
    int start = rl->search_hist_idx;
    if (start < 0) start = cnt - 1;
    for (int i = start; i >= 0; i--) {
        const char *entry = cfd_history_get(rl->history, i);
        if (!entry) continue;
        if (strstr(entry, rl->search_buf)) {
            strncpy(rl->buf, entry, CFD_MAX_INPUT - 1);
            rl->buf[CFD_MAX_INPUT - 1] = '\0';
            rl->len = (int)strlen(rl->buf);
            rl->cursor = rl->len;
            rl->search_hist_idx = i - 1;
            return;
        }
    }
    /* not found — keep current */
}

static void rl_search_enter(cfd_readline_t *rl) {
    rl->search_mode = 1;
    rl->search_buf[0] = '\0';
    rl->search_hist_idx = rl->history ? cfd_history_count(rl->history) - 1 : -1;
    /* save original */
    strncpy(rl->search_orig_buf, rl->buf, CFD_MAX_INPUT - 1);
    rl->search_orig_len    = rl->len;
    rl->search_orig_cursor = rl->cursor;
}

static void rl_search_cancel(cfd_readline_t *rl) {
    rl->search_mode = 0;
    strncpy(rl->buf, rl->search_orig_buf, CFD_MAX_INPUT - 1);
    rl->len    = rl->search_orig_len;
    rl->cursor = rl->search_orig_cursor;
    rl->buf[rl->len] = '\0';
}

static void rl_search_accept(cfd_readline_t *rl) {
    rl->search_mode = 0;
    /* buf already has the match; leave it */
}

/* ── Main readline loop ───────────────────────────────────────────────── */
char *cfd_readline_read(cfd_readline_t *rl, const char *prompt) {
    rl->len = rl->cursor = 0;
    rl->buf[0] = '\0';
    rl->search_mode = 0;
    rl->search_buf[0] = '\0';
    rl->suggest_buf[0] = '\0';
    if (rl->history) cfd_history_reset_pos(rl->history);

    if (prompt) { fputs(prompt, stdout); fflush(stdout); }

    cfd_platform_raw_mode_enter();

    while (1) {
        int key = cfd_read_key();
        if (key < 0) { /* EOF */
            cfd_platform_raw_mode_exit();
            if (rl->len == 0) return NULL;
            break;
        }

        /* ── search mode ── */
        if (rl->search_mode) {
            if (key == KEY_CTRL('G') || key == KEY_ESC) {
                rl_search_cancel(rl);
            } else if (key == KEY_CTRL('R')) {
                /* search further back */
                if (rl->search_hist_idx >= 0)
                    rl_search_find(rl);
            } else if (key == KEY_ENTER || key == '\n') {
                rl_search_accept(rl);
                rl_refresh(rl, prompt);
                break;
            } else if (key == KEY_BACKSPACE || key == 127 || key == '\b') {
                int slen = (int)strlen(rl->search_buf);
                if (slen > 0) {
                    rl->search_buf[slen - 1] = '\0';
                    /* reset search position */
                    rl->search_hist_idx = rl->history ? cfd_history_count(rl->history) - 1 : -1;
                    rl_search_find(rl);
                }
            } else if (key >= 0x20 && key < 0x100) {
                int slen = (int)strlen(rl->search_buf);
                if (slen < CFD_MAX_INPUT - 1) {
                    rl->search_buf[slen] = (char)key;
                    rl->search_buf[slen + 1] = '\0';
                    rl_search_find(rl);
                }
            }
            rl_refresh(rl, prompt);
            continue;
        }

        /* ── normal mode ── */
        if (key == KEY_ENTER || key == '\n') {
            break;
        } else if (key == KEY_BACKSPACE || key == 127 || key == '\b') {
            rl_delete_back(rl);
        } else if (key == KEY_DELETE) {
            rl_delete_fwd(rl);
        } else if (key == KEY_LEFT) {
            if (rl->cursor > 0) rl->cursor--;
        } else if (key == KEY_RIGHT) {
            /* accept auto-suggest if at end of buf */
            if (rl->cursor == rl->len && rl->suggest_buf[0]) {
                int slen = (int)strlen(rl->suggest_buf);
                if (rl->len + slen < CFD_MAX_INPUT - 1) {
                    memcpy(rl->buf + rl->len, rl->suggest_buf, slen);
                    rl->len += slen;
                    rl->cursor = rl->len;
                    rl->buf[rl->len] = '\0';
                    rl->suggest_buf[0] = '\0';
                }
            } else if (rl->cursor < rl->len) {
                rl->cursor++;
            }
        } else if (key == KEY_HOME || key == KEY_CTRL('A')) {
            rl->cursor = 0;
        } else if (key == KEY_END || key == KEY_CTRL('E')) {
            /* accept suggestion */
            if (rl->suggest_buf[0]) {
                int slen = (int)strlen(rl->suggest_buf);
                if (rl->len + slen < CFD_MAX_INPUT - 1) {
                    memcpy(rl->buf + rl->len, rl->suggest_buf, slen);
                    rl->len += slen;
                    rl->buf[rl->len] = '\0';
                    rl->suggest_buf[0] = '\0';
                }
            }
            rl->cursor = rl->len;
        } else if (key == KEY_UP && rl->history) {
            const char *prev = cfd_history_prev(rl->history);
            if (prev) {
                strncpy(rl->buf, prev, CFD_MAX_INPUT - 1);
                rl->len = rl->cursor = (int)strlen(rl->buf);
            }
        } else if (key == KEY_DOWN && rl->history) {
            const char *next = cfd_history_next(rl->history);
            if (next) {
                strncpy(rl->buf, next, CFD_MAX_INPUT - 1);
                rl->len = rl->cursor = (int)strlen(rl->buf);
            } else {
                rl->buf[0] = '\0';
                rl->len = rl->cursor = 0;
            }
        } else if (key == KEY_CTRL('K')) {
            rl_kill_to_end(rl);
        } else if (key == KEY_CTRL('Y')) {
            rl_yank(rl);
        } else if (key == KEY_CTRL('U')) {
            memmove(rl->buf, rl->buf + rl->cursor, rl->len - rl->cursor);
            rl->len    -= rl->cursor;
            rl->cursor  = 0;
            rl->buf[rl->len] = '\0';
        } else if (key == KEY_CTRL('W')) {
            int end = rl->cursor;
            while (rl->cursor > 0 && rl->buf[rl->cursor-1] == ' ') rl->cursor--;
            while (rl->cursor > 0 && rl->buf[rl->cursor-1] != ' ') rl->cursor--;
            int del = end - rl->cursor;
            memmove(rl->buf + rl->cursor, rl->buf + end, rl->len - end);
            rl->len -= del;
            rl->buf[rl->len] = '\0';
        } else if (key == KEY_CTRL('C')) {
            printf("^C\n");
            fflush(stdout);
            rl->buf[0] = '\0'; rl->len = rl->cursor = 0;
            cfd_platform_raw_mode_exit();
            return cfd_strdup("");
        } else if (key == KEY_CTRL('D')) {
            if (rl->len == 0) {
                cfd_platform_raw_mode_exit();
                return NULL;
            }
            rl_delete_fwd(rl);
        } else if (key == KEY_CTRL('L')) {
            display_erase_screen();
            if (prompt) { fputs(prompt, stdout); fflush(stdout); }
        } else if (key == KEY_CTRL('R')) {
            rl_search_enter(rl);
        } else if (key == KEY_TAB) {
            rl_handle_tab(rl, prompt);
        } else if (key >= 0x20 && key < 0x100) {
            rl_insert(rl, (char)key);
        }

        rl_refresh(rl, prompt);
    }

    cfd_platform_raw_mode_exit();
    printf("\n");
    fflush(stdout);
    return cfd_strdup(rl->buf);
}
