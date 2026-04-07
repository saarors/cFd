#include "cmd_rmdir.h"
#include <stdio.h>
#ifdef _WIN32
#  include <direct.h>
#  define rmdir(p) _rmdir(p)
#else
#  include <unistd.h>
#endif
int cmd_rmdir(cfd_session_t *sess, int argc, char **argv) {
    (void)sess; int ret = 0;
    for (int i = 1; i < argc; i++)
        if (rmdir(argv[i]) != 0) { perror(argv[i]); ret = 1; }
    return ret;
}
const cfd_command_t builtin_rmdir = {
    "rmdir","rmdir <dir...>","Remove empty directories","filesystem",cmd_rmdir,1,-1
};
