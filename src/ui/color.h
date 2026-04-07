#ifndef CFD_COLOR_H
#define CFD_COLOR_H

#include <stdbool.h>

/* ANSI color codes */
#define COLOR_RESET       "\033[0m"
#define COLOR_BOLD        "\033[1m"
#define COLOR_DIM         "\033[2m"
#define COLOR_UNDERLINE   "\033[4m"
#define COLOR_BLINK       "\033[5m"
#define COLOR_REVERSE     "\033[7m"

/* Foreground colors */
#define FG_BLACK          "\033[30m"
#define FG_RED            "\033[31m"
#define FG_GREEN          "\033[32m"
#define FG_YELLOW         "\033[33m"
#define FG_BLUE           "\033[34m"
#define FG_MAGENTA        "\033[35m"
#define FG_CYAN           "\033[36m"
#define FG_WHITE          "\033[37m"
#define FG_BRIGHT_BLACK   "\033[90m"
#define FG_BRIGHT_RED     "\033[91m"
#define FG_BRIGHT_GREEN   "\033[92m"
#define FG_BRIGHT_YELLOW  "\033[93m"
#define FG_BRIGHT_BLUE    "\033[94m"
#define FG_BRIGHT_MAGENTA "\033[95m"
#define FG_BRIGHT_CYAN    "\033[96m"
#define FG_BRIGHT_WHITE   "\033[97m"

/* Background colors */
#define BG_BLACK          "\033[40m"
#define BG_RED            "\033[41m"
#define BG_GREEN          "\033[42m"
#define BG_YELLOW         "\033[43m"
#define BG_BLUE           "\033[44m"
#define BG_MAGENTA        "\033[45m"
#define BG_CYAN           "\033[46m"
#define BG_WHITE          "\033[47m"

/* Named color indices */
typedef enum {
    CFD_COLOR_NONE    = 0,
    CFD_COLOR_BLACK,
    CFD_COLOR_RED,
    CFD_COLOR_GREEN,
    CFD_COLOR_YELLOW,
    CFD_COLOR_BLUE,
    CFD_COLOR_MAGENTA,
    CFD_COLOR_CYAN,
    CFD_COLOR_WHITE,
    CFD_COLOR_BRIGHT_BLACK,
    CFD_COLOR_BRIGHT_RED,
    CFD_COLOR_BRIGHT_GREEN,
    CFD_COLOR_BRIGHT_YELLOW,
    CFD_COLOR_BRIGHT_BLUE,
    CFD_COLOR_BRIGHT_MAGENTA,
    CFD_COLOR_BRIGHT_CYAN,
    CFD_COLOR_BRIGHT_WHITE,
    CFD_COLOR_COUNT
} cfd_color_id_t;

void        cfd_color_init(void);
bool        cfd_color_enabled(void);
void        cfd_color_set_enabled(bool enabled);

const char *cfd_color_fg(cfd_color_id_t id);
const char *cfd_color_bg(cfd_color_id_t id);
const char *cfd_color_reset(void);

/* Print with color */
void cfd_color_print(cfd_color_id_t fg, cfd_color_id_t bg, const char *text);
void cfd_color_printf(cfd_color_id_t fg, cfd_color_id_t bg, const char *fmt, ...);

/* 256-color escape */
char *cfd_color_256_fg(int n);
char *cfd_color_256_bg(int n);

#endif /* CFD_COLOR_H */
