#include "color.h"
#include "../platform/platform.h"
#include "../utils/mem.h"
#include "../utils/str_utils.h"
#include <stdio.h>
#include <stdarg.h>

static bool g_color_enabled = false;

static const char *s_fg_codes[CFD_COLOR_COUNT] = {
    "",
    "\033[30m", "\033[31m", "\033[32m", "\033[33m",
    "\033[34m", "\033[35m", "\033[36m", "\033[37m",
    "\033[90m", "\033[91m", "\033[92m", "\033[93m",
    "\033[94m", "\033[95m", "\033[96m", "\033[97m"
};

static const char *s_bg_codes[CFD_COLOR_COUNT] = {
    "",
    "\033[40m", "\033[41m", "\033[42m", "\033[43m",
    "\033[44m", "\033[45m", "\033[46m", "\033[47m",
    "\033[100m","\033[101m","\033[102m","\033[103m",
    "\033[104m","\033[105m","\033[106m","\033[107m"
};

void cfd_color_init(void) {
    g_color_enabled = cfd_platform_color_supported();
}

bool cfd_color_enabled(void) {
    return g_color_enabled;
}

void cfd_color_set_enabled(bool enabled) {
    g_color_enabled = enabled;
}

const char *cfd_color_fg(cfd_color_id_t id) {
    if (!g_color_enabled || id <= CFD_COLOR_NONE || id >= CFD_COLOR_COUNT) return "";
    return s_fg_codes[id];
}

const char *cfd_color_bg(cfd_color_id_t id) {
    if (!g_color_enabled || id <= CFD_COLOR_NONE || id >= CFD_COLOR_COUNT) return "";
    return s_bg_codes[id];
}

const char *cfd_color_reset(void) {
    return g_color_enabled ? COLOR_RESET : "";
}

void cfd_color_print(cfd_color_id_t fg, cfd_color_id_t bg, const char *text) {
    if (g_color_enabled) {
        printf("%s%s%s%s", cfd_color_fg(fg), cfd_color_bg(bg), text, COLOR_RESET);
    } else {
        printf("%s", text);
    }
}

void cfd_color_printf(cfd_color_id_t fg, cfd_color_id_t bg, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char buf[4096];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    cfd_color_print(fg, bg, buf);
}

char *cfd_color_256_fg(int n) {
    if (!g_color_enabled) return cfd_strdup("");
    return cfd_sprintf("\033[38;5;%dm", n);
}

char *cfd_color_256_bg(int n) {
    if (!g_color_enabled) return cfd_strdup("");
    return cfd_sprintf("\033[48;5;%dm", n);
}
