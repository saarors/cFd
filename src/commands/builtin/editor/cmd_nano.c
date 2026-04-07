#include "cmd_nano.h"
#include "../../../utils/mem.h"
#include "../../../platform/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/* ---- Key constants ---- */
#define KEY_CTRL(c)   ((c) - '@')
#define KEY_UP        1000
#define KEY_DOWN      1001
#define KEY_LEFT      1002
#define KEY_RIGHT     1003
#define KEY_HOME      1004
#define KEY_END       1005
#define KEY_PGUP      1006
#define KEY_PGDN      1007
#define KEY_DEL       1008
#define KEY_BACKSPACE 127
#define KEY_ENTER     '\r'
#define KEY_ESC       27

/* ---- Editor state ---- */
typedef struct {
    char **lines;
    int    line_count;
    int    line_cap;
    int    cx, cy;          /* cursor column, row */
    int    scroll_top;      /* first visible line index */
    int    cols, rows;      /* terminal dimensions */
    char   filename[512];
    bool   modified;
    char   cut_buf[4096];
    bool   show_line_nums;
    bool   running;
} nano_state_t;

/* ---- Line management ---- */
static void nano_ensure_cap(nano_state_t *s) {
    if (s->line_count >= s->line_cap) {
        int new_cap = s->line_cap ? s->line_cap * 2 : 16;
        char **new_lines = (char **)cfd_realloc(s->lines, (size_t)new_cap * sizeof(char *));
        if (!new_lines) return;
        s->lines    = new_lines;
        s->line_cap = new_cap;
    }
}

static void nano_insert_line(nano_state_t *s, int at, const char *text) {
    nano_ensure_cap(s);
    if (at < 0) at = 0;
    if (at > s->line_count) at = s->line_count;
    memmove(s->lines + at + 1, s->lines + at,
            (size_t)(s->line_count - at) * sizeof(char *));
    s->lines[at] = cfd_strdup(text ? text : "");
    s->line_count++;
}

static void nano_delete_line(nano_state_t *s, int at) {
    if (at < 0 || at >= s->line_count) return;
    cfd_free(s->lines[at]);
    memmove(s->lines + at, s->lines + at + 1,
            (size_t)(s->line_count - at - 1) * sizeof(char *));
    s->line_count--;
}

/* ---- File I/O ---- */
static int nano_load_file(nano_state_t *s, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        /* New file - start with one empty line */
        nano_insert_line(s, 0, "");
        return 0;
    }

    char buf[4096];
    while (fgets(buf, sizeof(buf), fp)) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n') buf[--len] = '\0';
        if (len > 0 && buf[len - 1] == '\r') buf[--len] = '\0';
        nano_insert_line(s, s->line_count, buf);
    }
    fclose(fp);

    if (s->line_count == 0)
        nano_insert_line(s, 0, "");

    return 0;
}

static int nano_save_file(nano_state_t *s) {
    if (s->filename[0] == '\0') return 1;

    FILE *fp = fopen(s->filename, "w");
    if (!fp) return 1;

    for (int i = 0; i < s->line_count; i++) {
        fputs(s->lines[i], fp);
        fputc('\n', fp);
    }
    fclose(fp);
    s->modified = false;
    return 0;
}

/* ---- Terminal helpers ---- */
static void nano_move_cursor(int row, int col) {
    printf("\033[%d;%dH", row, col);
}

static void nano_clear_line(void) {
    printf("\033[2K");
}

/* ---- Key reading (cross-platform) ---- */
static int nano_read_key(void) {
#ifdef _WIN32
    int c = cfd_platform_read_key();
    if (c == 0 || c == 0xE0) {
        int c2 = cfd_platform_read_key();
        switch (c2) {
            case 72: return KEY_UP;
            case 80: return KEY_DOWN;
            case 75: return KEY_LEFT;
            case 77: return KEY_RIGHT;
            case 71: return KEY_HOME;
            case 79: return KEY_END;
            case 73: return KEY_PGUP;
            case 81: return KEY_PGDN;
            case 83: return KEY_DEL;
            default: return c2;
        }
    }
    if (c == KEY_BACKSPACE || c == 8) return KEY_BACKSPACE;
    return c;
#else
    int c = cfd_platform_read_key();
    if (c == KEY_ESC) {
        int c2 = cfd_platform_read_key();
        if (c2 == '[') {
            int c3 = cfd_platform_read_key();
            switch (c3) {
                case 'A': return KEY_UP;
                case 'B': return KEY_DOWN;
                case 'C': return KEY_RIGHT;
                case 'D': return KEY_LEFT;
                case 'H': return KEY_HOME;
                case 'F': return KEY_END;
                case '1': { cfd_platform_read_key(); return KEY_HOME; }
                case '4': { cfd_platform_read_key(); return KEY_END; }
                case '5': { cfd_platform_read_key(); return KEY_PGUP; }
                case '6': { cfd_platform_read_key(); return KEY_PGDN; }
                case '3': { cfd_platform_read_key(); return KEY_DEL; }
                default:  return KEY_ESC;
            }
        }
        return KEY_ESC;
    }
    if (c == 8 || c == 127) return KEY_BACKSPACE;
    return c;
#endif
}

/* ---- Rendering ---- */
static void nano_render(nano_state_t *s) {
    cfd_platform_get_terminal_size(&s->cols, &s->rows);

    printf("\033[?25l"); /* hide cursor */
    printf("\033[H");    /* home */

    int line_num_width = s->show_line_nums ? 5 : 0;
    int text_cols      = s->cols - line_num_width;
    if (text_cols < 1) text_cols = 1;

    int editor_rows = s->rows - 2; /* reserve top title bar + bottom status */
    if (editor_rows < 1) editor_rows = 1;

    /* Top title bar */
    printf("\033[7m"); /* reverse video */
    printf("  GNU nano (cFd)");
    int pad = s->cols - 16 - (int)strlen(s->filename) - (s->modified ? 2 : 0) - 2;
    for (int i = 0; i < pad / 2 && i < s->cols; i++) putchar(' ');
    printf("%s%s", s->filename[0] ? s->filename : "New Buffer", s->modified ? " *" : "");
    for (int i = 0; i < s->cols; i++) putchar(' ');
    printf("\033[0m\r\n");

    /* Content lines */
    for (int r = 0; r < editor_rows; r++) {
        nano_clear_line();
        int line_idx = s->scroll_top + r;

        if (s->show_line_nums) {
            if (line_idx < s->line_count)
                printf("%4d ", line_idx + 1);
            else
                printf("     ");
        }

        if (line_idx < s->line_count) {
            const char *line = s->lines[line_idx];
            int len = (int)strlen(line);
            /* Horizontal scrolling: show text_cols chars */
            int start_col = 0; /* could scroll horizontal too, keeping simple */
            int shown = 0;
            for (int c = start_col; c < len && shown < text_cols; c++, shown++) {
                int ch = (unsigned char)line[c];
                if (ch == '\t') {
                    int spaces = 4 - (shown % 4);
                    for (int sp = 0; sp < spaces && shown < text_cols; sp++, shown++)
                        putchar(' ');
                    shown--; /* loop will ++ */
                } else if (ch < 32) {
                    printf("^%c", '@' + ch);
                    shown++;
                } else {
                    putchar(ch);
                }
            }
        }
        printf("\r\n");
    }

    /* Status bar */
    printf("\033[7m");
    nano_clear_line();
    char status[512];
    snprintf(status, sizeof(status),
        "^X Exit  ^S Save  ^W Search  ^K Cut  ^U Paste | %s%s | Ln %d, Col %d",
        s->filename[0] ? s->filename : "New Buffer",
        s->modified ? " [Modified]" : "",
        s->cy + 1,
        s->cx + 1);
    int slen = (int)strlen(status);
    if (slen > s->cols) slen = s->cols;
    fwrite(status, 1, (size_t)slen, stdout);
    for (int i = slen; i < s->cols; i++) putchar(' ');
    printf("\033[0m");

    /* Position cursor */
    int screen_row = s->cy - s->scroll_top + 2; /* +2 for title bar */
    int screen_col = s->cx + 1 + line_num_width;
    nano_move_cursor(screen_row, screen_col);

    printf("\033[?25h"); /* show cursor */
    fflush(stdout);
}

/* ---- Prompt input at bottom bar ---- */
static void nano_prompt(nano_state_t *s, const char *prompt, char *buf, int bufsz) {
    nano_move_cursor(s->rows, 1);
    printf("\033[2K\033[7m%s\033[0m", prompt);
    fflush(stdout);

    int pos = 0;
    buf[0] = '\0';

    for (;;) {
        int c = nano_read_key();
        if (c == KEY_ENTER || c == '\n') break;
        if (c == KEY_ESC || c == KEY_CTRL('C')) { buf[0] = '\0'; break; }
        if ((c == KEY_BACKSPACE) && pos > 0) {
            buf[--pos] = '\0';
            printf("\b \b");
            fflush(stdout);
        } else if (c >= 32 && c < 256 && pos < bufsz - 1) {
            buf[pos++] = (char)c;
            buf[pos]   = '\0';
            putchar(c);
            fflush(stdout);
        }
    }
}

/* ---- Search ---- */
static void nano_search(nano_state_t *s) {
    char query[256];
    nano_prompt(s, "Search: ", query, sizeof(query));
    if (query[0] == '\0') return;

    for (int row = s->cy; row < s->line_count; row++) {
        int start_col = (row == s->cy) ? s->cx + 1 : 0;
        const char *found = strstr(s->lines[row] + start_col, query);
        if (found) {
            s->cy = row;
            s->cx = (int)(found - s->lines[row]);
            /* Scroll into view */
            if (s->cy < s->scroll_top)
                s->scroll_top = s->cy;
            int editor_rows = s->rows - 2;
            if (s->cy >= s->scroll_top + editor_rows)
                s->scroll_top = s->cy - editor_rows + 1;
            return;
        }
    }

    /* Wrap around */
    for (int row = 0; row <= s->cy; row++) {
        const char *found = strstr(s->lines[row], query);
        if (found) {
            s->cy = row;
            s->cx = (int)(found - s->lines[row]);
            s->scroll_top = 0;
            return;
        }
    }
}

/* ---- Insert character ---- */
static void nano_insert_char(nano_state_t *s, char c) {
    if (s->cy >= s->line_count) {
        nano_insert_line(s, s->line_count, "");
        s->cy = s->line_count - 1;
    }

    char *line = s->lines[s->cy];
    int   len  = (int)strlen(line);

    char *new_line = (char *)cfd_malloc((size_t)(len + 2));
    if (!new_line) return;

    memcpy(new_line, line, (size_t)s->cx);
    new_line[s->cx] = c;
    memcpy(new_line + s->cx + 1, line + s->cx, (size_t)(len - s->cx + 1));

    cfd_free(s->lines[s->cy]);
    s->lines[s->cy] = new_line;
    s->cx++;
    s->modified = true;
}

/* ---- Backspace ---- */
static void nano_backspace(nano_state_t *s) {
    if (s->cx > 0) {
        char *line = s->lines[s->cy];
        int   len  = (int)strlen(line);
        memmove(line + s->cx - 1, line + s->cx, (size_t)(len - s->cx + 1));
        s->cx--;
        s->modified = true;
    } else if (s->cy > 0) {
        /* Join with previous line */
        char *prev = s->lines[s->cy - 1];
        char *curr = s->lines[s->cy];
        int prev_len = (int)strlen(prev);
        int curr_len = (int)strlen(curr);

        char *joined = (char *)cfd_malloc((size_t)(prev_len + curr_len + 1));
        if (!joined) return;
        memcpy(joined, prev, (size_t)prev_len);
        memcpy(joined + prev_len, curr, (size_t)(curr_len + 1));

        cfd_free(s->lines[s->cy - 1]);
        s->lines[s->cy - 1] = joined;
        nano_delete_line(s, s->cy);

        s->cx = prev_len;
        s->cy--;
        s->modified = true;
    }
}

/* ---- Delete key ---- */
static void nano_delete_char(nano_state_t *s) {
    if (s->cy >= s->line_count) return;
    char *line = s->lines[s->cy];
    int   len  = (int)strlen(line);

    if (s->cx < len) {
        memmove(line + s->cx, line + s->cx + 1, (size_t)(len - s->cx));
        s->modified = true;
    } else if (s->cy + 1 < s->line_count) {
        /* Join with next line */
        char *next = s->lines[s->cy + 1];
        int next_len = (int)strlen(next);

        char *joined = (char *)cfd_malloc((size_t)(len + next_len + 1));
        if (!joined) return;
        memcpy(joined, line, (size_t)len);
        memcpy(joined + len, next, (size_t)(next_len + 1));

        cfd_free(s->lines[s->cy]);
        s->lines[s->cy] = joined;
        nano_delete_line(s, s->cy + 1);
        s->modified = true;
    }
}

/* ---- Enter (split line) ---- */
static void nano_enter(nano_state_t *s) {
    if (s->cy >= s->line_count) {
        nano_insert_line(s, s->line_count, "");
        s->cy = s->line_count - 1;
        s->cx = 0;
        return;
    }

    char *line = s->lines[s->cy];
    int   len  = (int)strlen(line);

    char *second_part = cfd_strdup(line + s->cx);
    char *first_part  = (char *)cfd_malloc((size_t)(s->cx + 1));
    if (!second_part || !first_part) {
        cfd_free(second_part);
        cfd_free(first_part);
        return;
    }
    memcpy(first_part, line, (size_t)s->cx);
    first_part[s->cx] = '\0';
    (void)len;

    cfd_free(s->lines[s->cy]);
    s->lines[s->cy] = first_part;

    nano_insert_line(s, s->cy + 1, second_part);
    cfd_free(second_part);

    s->cy++;
    s->cx = 0;
    s->modified = true;
}

/* ---- Cut line ---- */
static void nano_cut_line(nano_state_t *s) {
    if (s->cy >= s->line_count) return;
    strncpy(s->cut_buf, s->lines[s->cy], sizeof(s->cut_buf) - 1);
    s->cut_buf[sizeof(s->cut_buf) - 1] = '\0';

    if (s->line_count > 1) {
        nano_delete_line(s, s->cy);
        if (s->cy >= s->line_count) s->cy = s->line_count - 1;
    } else {
        cfd_free(s->lines[0]);
        s->lines[0] = cfd_strdup("");
    }
    s->cx = 0;
    s->modified = true;
}

/* ---- Paste (uncut) ---- */
static void nano_paste(nano_state_t *s) {
    if (s->cut_buf[0] == '\0') return;
    nano_insert_line(s, s->cy, s->cut_buf);
    s->modified = true;
}

/* ---- Clamp cursor ---- */
static void nano_clamp_cursor(nano_state_t *s) {
    if (s->cy < 0)               s->cy = 0;
    if (s->cy >= s->line_count)  s->cy = s->line_count - 1;
    if (s->cy < 0)               s->cy = 0;

    if (s->line_count > 0 && s->cy < s->line_count) {
        int len = (int)strlen(s->lines[s->cy]);
        if (s->cx < 0)    s->cx = 0;
        if (s->cx > len)  s->cx = len;
    } else {
        s->cx = 0;
    }

    /* Scroll */
    int editor_rows = s->rows - 2;
    if (editor_rows < 1) editor_rows = 1;

    if (s->cy < s->scroll_top)
        s->scroll_top = s->cy;
    if (s->cy >= s->scroll_top + editor_rows)
        s->scroll_top = s->cy - editor_rows + 1;
}

/* ---- Ask yes/no ---- */
static bool nano_ask_yn(nano_state_t *s, const char *question) {
    nano_move_cursor(s->rows, 1);
    printf("\033[2K\033[7m%s (Y/N) \033[0m", question);
    fflush(stdout);

    for (;;) {
        int c = nano_read_key();
        if (c == 'y' || c == 'Y') return true;
        if (c == 'n' || c == 'N' || c == KEY_ESC || c == KEY_CTRL('C')) return false;
    }
}

/* ---- Help screen ---- */
static void nano_show_help(nano_state_t *s) {
    printf("\033[2J\033[H");
    printf("\033[7m  cFd nano - Help  \033[0m\n\n");
    printf("  ^X  Exit editor\n");
    printf("  ^S  Save file\n");
    printf("  ^O  Save as (write out)\n");
    printf("  ^W  Search\n");
    printf("  ^K  Cut line\n");
    printf("  ^U  Paste (uncut)\n");
    printf("  ^G  Show this help\n");
    printf("  ^L  Refresh/redraw screen\n");
    printf("  ^A  Go to start of line\n");
    printf("  ^E  Go to end of line\n");
    printf("\n  Arrow keys: move cursor\n");
    printf("  Home/End: start/end of line\n");
    printf("  PgUp/PgDn: page up/down\n");
    printf("  Backspace: delete char before cursor\n");
    printf("  Delete: delete char at cursor\n");
    printf("  Enter: insert new line\n");
    printf("\n  Press any key to return...\n");
    fflush(stdout);
    nano_read_key();
    (void)s;
}

/* ---- Main editor loop ---- */
int cmd_nano(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    nano_state_t *s = CFD_NEW(nano_state_t);
    if (!s) return 1;

    s->line_cap  = 16;
    s->lines     = (char **)cfd_malloc((size_t)s->line_cap * sizeof(char *));
    if (!s->lines) { cfd_free(s); return 1; }

    s->running         = true;
    s->show_line_nums  = false;

    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--linenumbers") == 0) {
            s->show_line_nums = true;
        } else if (argv[i][0] != '-' && s->filename[0] == '\0') {
            strncpy(s->filename, argv[i], sizeof(s->filename) - 1);
        }
    }

    /* Load file */
    nano_load_file(s, s->filename[0] ? s->filename : NULL);

    /* Enter raw mode */
    cfd_platform_raw_mode_enter();
    cfd_platform_get_terminal_size(&s->cols, &s->rows);

    nano_render(s);

    while (s->running) {
        int key = nano_read_key();

        switch (key) {
        case KEY_CTRL('X'): /* Exit */
            if (s->modified) {
                bool save = nano_ask_yn(s, "Save modified buffer?");
                if (save) {
                    if (s->filename[0] == '\0') {
                        nano_prompt(s, "File Name to Write: ",
                                    s->filename, sizeof(s->filename));
                    }
                    nano_save_file(s);
                }
            }
            s->running = false;
            break;

        case KEY_CTRL('S'):
        case KEY_CTRL('O'): /* Save */
            if (s->filename[0] == '\0') {
                nano_prompt(s, "File Name to Write: ",
                            s->filename, sizeof(s->filename));
            }
            if (s->filename[0] != '\0') {
                if (nano_save_file(s) == 0) {
                    nano_move_cursor(s->rows, 1);
                    printf("\033[2K\033[7mFile written.\033[0m");
                    fflush(stdout);
                    cfd_platform_sleep_ms(800);
                } else {
                    nano_move_cursor(s->rows, 1);
                    printf("\033[2K\033[7mError writing file!\033[0m");
                    fflush(stdout);
                    cfd_platform_sleep_ms(800);
                }
            }
            break;

        case KEY_CTRL('W'): /* Search */
            nano_search(s);
            break;

        case KEY_CTRL('K'): /* Cut line */
            nano_cut_line(s);
            break;

        case KEY_CTRL('U'): /* Uncut/Paste */
            nano_paste(s);
            break;

        case KEY_CTRL('G'): /* Help */
            nano_show_help(s);
            break;

        case KEY_CTRL('L'): /* Refresh */
            /* Just re-render */
            break;

        case KEY_CTRL('A'):
        case KEY_HOME:
            s->cx = 0;
            break;

        case KEY_CTRL('E'):
        case KEY_END:
            if (s->cy < s->line_count)
                s->cx = (int)strlen(s->lines[s->cy]);
            break;

        case KEY_UP:
            s->cy--;
            break;

        case KEY_DOWN:
            s->cy++;
            break;

        case KEY_LEFT:
            if (s->cx > 0) {
                s->cx--;
            } else if (s->cy > 0) {
                s->cy--;
                s->cx = (s->cy < s->line_count) ? (int)strlen(s->lines[s->cy]) : 0;
            }
            break;

        case KEY_RIGHT:
            if (s->cy < s->line_count) {
                int len = (int)strlen(s->lines[s->cy]);
                if (s->cx < len) {
                    s->cx++;
                } else if (s->cy + 1 < s->line_count) {
                    s->cy++;
                    s->cx = 0;
                }
            }
            break;

        case KEY_PGUP: {
            int page = s->rows - 3;
            if (page < 1) page = 1;
            s->cy -= page;
            s->scroll_top -= page;
            if (s->scroll_top < 0) s->scroll_top = 0;
            break;
        }

        case KEY_PGDN: {
            int page = s->rows - 3;
            if (page < 1) page = 1;
            s->cy += page;
            break;
        }

        case KEY_BACKSPACE:
            nano_backspace(s);
            break;

        case KEY_DEL:
            nano_delete_char(s);
            break;

        case KEY_ENTER:
        case '\n':
            nano_enter(s);
            break;

        default:
            if (key >= 32 && key < 256) {
                nano_insert_char(s, (char)key);
            }
            break;
        }

        nano_clamp_cursor(s);
        nano_render(s);
    }

    cfd_platform_raw_mode_exit();
    printf("\033[2J\033[H"); /* clear screen */
    fflush(stdout);

    /* Free state */
    for (int i = 0; i < s->line_count; i++)
        cfd_free(s->lines[i]);
    cfd_free(s->lines);
    cfd_free(s);

    return 0;
}

const cfd_command_t builtin_nano = {
    "nano",
    "nano [-l] [file]",
    "Terminal text editor",
    "editor",
    cmd_nano,
    0, -1
};
