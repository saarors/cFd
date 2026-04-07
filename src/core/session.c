#include "session.h"
#include "../parser/parser.h"
#include "../commands/dispatch.h"
#include "../platform/platform.h"
#include "../utils/mem.h"
#include "../utils/str_utils.h"

static void alias_free(void *p) { cfd_free(p); }

cfd_session_t *cfd_session_new(void) {
    cfd_session_t *sess = CFD_NEW(cfd_session_t);
    sess->cwd      = cfd_platform_getcwd();
    sess->prev_cwd = cfd_strdup(sess->cwd);
    sess->vars     = cfd_var_store_new();
    sess->funcs    = cfd_func_store_new();
    sess->aliases  = cfd_hash_new(64, alias_free);
    sess->running  = true;
    sess->pid      = cfd_platform_get_pid();

    /* Seed useful variables */
    cfd_session_set_var(sess, "CFD_VERSION", CFD_VERSION);
    cfd_session_set_var(sess, "TERM", "cFd");
    char pidbuf[16]; snprintf(pidbuf, sizeof(pidbuf), "%d", sess->pid);
    cfd_session_set_var(sess, "PID", pidbuf);

    return sess;
}

void cfd_session_free(cfd_session_t *sess) {
    if (!sess) return;
    cfd_free(sess->cwd);
    cfd_free(sess->prev_cwd);
    cfd_var_store_free(sess->vars);
    cfd_func_store_free(sess->funcs);
    cfd_hash_free(sess->aliases);
    cfd_free(sess);
}

void cfd_session_set_var(cfd_session_t *sess, const char *name, const char *val) {
    cfd_var_set(sess->vars, name, val, CFD_SCOPE_LOCAL);
}

const char *cfd_session_get_var(cfd_session_t *sess, const char *name) {
    return cfd_var_get(sess->vars, name);
}

const char *cfd_session_get_alias(cfd_session_t *sess, const char *name) {
    return (const char *)cfd_hash_get(sess->aliases, name);
}

int cfd_session_exec_node(cfd_session_t *sess, cfd_ast_node_t *node) {
    return cfd_dispatch(sess, node);
}

int cfd_session_exec_string(cfd_session_t *sess, const char *line) {
    if (!line || !*line) return 0;
    cfd_ast_node_t *ast = cfd_parse(line);
    if (!ast) return 0;
    int ret = cfd_session_exec_node(sess, ast);
    cfd_ast_free(ast);
    return ret;
}
