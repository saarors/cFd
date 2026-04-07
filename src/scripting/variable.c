#include "variable.h"
#include "../utils/mem.h"
#include "../utils/str_utils.h"
#include "../platform/platform.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static void var_free(void *p) {
    if (!p) return;
    cfd_variable_t *v = (cfd_variable_t *)p;
    cfd_free(v->name);
    cfd_free(v->value);
    cfd_free(v);
}

cfd_var_store_t *cfd_var_store_new(void) {
    cfd_var_store_t *vs = CFD_NEW(cfd_var_store_t);
    vs->vars = cfd_hash_new(64, var_free);
    return vs;
}

void cfd_var_store_free(cfd_var_store_t *vs) {
    if (!vs) return;
    cfd_hash_free(vs->vars);
    cfd_free(vs);
}

void cfd_var_set(cfd_var_store_t *vs, const char *name, const char *value, cfd_scope_t scope) {
    cfd_variable_t *v = (cfd_variable_t *)cfd_hash_get(vs->vars, name);
    if (v) {
        if (v->readonly) return;
        cfd_free(v->value);
        v->value = cfd_strdup(value ? value : "");
        v->scope = scope;
    } else {
        v = CFD_NEW(cfd_variable_t);
        v->name  = cfd_strdup(name);
        v->value = cfd_strdup(value ? value : "");
        v->scope = scope;
        cfd_hash_set(vs->vars, name, v);
    }
}

const char *cfd_var_get(cfd_var_store_t *vs, const char *name) {
    cfd_variable_t *v = (cfd_variable_t *)cfd_hash_get(vs->vars, name);
    if (v) return v->value;
    /* fall back to env */
    return getenv(name);
}

bool cfd_var_unset(cfd_var_store_t *vs, const char *name) {
    return cfd_hash_del(vs->vars, name);
}

bool cfd_var_exists(cfd_var_store_t *vs, const char *name) {
    return cfd_hash_has(vs->vars, name) || getenv(name) != NULL;
}

void cfd_var_set_readonly(cfd_var_store_t *vs, const char *name, bool ro) {
    cfd_variable_t *v = (cfd_variable_t *)cfd_hash_get(vs->vars, name);
    if (v) v->readonly = ro;
}

char *cfd_var_expand(cfd_var_store_t *vs, const char *s) {
    if (!s) return cfd_strdup("");
    char *out = cfd_malloc(strlen(s) * 4 + 256);
    size_t oi = 0;
    size_t cap = strlen(s) * 4 + 256;

    for (const char *p = s; *p;) {
        if (*p == '$') {
            p++;
            char name[256];
            int ni = 0;
            if (*p == '{') {
                p++;
                while (*p && *p != '}') name[ni++] = *p++;
                if (*p == '}') p++;
            } else {
                while (*p && ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
                              (*p >= '0' && *p <= '9') || *p == '_'))
                    name[ni++] = *p++;
            }
            name[ni] = '\0';
            if (ni > 0) {
                const char *val = cfd_var_get(vs, name);
                if (!val) val = "";
                size_t vlen = strlen(val);
                while (oi + vlen + 1 >= cap) { cap *= 2; out = cfd_realloc(out, cap); }
                memcpy(out + oi, val, vlen);
                oi += vlen;
            } else {
                out[oi++] = '$';
            }
        } else {
            if (oi + 2 >= cap) { cap *= 2; out = cfd_realloc(out, cap); }
            out[oi++] = *p++;
        }
    }
    out[oi] = '\0';
    return out;
}

static void export_one(const char *key, void *val, void *ctx) {
    (void)ctx;
    cfd_variable_t *v = (cfd_variable_t *)val;
    if (v->scope == CFD_SCOPE_EXPORT)
        cfd_platform_setenv(key, v->value);
}

void cfd_var_export_all(cfd_var_store_t *vs) {
    cfd_hash_foreach(vs->vars, export_one, NULL);
}

char **cfd_var_list(cfd_var_store_t *vs, int *count) {
    size_t n;
    char **keys = cfd_hash_keys(vs->vars, &n);
    if (count) *count = (int)n;
    return keys;
}
