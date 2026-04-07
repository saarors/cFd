#include "cmd_mv.h"
#include "../../../utils/path.h"
#include "../../../utils/mem.h"
#include <stdio.h>
int cmd_mv(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;
    if (argc < 3) { fprintf(stderr,"mv: missing operand\n"); return 1; }
    const char *dst = argv[argc-1]; int ret = 0;
    for (int i = 1; i < argc-1; i++) {
        char *dest;
        if (cfd_path_is_dir(dst)) {
            char *base = cfd_path_basename(argv[i]);
            dest = cfd_path_join(dst, base);
            cfd_free(base);
        } else dest = cfd_strdup(dst);
        if (rename(argv[i], dest) != 0) { perror(argv[i]); ret = 1; }
        cfd_free(dest);
    }
    return ret;
}
const cfd_command_t builtin_mv = {
    "mv","mv <src...> <dst>","Move/rename files","filesystem",cmd_mv,2,-1
};
