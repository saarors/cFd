#include "path.h"
#include "mem.h"
#include "str_utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#  include <windows.h>
#  include <direct.h>
#  define PATH_SEP '\\'
#  define PATH_SEP_S "\\"
#  define ALT_SEP '/'
#else
#  include <unistd.h>
#  include <sys/stat.h>
#  define PATH_SEP '/'
#  define PATH_SEP_S "/"
#  define ALT_SEP '\0'
#endif

char *cfd_path_join(const char *a, const char *b) {
    if (!a || !*a) return cfd_strdup(b ? b : "");
    if (!b || !*b) return cfd_strdup(a);
    size_t al = strlen(a);
    char last = a[al - 1];
    bool has_sep = (last == '/' || last == '\\');
    bool b_abs = (b[0] == '/' || b[0] == '\\');
    if (b_abs) return cfd_strdup(b);
    if (has_sep) return cfd_sprintf("%s%s", a, b);
    return cfd_sprintf("%s/%s", a, b);
}

char *cfd_path_dirname(const char *path) {
    if (!path) return cfd_strdup(".");
    char *copy = cfd_strdup(path);
    size_t len = strlen(copy);
    /* strip trailing separators */
    while (len > 1 && (copy[len-1] == '/' || copy[len-1] == '\\')) copy[--len] = '\0';
    char *last = NULL;
    for (char *p = copy; *p; p++)
        if (*p == '/' || *p == '\\') last = p;
    if (!last) { cfd_free(copy); return cfd_strdup("."); }
    if (last == copy) { last[1] = '\0'; return copy; }
    *last = '\0';
    return copy;
}

char *cfd_path_basename(const char *path) {
    if (!path || !*path) return cfd_strdup(".");
    const char *last = path;
    for (const char *p = path; *p; p++)
        if (*p == '/' || *p == '\\') last = p + 1;
    if (!*last) return cfd_strdup(".");
    return cfd_strdup(last);
}

char *cfd_path_ext(const char *path) {
    if (!path) return cfd_strdup("");
    const char *base = path;
    for (const char *p = path; *p; p++)
        if (*p == '/' || *p == '\\') base = p + 1;
    const char *dot = strrchr(base, '.');
    if (!dot || dot == base) return cfd_strdup("");
    return cfd_strdup(dot);
}

char *cfd_path_normalize(const char *path) {
    if (!path) return cfd_strdup(".");
    char *out = cfd_strdup(path);
    /* Replace backslashes with forward slashes */
    for (char *p = out; *p; p++) if (*p == '\\') *p = '/';
    return out;
}

char *cfd_path_expand_home(const char *path) {
    if (!path) return cfd_strdup(".");
    if (path[0] != '~') return cfd_strdup(path);
    const char *home = getenv("HOME");
#ifdef _WIN32
    if (!home) home = getenv("USERPROFILE");
#endif
    if (!home) home = ".";
    if (path[1] == '\0') return cfd_strdup(home);
    return cfd_path_join(home, path + 2);
}

char *cfd_path_absolute(const char *path) {
#ifdef _WIN32
    char buf[4096];
    if (_fullpath(buf, path, sizeof(buf))) return cfd_strdup(buf);
    return cfd_strdup(path);
#else
    char buf[4096];
    if (realpath(path, buf)) return cfd_strdup(buf);
    return cfd_strdup(path);
#endif
}

bool cfd_path_exists(const char *path) {
#ifdef _WIN32
    DWORD a = GetFileAttributesA(path);
    return a != INVALID_FILE_ATTRIBUTES;
#else
    struct stat st;
    return stat(path, &st) == 0;
#endif
}

bool cfd_path_is_dir(const char *path) {
#ifdef _WIN32
    DWORD a = GetFileAttributesA(path);
    return (a != INVALID_FILE_ATTRIBUTES) && (a & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
#endif
}

bool cfd_path_is_file(const char *path) {
#ifdef _WIN32
    DWORD a = GetFileAttributesA(path);
    return (a != INVALID_FILE_ATTRIBUTES) && !(a & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
#endif
}

bool cfd_path_is_absolute(const char *path) {
    if (!path) return false;
#ifdef _WIN32
    return (path[0] && path[1] == ':') || path[0] == '/' || path[0] == '\\';
#else
    return path[0] == '/';
#endif
}

char **cfd_path_split_dirs(const char *env_path, int *count) {
    if (!env_path) { if (count) *count = 0; return NULL; }
#ifdef _WIN32
    return cfd_strsplit(env_path, ";", count);
#else
    return cfd_strsplit(env_path, ":", count);
#endif
}
