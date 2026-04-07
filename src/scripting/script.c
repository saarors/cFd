#include "script.h"
#include "../core/session.h"
#include "../parser/parser.h"
#include "../utils/mem.h"
#include "../utils/path.h"
#include "../platform/platform.h"
#include "../../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *read_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);
    if (sz <= 0) { fclose(f); return cfd_strdup(""); }
    char *buf = cfd_malloc(sz + 1);
    size_t n = fread(buf, 1, sz, f);
    buf[n] = '\0';
    fclose(f);
    return buf;
}

int cfd_script_run_string(cfd_session_t *sess, const char *code) {
    if (!code || !*code) return 0;
    cfd_ast_node_t *ast = cfd_parse(code);
    if (!ast) return 0;
    extern int cfd_session_exec_node(cfd_session_t *sess, cfd_ast_node_t *node);
    int ret = cfd_session_exec_node(sess, ast);
    cfd_ast_free(ast);
    return ret;
}

int cfd_script_run_file(cfd_session_t *sess, const char *path) {
    char *code = read_file(path);
    if (!code) return -1;
    int ret = cfd_script_run_string(sess, code);
    cfd_free(code);
    return ret;
}

int cfd_script_run_rc(cfd_session_t *sess) {
    char *home = cfd_platform_get_home();
    char *rcpath = cfd_path_join(home, CFD_RCFILE);
    cfd_free(home);
    int ret = 0;
    if (cfd_path_exists(rcpath))
        ret = cfd_script_run_file(sess, rcpath);
    cfd_free(rcpath);
    return ret;
}
