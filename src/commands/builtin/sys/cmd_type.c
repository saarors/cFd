#include "cmd_type.h"
#include "../../../platform/platform.h"
#include "../../../utils/mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#  include <windows.h>
#  define PATH_SEP ";"
static const char *type_exec_exts[] = {".exe", ".cmd", ".bat", ".com", NULL};
#else
#  include <sys/stat.h>
#  define PATH_SEP ":"
#endif

static const char *type_builtins[] = {
    "ls","cd","pwd","mkdir","rmdir","rm","cp","mv","cat","touch","stat","find",
    "echo","grep","wc","sort","head","tail","cut","tr",
    "env","set","unset","date","clear","exit","alias","unalias","version","help",
    "exec","ps","kill","jobs","bg","fg",
    "curl","wget","ping","netstat","ipconfig","nano","pkg","md5","sha256",
    "calc","seq","expr","which","type","history","source","export","read","sleep",
    "uname","whoami","hostname","uptime","watch","test",
    "tee","uniq","diff","base64","xargs","column","fold",
    "du","df","ln","realpath","tree","chmod","wait",
    NULL
};

static bool type_is_builtin(const char *name) {
    for (int i = 0; type_builtins[i]; i++)
        if (strcmp(type_builtins[i], name) == 0) return true;
    return false;
}

#ifdef _WIN32
static bool type_find_in_path(const char *name, char *out, size_t outsz) {
    char *pe = cfd_platform_getenv("PATH");
    if (!pe) return false;
    char *pc = cfd_strdup(pe);
    cfd_free(pe);
    char *dir = strtok(pc, PATH_SEP);
    while (dir) {
        for (int e = 0; type_exec_exts[e]; e++) {
            char cand[1024];
            snprintf(cand, sizeof(cand), "%s\\%s%s", dir, name, type_exec_exts[e]);
            DWORD attr = GetFileAttributesA(cand);
            if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
                strncpy(out, cand, outsz - 1);
                out[outsz - 1] = '\0';
                cfd_free(pc);
                return true;
            }
        }
        dir = strtok(NULL, PATH_SEP);
    }
    cfd_free(pc);
    return false;
}
#else
static bool type_find_in_path(const char *name, char *out, size_t outsz) {
    char *pe = cfd_platform_getenv("PATH");
    if (!pe) return false;
    char *pc = cfd_strdup(pe);
    cfd_free(pe);
    char *dir = strtok(pc, PATH_SEP);
    while (dir) {
        char cand[1024];
        snprintf(cand, sizeof(cand), "%s/%s", dir, name);
        struct stat st;
        if (stat(cand, &st) == 0 && S_ISREG(st.st_mode) && (st.st_mode & S_IXUSR)) {
            strncpy(out, cand, outsz - 1);
            out[outsz - 1] = '\0';
            cfd_free(pc);
            return true;
        }
        dir = strtok(NULL, PATH_SEP);
    }
    cfd_free(pc);
    return false;
}
#endif

int cmd_type(cfd_session_t *sess, int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: type <name...>\n");
        return 1;
    }

    int ret = 0;
    for (int i = 1; i < argc; i++) {
        const char *name = argv[i];

        /* Check aliases */
        if (sess && sess->aliases) {
            const char *alias_val = cfd_session_get_alias(sess, name);
            if (alias_val) {
                printf("%s is an alias for %s\n", name, alias_val);
                continue;
            }
        }

        /* Check builtins */
        if (type_is_builtin(name)) {
            printf("%s is a shell builtin\n", name);
            continue;
        }

        /* Check PATH */
        char path[1024];
        if (type_find_in_path(name, path, sizeof(path))) {
            printf("%s is %s\n", name, path);
            continue;
        }

        fprintf(stderr, "%s: not found\n", name);
        ret = 1;
    }
    return ret;
}

const cfd_command_t builtin_type = {
    "type",
    "type <name...>",
    "Describe how each name would be interpreted",
    "system",
    cmd_type,
    1, -1
};
