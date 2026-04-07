#ifndef CFD_REGISTRY_H
#define CFD_REGISTRY_H

#include "command.h"
#include "../utils/hash.h"

typedef struct cfd_registry {
    cfd_hash_t *commands;
} cfd_registry_t;

cfd_registry_t   *cfd_registry_new(void);
void              cfd_registry_free(cfd_registry_t *reg);

void              cfd_registry_register(cfd_registry_t *reg, const cfd_command_t *cmd);
const cfd_command_t *cfd_registry_find(cfd_registry_t *reg, const char *name);
void              cfd_registry_unregister(cfd_registry_t *reg, const char *name);

/* List all command names. Caller frees returned array (NOT the strings). */
const char      **cfd_registry_list_names(int *count);

/* Global registry instance */
extern cfd_registry_t *g_registry;

void cfd_registry_init(void);
void cfd_registry_shutdown(void);

/* Register all built-in commands */
void cfd_register_all_builtins(cfd_registry_t *reg);

#endif /* CFD_REGISTRY_H */
