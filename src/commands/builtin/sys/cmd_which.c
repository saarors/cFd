#include "cmd_which.h"
#include "../../../platform/platform.h"
#include "../../../utils/mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#  include <windows.h>
#  define PATH_SEP ";"
#  define EXEC_EXTS_COUNT 4
static const char *exec_exts[EXEC_EXTS_COUNT] = {".exe", ".cmd", ".bat", ".com"};
#else
#  include <sys/stat.h>
#  include <unistd.h>
#  define PATH_SEP ":"
#endif

/* Built-in command names */
static const char *builtins[] = {
    "ls","cd","pwd","mkdir","rmdir","rm","cp","mv","cat","touch","stat","find",
    "echo","grep","wc","sort","head","tail","cut","tr",
    "env","set","unset","date","clear","exit","alias","unalias","version","help",
    "exec","ps","kill","jobs","bg","fg",
    "curl","wget","ping","netstat","ipconfig",
    "nano","pkg","md5","sha256",
    "calc","seq","expr",
    "which","type","history","source","export","read","sleep","uname","whoami",
    "hostname","uptime","watch","test",
    "tee","uniq","diff","base64","xargs","column","fold",
    "du","df","ln","realpath","tree","chmod",
    "wait",
    NULL
};

static bool is_builtin(const char *name) {
    for (int i = 0; builtins[i]; i++) {
        if (strcmp(builtins[i], name) == 0) return true;
    }
    return false;
}

#ifdef _WIN32
static bool file_is_exec(const char *path) {
    DWORD attr = GetFileAttributesA(path);
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

static bool find_in_path(const char *name, char *found, size_t fsz) {
    char *path_env = cfd_platform_getenv("PATH");
    if (!path_env) return false;

    char *path_copy = cfd_strdup(path_env);
    cfd_free(path_env);

    char *dir = strtok(path_copy, PATH_SEP);
    while (dir) {
        for (int e = 0; e < EXEC_EXTS_COUNT; e++) {
            char candidate[1024];
            snprintf(candidate, sizeof(candidate), "%s\\%s%s", dir, name, exec_exts[e]);
            if (file_is_exec(candidate)) {
                strncpy(found, candidate, fsz - 1);
                found[fsz - 1] = '\0';
                cfd_free(path_copy);
                return true;
            }
        }
        /* Try with no extension too */
        char candidate[1024];
        snprintf(candidate, sizeof(candidate), "%s\\%s", dir, name);
        if (file_is_exec(candidate)) {
            strncpy(found, candidate, fsz - 1);
            found[fsz - 1] = '\0';
            cfd_free(path_copy);
            return true;
        }
        dir = strtok(NULL, PATH_SEP);
    }
    cfd_free(path_copy);
    return false;
}
#else
static bool file_is_exec(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0 && (st.st_mode & S_IXUSR) && S_ISREG(st.st_mode));
}

static bool find_in_path(const char *name, char *found, size_t fsz) {
    char *path_env = cfd_platform_getenv("PATH");
    if (!path_env) return false;

    char *path_copy = cfd_strdup(path_env);
    cfd_free(path_env);

    char *dir = strtok(path_copy, PATH_SEP);
    while (dir) {
        char candidate[1024];
        snprintf(candidate, sizeof(candidate), "%s/%s", dir, name);
        if (file_is_exec(candidate)) {
            strncpy(found, candidate, fsz - 1);
            found[fsz - 1] = '\0';
            cfd_free(path_copy);
            return true;
        }
        dir = strtok(NULL, PATH_SEP);
    }
    cfd_free(path_copy);
    return false;
}
#endif

static int which_one(const char *name, bool all) {
    bool found_any = false;

    /* Check builtins first */
    if (is_builtin(name)) {
        printf("%s: shell builtin\n", name);
        found_any = true;
        if (!all) return 0;
    }

    /* Search PATH */
    if (!all) {
        char found[1024];
        if (find_in_path(name, found, sizeof(found))) {
            printf("%s\n", found);
            return 0;
        }
        if (!found_any) {
            fprintf(stderr, "which: %s: not found\n", name);
            return 1;
        }
        return 0;
    }

    /* -a: find all occurrences */
    char *path_env = cfd_platform_getenv("PATH");
    if (!path_env) return found_any ? 0 : 1;
    char *path_copy = cfd_strdup(path_env);
    cfd_free(path_env);

    char *dir = strtok(path_copy, PATH_SEP);
    while (dir) {
#ifdef _WIN32
        for (int e = 0; e < EXEC_EXTS_COUNT; e++) {
            char candidate[1024];
            snprintf(candidate, sizeof(candidate), "%s\\%s%s", dir, name, exec_exts[e]);
            if (file_is_exec(candidate)) {
                printf("%s\n", candidate);
                found_any = true;
            }
        }
#else
        char candidate[1024];
        snprintf(candidate, sizeof(candidate), "%s/%s", dir, name);
        if (file_is_exec(candidate)) {
            printf("%s\n", candidate);
            found_any = true;
        }
#endif
        dir = strtok(NULL, PATH_SEP);
    }
    cfd_free(path_copy);

    if (!found_any) {
        fprintf(stderr, "which: %s: not found\n", name);
        return 1;
    }
    return 0;
}

int cmd_which(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    if (argc < 2) {
        fprintf(stderr, "Usage: which [-a] <command...>\n");
        return 1;
    }

    bool all = false;
    int ret = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0) {
            all = true;
        } else {
            ret |= which_one(argv[i], all);
        }
    }
    return ret;
}

const cfd_command_t builtin_which = {
    "which",
    "which [-a] <command...>",
    "Locate a command in PATH",
    "system",
    cmd_which,
    1, -1
};
