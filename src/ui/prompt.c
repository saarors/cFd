#include "prompt.h"
#include "color.h"
#include "theme.h"
#include "display.h"
#include "../platform/platform.h"
#include "../utils/mem.h"
#include "../utils/str_utils.h"
#include "../core/session.h"
#include "../../include/config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#endif

static char g_prompt_fmt[CFD_MAX_PROMPT] = "\\u@\\h \\w> ";

void cfd_prompt_set_format(const char *fmt) {
    if (fmt) strncpy(g_prompt_fmt, fmt, CFD_MAX_PROMPT - 1);
}

const char *cfd_prompt_get_format(void) {
    return g_prompt_fmt;
}

char *cfd_prompt_segment_user(void) {
    const char *u = cfd_platform_getenv("USERNAME");
    if (!u) u = cfd_platform_getenv("USER");
    if (!u) u = "user";
    const cfd_theme_t *t = cfd_theme_get();
    return cfd_sprintf("%s%s%s", cfd_color_fg(t->prompt_user), u, cfd_color_reset());
}

char *cfd_prompt_segment_host(void) {
    char host[256] = "localhost";
#ifdef _WIN32
    DWORD sz = sizeof(host);
    GetComputerNameA(host, &sz);
#else
    gethostname(host, sizeof(host));
#endif
    const cfd_theme_t *t = cfd_theme_get();
    return cfd_sprintf("%s%s%s", cfd_color_fg(t->prompt_host), host, cfd_color_reset());
}

char *cfd_prompt_segment_cwd(cfd_session_t *sess) {
    const char *cwd = sess ? sess->cwd : ".";
    const char *home = cfd_platform_get_home();
    char display[CFD_MAX_PATH];
    if (home && strncmp(cwd, home, strlen(home)) == 0) {
        snprintf(display, sizeof(display), "~%s", cwd + strlen(home));
    } else {
        strncpy(display, cwd, sizeof(display) - 1);
    }
    cfd_free((void*)home);
    const cfd_theme_t *t = cfd_theme_get();
    return cfd_sprintf("%s%s%s", cfd_color_fg(t->prompt_path), display, cfd_color_reset());
}

char *cfd_prompt_segment_status(int last_exit) {
    if (last_exit == 0) return cfd_strdup("");
    const cfd_theme_t *t = cfd_theme_get();
    return cfd_sprintf("%s[%d]%s ", cfd_color_fg(t->error_color), last_exit, cfd_color_reset());
}

char *cfd_prompt_segment_git(void) {
    /* Simple git branch detection */
    FILE *f = popen("git rev-parse --abbrev-ref HEAD 2>/dev/null", "r");
    if (!f) return cfd_strdup("");
    char branch[128] = {0};
    if (!fgets(branch, sizeof(branch), f)) { pclose(f); return cfd_strdup(""); }
    pclose(f);
    size_t len = strlen(branch);
    while (len > 0 && (branch[len-1] == '\n' || branch[len-1] == '\r')) branch[--len] = '\0';
    if (!*branch) return cfd_strdup("");
    return cfd_sprintf("%s(%s)%s ", cfd_color_fg(CFD_COLOR_BRIGHT_MAGENTA),
                       branch, cfd_color_reset());
}

char *cfd_prompt_build(cfd_session_t *sess) {
    const char *fmt = g_prompt_fmt;
    char *out = cfd_strdup("");
    for (const char *p = fmt; *p; p++) {
        if (*p == '\\' && *(p+1)) {
            char *seg = NULL;
            switch (*++p) {
                case 'u': seg = cfd_prompt_segment_user(); break;
                case 'h': seg = cfd_prompt_segment_host(); break;
                case 'w': seg = cfd_prompt_segment_cwd(sess); break;
                case 'g': seg = cfd_prompt_segment_git(); break;
                case 'n': seg = cfd_strdup("\n"); break;
                case '$': seg = cfd_strdup("$ "); break;
                default: {
                    char tmp[3] = {'\\', *p, '\0'};
                    seg = cfd_strdup(tmp);
                }
            }
            if (seg) {
                char *tmp = cfd_sprintf("%s%s", out, seg);
                cfd_free(out); cfd_free(seg);
                out = tmp;
            }
        } else {
            char tmp[2] = {*p, '\0'};
            char *combined = cfd_sprintf("%s%s", out, tmp);
            cfd_free(out);
            out = combined;
        }
    }
    return out;
}

void cfd_prompt_print(cfd_session_t *sess) {
    char *p = cfd_prompt_build(sess);
    fputs(p, stdout);
    fflush(stdout);
    cfd_free(p);
}
