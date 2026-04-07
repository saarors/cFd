#include "readline.h"
#include "keybind.h"
#include "completion.h"
#include "../utils/mem.h"
#include "../utils/str_utils.h"
#include "../ui/display.h"
#include "../platform/platform.h"
#include "../../include/config.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

cfd_readline_t *cfd_readline_new(cfd_session_t *sess, cfd_history_t *history) {
    cfd_readline_t *rl = CFD_NEW(cfd_readline_t);
    rl->sess    = sess;
    rl->history = history;
    return rl;
}

void cfd_readline_free(cfd_readline_t *rl) {
    cfd_free(rl);
}

static void rl_refresh(cfd_readline_t *rl, const char *prompt) {
    display_erase_line();
    printf("%s%.*s", prompt, rl->len, rl->buf);
    fflush(stdout);
    /* reposition cursor */
    int prompt_len = (int)strlen(prompt);
    int total = prompt_len + rl->cursor;
    int actual = prompt_len + rl->len;
    if (actual > total) display_cursor_left(actual - total);
}

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

static void rl_handle_tab(cfd_readline_t *rl, const char *prompt) {
    cfd_completion_result_t cr = cfd_completion_get(rl->sess, rl->buf, rl->cursor);
    if (cr.count == 0) {
        /* bell */
        putchar('\a'); fflush(stdout);
    } else if (cr.count == 1) {
        /* complete the word */
        int start = rl->cursor;
        while (start > 0 && rl->buf[start-1] != ' ' && rl->buf[start-1] != '\t') start--;
        int wlen = cr.common_prefix_len;
        char *match = cr.matches[0];
        /* replace word from start to cursor with match */
        int match_len = (int)strlen(match);
        int tail_len  = rl->len - rl->cursor;
        memmove(rl->buf + start + match_len, rl->buf + rl->cursor, tail_len);
        memcpy(rl->buf + start, match, match_len);
        rl->len    = start + match_len + tail_len;
        rl->cursor = start + match_len;
        rl->buf[rl->len] = '\0';
    } else {
        /* show all options */
        printf("\n");
        for (int i = 0; i < cr.count; i++) {
            printf("  %s", cr.matches[i]);
            if ((i + 1) % 4 == 0 || i == cr.count - 1) printf("\n");
        }
        rl_refresh(rl, prompt);
        (void)wlen;
    }
    cfd_completion_result_free(&cr);
}

char *cfd_readline_read(cfd_readline_t *rl, const char *prompt) {
    rl->len = rl->cursor = 0;
    rl->buf[0] = '\0';
    if (rl->history) cfd_history_reset_pos(rl->history);

    if (prompt) { fputs(prompt, stdout); fflush(stdout); }

    cfd_platform_raw_mode_enter();

    while (true) {
        int key = cfd_read_key();
        if (key < 0) { /* EOF */
            cfd_platform_raw_mode_exit();
            if (rl->len == 0) return NULL;
            break;
        }

        if (key == KEY_ENTER || key == '\n') break;

        if (key == KEY_BACKSPACE || key == 127 || key == '\b') {
            rl_delete_back(rl);
        } else if (key == KEY_DELETE) {
            rl_delete_fwd(rl);
        } else if (key == KEY_LEFT) {
            if (rl->cursor > 0) rl->cursor--;
        } else if (key == KEY_RIGHT) {
            if (rl->cursor < rl->len) rl->cursor++;
        } else if (key == KEY_HOME || key == KEY_CTRL('A')) {
            rl->cursor = 0;
        } else if (key == KEY_END || key == KEY_CTRL('E')) {
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
            /* kill to beginning */
            memmove(rl->buf, rl->buf + rl->cursor, rl->len - rl->cursor);
            rl->len    -= rl->cursor;
            rl->cursor  = 0;
            rl->buf[rl->len] = '\0';
        } else if (key == KEY_CTRL('W')) {
            /* kill word backward */
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
