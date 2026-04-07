#include "cmd_cp.h"
#include "../../../utils/path.h"
#include "../../../utils/mem.h"
#include <stdio.h>
#include <string.h>

static int copy_file(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) { perror(src); return 1; }
    FILE *out = fopen(dst, "wb");
    if (!out) { perror(dst); fclose(in); return 1; }
    char buf[65536]; size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
        fwrite(buf, 1, n, out);
    fclose(in); fclose(out);
    return 0;
}

int cmd_cp(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;
    if (argc < 3) { fprintf(stderr, "cp: missing operand\n"); return 1; }
    const char *dst = argv[argc-1];
    int ret = 0;
    for (int i = 1; i < argc-1; i++) {
        char *dest;
        if (cfd_path_is_dir(dst)) {
            char *base = cfd_path_basename(argv[i]);
            dest = cfd_path_join(dst, base);
            cfd_free(base);
        } else {
            dest = cfd_strdup(dst);
        }
        ret |= copy_file(argv[i], dest);
        cfd_free(dest);
    }
    return ret;
}
const cfd_command_t builtin_cp = {
    "cp","cp <src...> <dst>","Copy files","filesystem",cmd_cp,2,-1
};
