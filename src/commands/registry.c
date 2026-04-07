#include "registry.h"
#include "../utils/mem.h"
#include "../utils/str_utils.h"
#include <string.h>
#include <stdio.h>

cfd_registry_t *g_registry = NULL;

cfd_registry_t *cfd_registry_new(void) {
    cfd_registry_t *r = CFD_NEW(cfd_registry_t);
    r->commands = cfd_hash_new(128, NULL); /* commands are static, don't free */
    return r;
}

void cfd_registry_free(cfd_registry_t *reg) {
    if (!reg) return;
    cfd_hash_free(reg->commands);
    cfd_free(reg);
}

void cfd_registry_register(cfd_registry_t *reg, const cfd_command_t *cmd) {
    if (!reg || !cmd || !cmd->name) return;
    cfd_hash_set(reg->commands, cmd->name, (void *)cmd);
}

const cfd_command_t *cfd_registry_find(cfd_registry_t *reg, const char *name) {
    if (!reg || !name) return NULL;
    return (const cfd_command_t *)cfd_hash_get(reg->commands, name);
}

void cfd_registry_unregister(cfd_registry_t *reg, const char *name) {
    if (reg && name) cfd_hash_del(reg->commands, name);
}

const char **cfd_registry_list_names(int *count) {
    if (!g_registry) { if (count) *count = 0; return NULL; }
    size_t n;
    char **keys = cfd_hash_keys(g_registry->commands, &n);
    if (count) *count = (int)n;
    return (const char **)keys;
}

void cfd_registry_init(void) {
    g_registry = cfd_registry_new();
    cfd_register_all_builtins(g_registry);
}

void cfd_registry_shutdown(void) {
    cfd_registry_free(g_registry);
    g_registry = NULL;
}
