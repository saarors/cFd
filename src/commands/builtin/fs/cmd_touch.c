#include "cmd_touch.h"
#include <stdio.h>
#include <utime.h>
int cmd_touch(cfd_session_t *sess, int argc, char **argv) {
    (void)sess; int ret = 0;
    for (int i = 1; i < argc; i++) {
        FILE *f = fopen(argv[i], "ab");
        if (!f) { perror(argv[i]); ret=1; continue; }
        fclose(f);
        utime(argv[i], NULL);
    }
    return ret;
}
const cfd_command_t builtin_touch = {
    "touch","touch <file...>","Create or update file timestamps","filesystem",cmd_touch,1,-1
};
