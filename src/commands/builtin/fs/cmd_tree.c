#include "cmd_tree.h"
#include "../../../utils/mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <dirent.h>
#endif

static int  g_dirs  = 0;
static int  g_files = 0;

#ifdef _WIN32
static void tree_print_win(const char *path, const char *prefix,
                           int depth, int max_depth, bool show_hidden) {
    if (max_depth >= 0 && depth > max_depth) return;

    char search[MAX_PATH];
    snprintf(search, sizeof(search), "%s\\*", path);

    WIN32_FIND_DATAA ffd;
    HANDLE hFind = FindFirstFileA(search, &ffd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    /* Collect entries */
    char **names  = (char **)cfd_malloc(4096 * sizeof(char *));
    bool *is_dirs = (bool *)cfd_calloc(4096, sizeof(bool));
    int   count   = 0;

    do {
        if (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0)
            continue;
        if (!show_hidden && ffd.cFileName[0] == '.') continue;
        if (!show_hidden && (ffd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) continue;
        if (count < 4096) {
            names[count]    = cfd_strdup(ffd.cFileName);
            is_dirs[count]  = !!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
            count++;
        }
    } while (FindNextFileA(hFind, &ffd));
    FindClose(hFind);

    for (int i = 0; i < count; i++) {
        bool last = (i == count - 1);
        const char *branch = last ? "\xe2\x94\x94\xe2\x94\x80\xe2\x94\x80 " : "\xe2\x94\x9c\xe2\x94\x80\xe2\x94\x80 ";
        printf("%s%s%s\n", prefix, branch, names[i]);

        if (is_dirs[i]) {
            g_dirs++;
            char new_path[MAX_PATH];
            snprintf(new_path, sizeof(new_path), "%s\\%s", path, names[i]);
            char new_prefix[1024];
            snprintf(new_prefix, sizeof(new_prefix), "%s%s",
                     prefix, last ? "    " : "\xe2\x94\x82   ");
            tree_print_win(new_path, new_prefix, depth + 1, max_depth, show_hidden);
        } else {
            g_files++;
        }
        cfd_free(names[i]);
    }
    cfd_free(names);
    cfd_free(is_dirs);
}

#else

static int cmp_str(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

static void tree_print_unix(const char *path, const char *prefix,
                            int depth, int max_depth, bool show_hidden) {
    if (max_depth >= 0 && depth > max_depth) return;

    DIR *dp = opendir(path);
    if (!dp) return;

    char **names  = (char **)cfd_malloc(4096 * sizeof(char *));
    bool *is_dirs = (bool *)cfd_calloc(4096, sizeof(bool));
    int   count   = 0;

    struct dirent *de;
    while ((de = readdir(dp))) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;
        if (!show_hidden && de->d_name[0] == '.') continue;
        if (count < 4096) {
            names[count] = cfd_strdup(de->d_name);

            char child[4096];
            snprintf(child, sizeof(child), "%s/%s", path, de->d_name);
            struct stat st;
            is_dirs[count] = (stat(child, &st) == 0 && S_ISDIR(st.st_mode));
            count++;
        }
    }
    closedir(dp);

    qsort(names, (size_t)count, sizeof(char *), cmp_str);

    for (int i = 0; i < count; i++) {
        bool last = (i == count - 1);
        const char *branch = last ? "\xe2\x94\x94\xe2\x94\x80\xe2\x94\x80 " : "\xe2\x94\x9c\xe2\x94\x80\xe2\x94\x80 ";
        printf("%s%s%s\n", prefix, branch, names[i]);

        if (is_dirs[i]) {
            g_dirs++;
            char new_path[4096];
            snprintf(new_path, sizeof(new_path), "%s/%s", path, names[i]);
            char new_prefix[1024];
            snprintf(new_prefix, sizeof(new_prefix), "%s%s",
                     prefix, last ? "    " : "\xe2\x94\x82   ");
            tree_print_unix(new_path, new_prefix, depth + 1, max_depth, show_hidden);
        } else {
            g_files++;
        }
        cfd_free(names[i]);
    }
    cfd_free(names);
    cfd_free(is_dirs);
}
#endif

int cmd_tree(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    int         max_depth   = -1; /* -1 = unlimited */
    bool        show_hidden = false;
    const char *path        = ".";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-L") == 0 && i + 1 < argc) {
            max_depth = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-a") == 0) {
            show_hidden = true;
        } else if (argv[i][0] != '-') {
            path = argv[i];
        }
    }

    g_dirs  = 0;
    g_files = 0;

    printf("%s\n", path);

#ifdef _WIN32
    tree_print_win(path, "", 1, max_depth, show_hidden);
#else
    tree_print_unix(path, "", 1, max_depth, show_hidden);
#endif

    printf("\n%d director%s, %d file%s\n",
           g_dirs,  g_dirs  == 1 ? "y"  : "ies",
           g_files, g_files == 1 ? ""   : "s");

    return 0;
}

const cfd_command_t builtin_tree = {
    "tree",
    "tree [-L level] [-a] [path]",
    "List directory contents in a tree format",
    "fs",
    cmd_tree,
    0, -1
};
