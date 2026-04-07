#ifndef CFD_VARIABLE_H
#define CFD_VARIABLE_H

#include "../../include/types.h"
#include "../utils/hash.h"
#include <stdbool.h>

typedef struct cfd_variable {
    char       *name;
    char       *value;
    cfd_scope_t scope;
    bool        readonly;
} cfd_variable_t;

typedef struct cfd_var_store {
    cfd_hash_t *vars;   /* name -> cfd_variable_t* */
} cfd_var_store_t;

cfd_var_store_t *cfd_var_store_new(void);
void             cfd_var_store_free(cfd_var_store_t *vs);

void             cfd_var_set(cfd_var_store_t *vs, const char *name,
                             const char *value, cfd_scope_t scope);
const char      *cfd_var_get(cfd_var_store_t *vs, const char *name);
bool             cfd_var_unset(cfd_var_store_t *vs, const char *name);
bool             cfd_var_exists(cfd_var_store_t *vs, const char *name);
void             cfd_var_set_readonly(cfd_var_store_t *vs, const char *name, bool ro);

/* Variable expansion in a string: replaces $VAR and ${VAR} */
char            *cfd_var_expand(cfd_var_store_t *vs, const char *s);

/* Export variables to the process environment */
void             cfd_var_export_all(cfd_var_store_t *vs);

/* List all variable names */
char           **cfd_var_list(cfd_var_store_t *vs, int *count);

#endif /* CFD_VARIABLE_H */
