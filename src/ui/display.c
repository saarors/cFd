#include "display.h"
#include "theme.h"
#include "../platform/platform.h"
#include "../utils/mem.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

void display_cursor_up(int n)      { printf("\033[%dA", n); fflush(stdout); }
void display_cursor_down(int n)    { printf("\033[%dB", n); fflush(stdout); }
void display_cursor_right(int n)   { printf("\033[%dC", n); fflush(stdout); }
void display_cursor_left(int n)    { printf("\033[%dD", n); fflush(stdout); }
void display_cursor_move(int r, int c) { printf("\033[%d;%dH", r, c); fflush(stdout); }
void display_cursor_save(void)     { printf("\033[s"); fflush(stdout); }
void display_cursor_restore(void)  { printf("\033[u"); fflush(stdout); }
void display_cursor_show(bool show){ printf(show ? "\033[?25h" : "\033[?25l"); fflush(stdout); }

void display_erase_line(void)      { printf("\033[2K\r"); fflush(stdout); }
void display_erase_to_end(void)    { printf("\033[K");    fflush(stdout); }
void display_erase_to_start(void)  { printf("\033[1K");   fflush(stdout); }
void display_erase_screen(void)    { printf("\033[2J\033[H"); fflush(stdout); }

void display_print(const char *s)  { fputs(s, stdout); fflush(stdout); }

void display_println(const char *s){ puts(s); fflush(stdout); }

void display_printf(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    vprintf(fmt, ap); va_end(ap);
    fflush(stdout);
}

void display_error(const char *fmt, ...) {
    const cfd_theme_t *t = cfd_theme_get();
    fprintf(stderr, "%s", cfd_color_fg(t->error_color));
    va_list ap; va_start(ap, fmt);
    vfprintf(stderr, fmt, ap); va_end(ap);
    fprintf(stderr, "%s\n", cfd_color_reset());
    fflush(stderr);
}

void display_warn(const char *fmt, ...) {
    const cfd_theme_t *t = cfd_theme_get();
    fprintf(stderr, "%s", cfd_color_fg(t->warn_color));
    va_list ap; va_start(ap, fmt);
    vfprintf(stderr, fmt, ap); va_end(ap);
    fprintf(stderr, "%s\n", cfd_color_reset());
    fflush(stderr);
}

void display_info(const char *fmt, ...) {
    const cfd_theme_t *t = cfd_theme_get();
    printf("%s", cfd_color_fg(t->info_color));
    va_list ap; va_start(ap, fmt);
    vprintf(fmt, ap); va_end(ap);
    printf("%s\n", cfd_color_reset());
    fflush(stdout);
}

void display_success(const char *fmt, ...) {
    const cfd_theme_t *t = cfd_theme_get();
    printf("%s", cfd_color_fg(t->success_color));
    va_list ap; va_start(ap, fmt);
    vprintf(fmt, ap); va_end(ap);
    printf("%s\n", cfd_color_reset());
    fflush(stdout);
}

void display_divider(char ch, int width) {
    if (width <= 0) {
        cfd_platform_get_terminal_size(&width, NULL);
        if (width <= 0) width = 80;
    }
    for (int i = 0; i < width; i++) putchar(ch);
    putchar('\n');
    fflush(stdout);
}

void display_banner(const char *text) {
    int cols = 80;
    cfd_platform_get_terminal_size(&cols, NULL);
    display_divider('=', cols);
    int len = (int)strlen(text);
    int pad = (cols - len) / 2;
    for (int i = 0; i < pad; i++) putchar(' ');
    puts(text);
    display_divider('=', cols);
}

void display_get_size(int *cols, int *rows) {
    cfd_platform_get_terminal_size(cols, rows);
}
