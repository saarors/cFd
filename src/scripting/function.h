#ifndef CFD_FUNCTION_H
#define CFD_FUNCTION_H

#include "../../include/types.h"
#include "../parser/ast.h"
#include "../utils/hash.h"

typedef struct cfd_function {
    char           *name;
    cfd_ast_node_t *body;
} cfd_function_t;

typedef struct cfd_func_store {
    cfd_hash_t *funcs;
} cfd_func_store_t;

cfd_func_store_t *cfd_func_store_new(void);
void              cfd_func_store_free(cfd_func_store_t *fs);

void              cfd_func_define(cfd_func_store_t *fs, const char *name, cfd_ast_node_t *body);
cfd_function_t   *cfd_func_get(cfd_func_store_t *fs, const char *name);
bool              cfd_func_undefine(cfd_func_store_t *fs, const char *name);

int               cfd_func_call(cfd_session_t *sess, cfd_function_t *fn, int argc, char **argv);

#endif /* CFD_FUNCTION_H */
