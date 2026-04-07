#include "ui.h"
#include "../platform/platform.h"
#include "../utils/mem.h"
#include "../utils/str_utils.h"
#include "../../include/version.h"
#include <stdio.h>
#include <string.h>

void cfd_ui_init(void) {
    cfd_color_init();
    cfd_theme_init();
}

void cfd_ui_shutdown(void) {
    /* restore normal output state */
    printf("%s", cfd_color_reset());
    fflush(stdout);
}

void cfd_ui_print_banner(void) {
    const cfd_theme_t *t = cfd_theme_get();
    int cols = 80;
    cfd_platform_get_terminal_size(&cols, NULL);

    printf("\n");
    printf("%s", cfd_color_fg(t->prompt_symbol));
    display_divider('-', cols);
    printf("  %s%s%s  version %s\n",
           cfd_color_fg(t->prompt_user), CFD_NAME, cfd_color_fg(t->prompt_symbol),
           CFD_VERSION);
    printf("  %s\n", CFD_DESCRIPTION);
    printf("  Type 'help' to list available commands.\n");
    display_divider('-', cols);
    printf("%s\n", cfd_color_reset());
    fflush(stdout);
}

void cfd_ui_set_title(const char *title) {
    /* OSC escape for terminal title */
    printf("\033]0;%s\007", title);
    fflush(stdout);
}

int cfd_ui_pager(const char *text) {
    if (!text) return 0;
    int rows = 24;
    cfd_platform_get_terminal_size(NULL, &rows);
    if (rows < 4) rows = 24;

    const char *p = text;
    int line_count = 0;

    while (*p) {
        /* Count to next newline */
        const char *nl = strchr(p, '\n');
        size_t len = nl ? (size_t)(nl - p + 1) : strlen(p);
        fwrite(p, 1, len, stdout);
        fflush(stdout);
        p += len;
        line_count++;

        if (line_count >= rows - 2 && *p) {
            printf("%s-- More -- (q to quit, Enter to continue) --%s",
                   cfd_color_fg(CFD_COLOR_BRIGHT_WHITE), cfd_color_reset());
            fflush(stdout);
            int c = getchar();
            printf("\r\033[K");
            fflush(stdout);
            if (c == 'q' || c == 'Q') break;
            line_count = 0;
        }
    }
    return 0;
}
