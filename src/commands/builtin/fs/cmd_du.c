#include "cmd_du.h"
#include "../../../utils/mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <sys/stat.h>
#  include <dirent.h>
#  include <sys/types.h>
#endif

static char *du_human(uint64_t bytes, char *buf, size_t sz) {
    const char *units[] = {"B", "K", "M", "G", "T"};
    double val = (double)bytes;
    int u = 0;
    while (val >= 1024.0 && u < 4) { val /= 1024.0; u++; }
    if (u == 0)
        snprintf(buf, sz, "%llu%s", (unsigned long long)bytes, units[u]);
    else
        snprintf(buf, sz, "%.1f%s", val, units[u]);
    return buf;
}

#ifdef _WIN32
static uint64_t du_dir_win(const char *path, bool summary, bool human) {
    WIN32_FIND_DATAA ffd;
    char search[MAX_PATH];
    snprintf(search, sizeof(search), "%s\\*", path);

    HANDLE hFind = FindFirstFileA(search, &ffd);
    if (hFind == INVALID_HANDLE_VALUE) return 0;

    uint64_t total = 0;
    do {
        if (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0)
            continue;

        char child[MAX_PATH];
        snprintf(child, sizeof(child), "%s\\%s", path, ffd.cFileName);

        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            uint64_t sub = du_dir_win(child, false, human);
            if (!summary) {
                char hbuf[32];
                if (human) printf("%s\t%s\n", du_human(sub, hbuf, sizeof(hbuf)), child);
                else printf("%llu\t%s\n", (unsigned long long)(sub / 1024), child);
            }
            total += sub;
        } else {
            uint64_t fsz = ((uint64_t)ffd.nFileSizeHigh << 32) | ffd.nFileSizeLow;
            total += fsz;
        }
    } while (FindNextFileA(hFind, &ffd));
    FindClose(hFind);
    return total;
}
#else
static uint64_t du_dir_unix(const char *path, bool summary, bool human) {
    DIR *dp = opendir(path);
    if (!dp) {
        struct stat st;
        if (stat(path, &st) == 0) return (uint64_t)st.st_size;
        return 0;
    }

    uint64_t total = 0;
    struct dirent *de;
    while ((de = readdir(dp))) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;

        char child[4096];
        snprintf(child, sizeof(child), "%s/%s", path, de->d_name);

        struct stat st;
        if (lstat(child, &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            uint64_t sub = du_dir_unix(child, false, human);
            if (!summary) {
                char hbuf[32];
                if (human) printf("%s\t%s\n", du_human(sub, hbuf, sizeof(hbuf)), child);
                else printf("%llu\t%s\n", (unsigned long long)(sub / 1024), child);
            }
            total += sub;
        } else {
            total += (uint64_t)st.st_size;
        }
    }
    closedir(dp);
    return total;
}
#endif

int cmd_du(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    bool summary = false;
    bool human   = false;
    bool any_path = false;

    int ret = 0;

    /* Collect flags and paths */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--summarize") == 0)
            summary = true;
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--human-readable") == 0)
            human = true;
        else if (argv[i][0] == '-') {
            /* Check combined -sh */
            for (int j = 1; argv[i][j]; j++) {
                if (argv[i][j] == 's') summary = true;
                else if (argv[i][j] == 'h') human = true;
            }
        }
    }

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') continue;
        any_path = true;
        const char *path = argv[i];
        uint64_t total;

#ifdef _WIN32
        DWORD attr = GetFileAttributesA(path);
        if (attr == INVALID_FILE_ATTRIBUTES) {
            fprintf(stderr, "du: %s: No such file or directory\n", path);
            ret = 1;
            continue;
        }
        if (attr & FILE_ATTRIBUTE_DIRECTORY)
            total = du_dir_win(path, summary, human);
        else {
            WIN32_FILE_ATTRIBUTE_DATA fad;
            GetFileAttributesExA(path, GetFileExInfoStandard, &fad);
            total = ((uint64_t)fad.nFileSizeHigh << 32) | fad.nFileSizeLow;
        }
#else
        struct stat st;
        if (lstat(path, &st) != 0) {
            fprintf(stderr, "du: %s: No such file or directory\n", path);
            ret = 1;
            continue;
        }
        if (S_ISDIR(st.st_mode))
            total = du_dir_unix(path, summary, human);
        else
            total = (uint64_t)st.st_size;
#endif

        char hbuf[32];
        if (human)
            printf("%s\t%s\n", du_human(total, hbuf, sizeof(hbuf)), path);
        else
            printf("%llu\t%s\n", (unsigned long long)(total / 1024), path);
    }

    if (!any_path) {
        /* Default: current directory */
        uint64_t total;
#ifdef _WIN32
        total = du_dir_win(".", summary, human);
#else
        total = du_dir_unix(".", summary, human);
#endif
        char hbuf[32];
        if (human)
            printf("%s\t.\n", du_human(total, hbuf, sizeof(hbuf)));
        else
            printf("%llu\t.\n", (unsigned long long)(total / 1024));
    }

    return ret;
}

const cfd_command_t builtin_du = {
    "du",
    "du [-s] [-h] [path...]",
    "Estimate file and directory space usage",
    "fs",
    cmd_du,
    0, -1
};
