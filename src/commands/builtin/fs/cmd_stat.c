#include "cmd_stat.h"
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
int cmd_stat(cfd_session_t *sess, int argc, char **argv) {
    (void)sess; int ret = 0;
    for (int i = 1; i < argc; i++) {
        struct stat st;
        if (stat(argv[i], &st) != 0) { perror(argv[i]); ret=1; continue; }
        char tbuf[64];
        strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", localtime(&st.st_mtime));
        printf("  File: %s\n", argv[i]);
        printf("  Size: %lld\n", (long long)st.st_size);
#ifndef _WIN32
        printf(" Inode: %lu\n", (unsigned long)st.st_ino);
        printf(" Perms: %o\n", (unsigned)st.st_mode & 07777);
#endif
        printf(" MTime: %s\n", tbuf);
        printf("  Type: %s\n", S_ISDIR(st.st_mode)?"directory":"file");
    }
    return ret;
}
const cfd_command_t builtin_stat = {
    "stat","stat <file...>","Display file status","filesystem",cmd_stat,1,-1
};
