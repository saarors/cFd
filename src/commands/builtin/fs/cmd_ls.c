#include "cmd_ls.h"
#include "../../../ui/color.h"
#include "../../../ui/theme.h"
#include "../../../utils/mem.h"
#include "../../../utils/str_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

#ifdef _WIN32
#  include <windows.h>
#  include <io.h>
#else
#  include <dirent.h>
#  include <unistd.h>
#  include <pwd.h>
#  include <grp.h>
#endif

typedef struct {
    char   name[512];
    int    is_dir;
    int    is_exec;
    long   size;
    time_t mtime;
} entry_t;

static int entry_cmp(const void *a, const void *b) {
    const entry_t *ea = (const entry_t *)a;
    const entry_t *eb = (const entry_t *)b;
    /* Dirs first, then alphabetical */
    if (ea->is_dir != eb->is_dir) return eb->is_dir - ea->is_dir;
    return strcmp(ea->name, eb->name);
}

static void print_entry(const entry_t *e, bool long_fmt, bool color) {
    const cfd_theme_t *t = cfd_theme_get();
    if (long_fmt) {
        char timebuf[32];
        struct tm *tm = localtime(&e->mtime);
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M", tm);
        if (color) {
            if (e->is_dir)       printf("%s", cfd_color_fg(t->dir_color));
            else if (e->is_exec) printf("%s", cfd_color_fg(t->exec_color));
            else                 printf("%s", cfd_color_fg(t->file_color));
        }
        printf("%-12ld  %s  %s%s\n", e->size, timebuf, e->name,
               e->is_dir ? "/" : "");
        if (color) printf("%s", cfd_color_reset());
    } else {
        if (color) {
            if (e->is_dir)       printf("%s", cfd_color_fg(t->dir_color));
            else if (e->is_exec) printf("%s", cfd_color_fg(t->exec_color));
            else                 printf("%s", cfd_color_fg(t->file_color));
        }
        printf("%s%s", e->name, e->is_dir ? "/" : "");
        if (color) printf("%s", cfd_color_reset());
        printf("  ");
    }
}

int cmd_ls(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;
    bool long_fmt = false, all = false;
    const char *dir = ".";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) long_fmt = true;
        else if (strcmp(argv[i], "-a") == 0) all = true;
        else if (strcmp(argv[i], "-la") == 0 || strcmp(argv[i], "-al") == 0)
            { long_fmt = true; all = true; }
        else if (argv[i][0] != '-') dir = argv[i];
    }

    entry_t *entries = (entry_t *)malloc(4096 * sizeof(entry_t));
    if (!entries) return 1;
    int count = 0;

#ifdef _WIN32
    char pattern[512];
    snprintf(pattern, sizeof(pattern), "%s\\*", dir);
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) { perror(dir); free(entries); return 1; }
    do {
        if (!all && fd.cFileName[0] == '.') continue;
        if (count >= 4096) break;
        entry_t *e = &entries[count++];
        strncpy(e->name, fd.cFileName, sizeof(e->name)-1);
        e->is_dir  = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
        e->is_exec = 0;
        ULARGE_INTEGER sz;
        sz.LowPart  = fd.nFileSizeLow;
        sz.HighPart = fd.nFileSizeHigh;
        e->size = (long)sz.QuadPart;
        ULARGE_INTEGER ft;
        ft.LowPart  = fd.ftLastWriteTime.dwLowDateTime;
        ft.HighPart = fd.ftLastWriteTime.dwHighDateTime;
        e->mtime = (time_t)((ft.QuadPart - 116444736000000000ULL) / 10000000ULL);
    } while (FindNextFileA(h, &fd));
    FindClose(h);
#else
    DIR *d = opendir(dir);
    if (!d) { perror(dir); free(entries); return 1; }
    struct dirent *ent;
    while ((ent = readdir(d)) && count < 4096) {
        if (!all && ent->d_name[0] == '.') continue;
        entry_t *e = &entries[count++];
        strncpy(e->name, ent->d_name, sizeof(e->name)-1);
        char full[1024];
        snprintf(full, sizeof(full), "%s/%s", dir, ent->d_name);
        struct stat st;
        if (stat(full, &st) == 0) {
            e->is_dir  = S_ISDIR(st.st_mode)  ? 1 : 0;
            e->is_exec = (st.st_mode & S_IXUSR) ? 1 : 0;
            e->size    = (long)st.st_size;
            e->mtime   = st.st_mtime;
        }
    }
    closedir(d);
#endif

    qsort(entries, count, sizeof(entry_t), entry_cmp);
    bool color = cfd_color_enabled();

    for (int i = 0; i < count; i++) {
        print_entry(&entries[i], long_fmt, color);
        if (!long_fmt && (i + 1) % 5 == 0) printf("\n");
    }
    if (!long_fmt && count % 5 != 0) printf("\n");
    free(entries);
    return 0;
}

const cfd_command_t builtin_ls = {
    "ls", "ls [-la] [path]", "List directory contents", "filesystem",
    cmd_ls, 0, -1
};
