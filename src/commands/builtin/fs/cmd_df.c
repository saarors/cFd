#include "cmd_df.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <sys/statvfs.h>
#endif

static void df_human(uint64_t bytes, char *buf, size_t sz) {
    const char *units[] = {"B", "K", "M", "G", "T"};
    double val = (double)bytes;
    int u = 0;
    while (val >= 1024.0 && u < 4) { val /= 1024.0; u++; }
    if (u == 0)
        snprintf(buf, sz, "%llu%s", (unsigned long long)bytes, units[u]);
    else
        snprintf(buf, sz, "%.1f%s", val, units[u]);
}

static void df_print_header(void) {
    printf("%-20s %10s %10s %10s %6s %s\n",
           "Filesystem", "Size", "Used", "Avail", "Use%", "Mounted on");
}

static void df_print_row(const char *fs, uint64_t total, uint64_t avail, bool human, const char *mount) {
    uint64_t used = (total > avail) ? (total - avail) : 0;
    double pct = total > 0 ? ((double)used / (double)total) * 100.0 : 0.0;

    if (human) {
        char sbuf[32], ubuf[32], abuf[32];
        df_human(total, sbuf, sizeof(sbuf));
        df_human(used,  ubuf, sizeof(ubuf));
        df_human(avail, abuf, sizeof(abuf));
        printf("%-20s %10s %10s %10s %5.0f%% %s\n", fs, sbuf, ubuf, abuf, pct, mount);
    } else {
        /* 1K blocks */
        printf("%-20s %10llu %10llu %10llu %5.0f%% %s\n",
               fs,
               (unsigned long long)(total / 1024),
               (unsigned long long)(used  / 1024),
               (unsigned long long)(avail / 1024),
               pct, mount);
    }
}

int cmd_df(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    bool human    = false;
    const char *path = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--human-readable") == 0)
            human = true;
        else if (argv[i][0] != '-')
            path = argv[i];
    }

    if (!path) path = ".";

    df_print_header();

#ifdef _WIN32
    /* Get full path */
    char full[MAX_PATH];
    if (!GetFullPathNameA(path, sizeof(full), full, NULL)) {
        strncpy(full, path, sizeof(full) - 1);
    }

    /* Root directory (e.g., C:\) */
    char root[4] = "C:\\";
    if (full[1] == ':') {
        root[0] = full[0];
        root[1] = ':';
        root[2] = '\\';
        root[3] = '\0';
    }

    ULARGE_INTEGER free_bytes_available, total_bytes, free_bytes_total;
    if (!GetDiskFreeSpaceExA(root, &free_bytes_available, &total_bytes, &free_bytes_total)) {
        fprintf(stderr, "df: cannot get disk info for %s\n", path);
        return 1;
    }

    uint64_t total = total_bytes.QuadPart;
    uint64_t avail = free_bytes_available.QuadPart;
    df_print_row(root, total, avail, human, root);
#else
    struct statvfs sv;
    if (statvfs(path, &sv) != 0) {
        perror("df: statvfs");
        return 1;
    }

    uint64_t total = (uint64_t)sv.f_blocks * (uint64_t)sv.f_frsize;
    uint64_t avail = (uint64_t)sv.f_bavail * (uint64_t)sv.f_frsize;
    df_print_row(path, total, avail, human, path);
#endif

    return 0;
}

const cfd_command_t builtin_df = {
    "df",
    "df [-h] [path]",
    "Report file system disk space usage",
    "fs",
    cmd_df,
    0, -1
};
