#include "theme.h"
#include "../utils/str_utils.h"
#include <string.h>

const cfd_theme_t cfd_theme_default = {
    .name          = "default",
    .prompt_user   = CFD_COLOR_BRIGHT_GREEN,
    .prompt_host   = CFD_COLOR_GREEN,
    .prompt_path   = CFD_COLOR_BRIGHT_BLUE,
    .prompt_symbol = CFD_COLOR_WHITE,
    .error_color   = CFD_COLOR_BRIGHT_RED,
    .success_color = CFD_COLOR_BRIGHT_GREEN,
    .info_color    = CFD_COLOR_BRIGHT_CYAN,
    .warn_color    = CFD_COLOR_BRIGHT_YELLOW,
    .dir_color     = CFD_COLOR_BRIGHT_BLUE,
    .file_color    = CFD_COLOR_WHITE,
    .exec_color    = CFD_COLOR_BRIGHT_GREEN,
    .link_color    = CFD_COLOR_BRIGHT_CYAN
};

const cfd_theme_t cfd_theme_dark = {
    .name          = "dark",
    .prompt_user   = CFD_COLOR_CYAN,
    .prompt_host   = CFD_COLOR_BRIGHT_CYAN,
    .prompt_path   = CFD_COLOR_YELLOW,
    .prompt_symbol = CFD_COLOR_BRIGHT_WHITE,
    .error_color   = CFD_COLOR_RED,
    .success_color = CFD_COLOR_GREEN,
    .info_color    = CFD_COLOR_CYAN,
    .warn_color    = CFD_COLOR_YELLOW,
    .dir_color     = CFD_COLOR_YELLOW,
    .file_color    = CFD_COLOR_BRIGHT_WHITE,
    .exec_color    = CFD_COLOR_GREEN,
    .link_color    = CFD_COLOR_CYAN
};

const cfd_theme_t cfd_theme_minimal = {
    .name          = "minimal",
    .prompt_user   = CFD_COLOR_NONE,
    .prompt_host   = CFD_COLOR_NONE,
    .prompt_path   = CFD_COLOR_NONE,
    .prompt_symbol = CFD_COLOR_NONE,
    .error_color   = CFD_COLOR_NONE,
    .success_color = CFD_COLOR_NONE,
    .info_color    = CFD_COLOR_NONE,
    .warn_color    = CFD_COLOR_NONE,
    .dir_color     = CFD_COLOR_NONE,
    .file_color    = CFD_COLOR_NONE,
    .exec_color    = CFD_COLOR_NONE,
    .link_color    = CFD_COLOR_NONE
};

static const cfd_theme_t *g_theme = &cfd_theme_default;

void cfd_theme_init(void) {
    g_theme = &cfd_theme_default;
}

const cfd_theme_t *cfd_theme_get(void) {
    return g_theme;
}

void cfd_theme_set(const char *name) {
    if (!name) return;
    if (strcmp(name, "dark")    == 0) { g_theme = &cfd_theme_dark;    return; }
    if (strcmp(name, "minimal") == 0) { g_theme = &cfd_theme_minimal; return; }
    g_theme = &cfd_theme_default;
}
