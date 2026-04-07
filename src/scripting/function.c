#include "function.h"
#include "../utils/mem.h"
#include "../core/session.h"
#include <string.h>

static void func_free(void *p) {
    if (!p) return;
    cfd_function_t *fn = (cfd_function_t *)p;
    cfd_free(fn->name);
    cfd_ast_free(fn->body);
    cfd_free(fn);
}

cfd_func_store_t *cfd_func_store_new(void) {
    cfd_func_store_t *fs = CFD_NEW(cfd_func_store_t);
    fs->funcs = cfd_hash_new(32, func_free);
    return fs;
}

void cfd_func_store_free(cfd_func_store_t *fs) {
    if (!fs) return;
    cfd_hash_free(fs->funcs);
    cfd_free(fs);
}

void cfd_func_define(cfd_func_store_t *fs, const char *name, cfd_ast_node_t *body) {
    /* Check if overwriting */
    cfd_function_t *existing = (cfd_function_t *)cfd_hash_get(fs->funcs, name);
    if (existing) {
        cfd_ast_free(existing->body);
        existing->body = body;
        return;
    }
    cfd_function_t *fn = CFD_NEW(cfd_function_t);
    fn->name = cfd_strdup(name);
    fn->body = body;
    cfd_hash_set(fs->funcs, name, fn);
}

cfd_function_t *cfd_func_get(cfd_func_store_t *fs, const char *name) {
    return (cfd_function_t *)cfd_hash_get(fs->funcs, name);
}

bool cfd_func_undefine(cfd_func_store_t *fs, const char *name) {
    return cfd_hash_del(fs->funcs, name);
}

int cfd_func_call(cfd_session_t *sess, cfd_function_t *fn, int argc, char **argv) {
    if (!sess || !fn || !fn->body) return 1;
    /* Set positional parameters $1, $2, ... */
    char num[16];
    for (int i = 1; i < argc; i++) {
        snprintf(num, sizeof(num), "%d", i);
        cfd_session_set_var(sess, num, argv[i]);
    }
    snprintf(num, sizeof(num), "%d", argc - 1);
    cfd_session_set_var(sess, "#", num);
    extern int cfd_session_exec_node(cfd_session_t *sess, cfd_ast_node_t *node);
    return cfd_session_exec_node(sess, fn->body);
}
