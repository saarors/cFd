#ifndef CFD_LIST_H
#define CFD_LIST_H

#include <stddef.h>
#include <stdbool.h>

typedef void (*cfd_list_free_fn)(void *data);
typedef int  (*cfd_list_cmp_fn)(const void *a, const void *b);

typedef struct cfd_list_node {
    void               *data;
    struct cfd_list_node *prev;
    struct cfd_list_node *next;
} cfd_list_node_t;

typedef struct cfd_list {
    cfd_list_node_t *head;
    cfd_list_node_t *tail;
    size_t            size;
    cfd_list_free_fn  free_fn;
} cfd_list_t;

cfd_list_t *cfd_list_new(cfd_list_free_fn free_fn);
void        cfd_list_free(cfd_list_t *list);

void        cfd_list_push_back(cfd_list_t *list, void *data);
void        cfd_list_push_front(cfd_list_t *list, void *data);
void       *cfd_list_pop_back(cfd_list_t *list);
void       *cfd_list_pop_front(cfd_list_t *list);

void       *cfd_list_get(cfd_list_t *list, size_t index);
bool        cfd_list_remove(cfd_list_t *list, void *data, cfd_list_cmp_fn cmp);
void        cfd_list_clear(cfd_list_t *list);

bool        cfd_list_empty(const cfd_list_t *list);
size_t      cfd_list_size(const cfd_list_t *list);

void        cfd_list_foreach(cfd_list_t *list, void (*fn)(void *data, void *ctx), void *ctx);
void        cfd_list_sort(cfd_list_t *list, cfd_list_cmp_fn cmp);

#endif /* CFD_LIST_H */
