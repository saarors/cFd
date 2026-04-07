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
#include <time.h>
#ifdef _WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#endif

static char g_prompt_fmt[CFD_MAX_PROMPT] = "\\e\\u@\\h \\w\\$ ";

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
    char *home = cfd_platform_get_home();
    char display[CFD_MAX_PATH];
    if (home && strncmp(cwd, home, strlen(home)) == 0) {
        snprintf(display, sizeof(display), "~%s", cwd + strlen(home));
    } else {
        strncpy(display, cwd, sizeof(display) - 1);
        display[sizeof(display)-1] = '\0';
    }
    cfd_free(home);
    const cfd_theme_t *t = cfd_theme_get();
    return cfd_sprintf("%s%s%s", cfd_color_fg(t->prompt_path), display, cfd_color_reset());
}

char *cfd_prompt_segment_status(int last_exit) {
    if (last_exit == 0) return cfd_strdup("");
    const cfd_theme_t *t = cfd_theme_get();
    return cfd_sprintf("%s[%d]%s ", cfd_color_fg(t->error_color), last_exit, cfd_color_reset());
}

/* \g — git branch by reading .git/HEAD directly */
char *cfd_prompt_segment_git(void) {
    /* Walk up from cwd looking for .git/HEAD */
    char dir[CFD_MAX_PATH];
    char *cwd = cfd_platform_getcwd();
    if (!cwd) return cfd_strdup("");
    strncpy(dir, cwd, sizeof(dir) - 1);
    dir[sizeof(dir)-1] = '\0';
    cfd_free(cwd);

    char head_path[CFD_MAX_PATH];
    char branch[256] = {0};
    int found = 0;

    for (int depth = 0; depth < 32 && !found; depth++) {
        snprintf(head_path, sizeof(head_path), "%s/.git/HEAD", dir);
        FILE *f = fopen(head_path, "r");
        if (f) {
            char line[256] = {0};
            if (fgets(line, sizeof(line), f)) {
                /* "ref: refs/heads/main\n" */
                if (strncmp(line, "ref: refs/heads/", 16) == 0) {
                    strncpy(branch, line + 16, sizeof(branch) - 1);
                } else {
                    /* detached HEAD — show short SHA */
                    strncpy(branch, line, 8);
                    branch[8] = '\0';
                }
                size_t blen = strlen(branch);
                while (blen > 0 && (branch[blen-1] == '\n' || branch[blen-1] == '\r'))
                    branch[--blen] = '\0';
                found = 1;
            }
            fclose(f);
        }
        if (!found) {
            /* go up one directory */
            char *sep = NULL;
            /* find last separator */
            for (char *p = dir + strlen(dir) - 1; p >= dir; p--) {
                if (*p == '/' || *p == '\\') { sep = p; break; }
            }
            if (!sep || sep == dir) break;
            *sep = '\0';
        }
    }

    if (!found || !branch[0]) return cfd_strdup("");
    return cfd_sprintf("%s(%s)%s ", cfd_color_fg(CFD_COLOR_BRIGHT_MAGENTA),
                       branch, cfd_color_reset());
}

/* \$ — '>' colored green/red based on last exit, '#' if admin */
static char *prompt_segment_dollar(cfd_session_t *sess) {
    int last_exit = sess ? sess->last_exit : 0;
#ifdef _WIN32
    /* Check if running as administrator */
    BOOL is_admin = FALSE;
    PSID admin_group = NULL;
    SID_IDENTIFIER_AUTHORITY nt_authority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&nt_authority, 2,
            SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0, &admin_group)) {
        CheckTokenMembership(NULL, admin_group, &is_admin);
        FreeSid(admin_group);
    }
    const char *sym = is_admin ? "#" : ">";
#else
    const char *sym = (getuid() == 0) ? "#" : ">";
#endif
    if (last_exit == 0)
        return cfd_sprintf("\033[32m%s\033[0m", sym);
    else
        return cfd_sprintf("\033[31m%s\033[0m", sym);
}

/* \e — exit code display */
static char *prompt_segment_exitcode(cfd_session_t *sess) {
    if (!sess || sess->last_exit == 0) return cfd_strdup("");
    return cfd_sprintf("\033[31m[%d]\033[0m ", sess->last_exit);
}

/* \t — current time HH:MM:SS */
static char *prompt_segment_time(void) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char buf[16];
    strftime(buf, sizeof(buf), "%H:%M:%S", tm_info);
    return cfd_sprintf("\033[90m%s\033[0m", buf);
}

/* \j — number of background jobs */
static char *prompt_segment_jobs(cfd_session_t *sess) {
    int count = sess ? sess->job_count : 0;
    if (count == 0) return cfd_strdup("");
    return cfd_sprintf("\033[33m[%d jobs]\033[0m ", count);
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
                case '$': seg = prompt_segment_dollar(sess); break;
                case 'e': seg = prompt_segment_exitcode(sess); break;
                case 't': seg = prompt_segment_time(); break;
                case 'j': seg = prompt_segment_jobs(sess); break;
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
