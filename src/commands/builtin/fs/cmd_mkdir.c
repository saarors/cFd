#include "cmd_mkdir.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#  include <direct.h>
#  define mkdir(p,m) _mkdir(p)
#endif

static int mkdirp(const char *path) {
    char tmp[4096]; int len;
    strncpy(tmp, path, sizeof(tmp)-1);
    len = (int)strlen(tmp);
    if (tmp[len-1] == '/' || tmp[len-1] == '\\') tmp[--len] = '\0';
    for (char *p = tmp+1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            *p = '\0';
#ifdef _WIN32
            _mkdir(tmp);
#else
            mkdir(tmp, 0755);
#endif
            *p = '/';
        }
    }
#ifdef _WIN32
    return _mkdir(tmp);
#else
    return mkdir(tmp, 0755);
#endif
}

int cmd_mkdir(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;
    bool parents = false;
    int ret = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) { parents = true; continue; }
        int r;
        if (parents) r = mkdirp(argv[i]);
        else {
#ifdef _WIN32
            r = _mkdir(argv[i]);
#else
            r = mkdir(argv[i], 0755);
#endif
        }
        if (r != 0) { perror(argv[i]); ret = 1; }
    }
    return ret;
}

const cfd_command_t builtin_mkdir = {
    "mkdir","mkdir [-p] <dir...>","Create directories","filesystem",cmd_mkdir,1,-1
};
