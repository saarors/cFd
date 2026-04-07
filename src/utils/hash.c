#include "hash.h"
#include "mem.h"
#include <string.h>
#include <stdlib.h>

static size_t hash_key(const char *key, size_t cap) {
    size_t h = 5381;
    while (*key) h = ((h << 5) + h) ^ (unsigned char)*key++;
    return h % cap;
}

cfd_hash_t *cfd_hash_new(size_t capacity, cfd_hash_free_fn free_fn) {
    cfd_hash_t *h = CFD_NEW(cfd_hash_t);
    h->capacity = capacity ? capacity : 64;
    h->buckets  = cfd_calloc(h->capacity, sizeof(cfd_hash_entry_t *));
    h->free_fn  = free_fn;
    return h;
}

void cfd_hash_free(cfd_hash_t *h) {
    if (!h) return;
    cfd_hash_clear(h);
    cfd_free(h->buckets);
    cfd_free(h);
}

void cfd_hash_set(cfd_hash_t *h, const char *key, void *val) {
    size_t idx = hash_key(key, h->capacity);
    for (cfd_hash_entry_t *e = h->buckets[idx]; e; e = e->next) {
        if (strcmp(e->key, key) == 0) {
            if (h->free_fn && e->val) h->free_fn(e->val);
            e->val = val;
            return;
        }
    }
    cfd_hash_entry_t *e = CFD_NEW(cfd_hash_entry_t);
    e->key = cfd_strdup(key);
    e->val = val;
    e->next = h->buckets[idx];
    h->buckets[idx] = e;
    h->size++;
}

void *cfd_hash_get(cfd_hash_t *h, const char *key) {
    size_t idx = hash_key(key, h->capacity);
    for (cfd_hash_entry_t *e = h->buckets[idx]; e; e = e->next)
        if (strcmp(e->key, key) == 0) return e->val;
    return NULL;
}

bool cfd_hash_has(cfd_hash_t *h, const char *key) {
    size_t idx = hash_key(key, h->capacity);
    for (cfd_hash_entry_t *e = h->buckets[idx]; e; e = e->next)
        if (strcmp(e->key, key) == 0) return true;
    return false;
}

bool cfd_hash_del(cfd_hash_t *h, const char *key) {
    size_t idx = hash_key(key, h->capacity);
    cfd_hash_entry_t **pp = &h->buckets[idx];
    while (*pp) {
        if (strcmp((*pp)->key, key) == 0) {
            cfd_hash_entry_t *dead = *pp;
            *pp = dead->next;
            if (h->free_fn && dead->val) h->free_fn(dead->val);
            cfd_free(dead->key);
            cfd_free(dead);
            h->size--;
            return true;
        }
        pp = &(*pp)->next;
    }
    return false;
}

void cfd_hash_clear(cfd_hash_t *h) {
    for (size_t i = 0; i < h->capacity; i++) {
        cfd_hash_entry_t *e = h->buckets[i];
        while (e) {
            cfd_hash_entry_t *next = e->next;
            if (h->free_fn && e->val) h->free_fn(e->val);
            cfd_free(e->key);
            cfd_free(e);
            e = next;
        }
        h->buckets[i] = NULL;
    }
    h->size = 0;
}

size_t cfd_hash_size(const cfd_hash_t *h) {
    return h ? h->size : 0;
}

char **cfd_hash_keys(cfd_hash_t *h, size_t *count) {
    char **keys = cfd_malloc((h->size + 1) * sizeof(char *));
    size_t n = 0;
    for (size_t i = 0; i < h->capacity; i++)
        for (cfd_hash_entry_t *e = h->buckets[i]; e; e = e->next)
            keys[n++] = cfd_strdup(e->key);
    keys[n] = NULL;
    if (count) *count = n;
    return keys;
}

void cfd_hash_foreach(cfd_hash_t *h,
                      void (*fn)(const char *key, void *val, void *ctx),
                      void *ctx) {
    for (size_t i = 0; i < h->capacity; i++)
        for (cfd_hash_entry_t *e = h->buckets[i]; e; e = e->next)
            fn(e->key, e->val, ctx);
}
