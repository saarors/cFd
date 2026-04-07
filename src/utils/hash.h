#ifndef CFD_HASH_H
#define CFD_HASH_H

#include <stddef.h>
#include <stdbool.h>

typedef void (*cfd_hash_free_fn)(void *val);

typedef struct cfd_hash_entry {
    char               *key;
    void               *val;
    struct cfd_hash_entry *next;
} cfd_hash_entry_t;

typedef struct cfd_hash {
    cfd_hash_entry_t **buckets;
    size_t             capacity;
    size_t             size;
    cfd_hash_free_fn   free_fn;
} cfd_hash_t;

cfd_hash_t *cfd_hash_new(size_t capacity, cfd_hash_free_fn free_fn);
void        cfd_hash_free(cfd_hash_t *h);

void        cfd_hash_set(cfd_hash_t *h, const char *key, void *val);
void       *cfd_hash_get(cfd_hash_t *h, const char *key);
bool        cfd_hash_has(cfd_hash_t *h, const char *key);
bool        cfd_hash_del(cfd_hash_t *h, const char *key);
void        cfd_hash_clear(cfd_hash_t *h);

size_t      cfd_hash_size(const cfd_hash_t *h);
char      **cfd_hash_keys(cfd_hash_t *h, size_t *count);

void        cfd_hash_foreach(cfd_hash_t *h,
                             void (*fn)(const char *key, void *val, void *ctx),
                             void *ctx);

#endif /* CFD_HASH_H */
