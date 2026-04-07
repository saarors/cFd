#include "cmd_cd.h"
#include "../../../platform/platform.h"
#include "../../../utils/mem.h"
#include "../../../utils/path.h"
#include <stdio.h>
#include <string.h>

int cmd_cd(cfd_session_t *sess, int argc, char **argv) {
    const char *dest;
    if (argc < 2) {
        dest = cfd_platform_get_home();
    } else if (strcmp(argv[1], "-") == 0) {
        dest = sess->prev_cwd ? sess->prev_cwd : sess->cwd;
    } else {
        dest = argv[1];
    }

    char *expanded = cfd_path_expand_home(dest);
    if (cfd_platform_chdir(expanded) != 0) {
        perror(expanded);
        cfd_free(expanded);
        return 1;
    }

    /* Update session cwd */
    char *newcwd = cfd_platform_getcwd();
    if (sess->prev_cwd) cfd_free(sess->prev_cwd);
    sess->prev_cwd = cfd_strdup(sess->cwd);
    if (sess->cwd) cfd_free(sess->cwd);
    sess->cwd = newcwd;

    cfd_free(expanded);
    return 0;
}

const cfd_command_t builtin_cd = {
    "cd", "cd [dir]", "Change working directory", "filesystem",
    cmd_cd, 0, 1
};
