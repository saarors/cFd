#include "list.h"
#include "mem.h"
#include <stdlib.h>

cfd_list_t *cfd_list_new(cfd_list_free_fn free_fn) {
    cfd_list_t *list = CFD_NEW(cfd_list_t);
    list->free_fn = free_fn;
    return list;
}

void cfd_list_free(cfd_list_t *list) {
    if (!list) return;
    cfd_list_clear(list);
    cfd_free(list);
}

static cfd_list_node_t *node_new(void *data) {
    cfd_list_node_t *n = CFD_NEW(cfd_list_node_t);
    n->data = data;
    return n;
}

void cfd_list_push_back(cfd_list_t *list, void *data) {
    cfd_list_node_t *n = node_new(data);
    if (!list->tail) {
        list->head = list->tail = n;
    } else {
        n->prev = list->tail;
        list->tail->next = n;
        list->tail = n;
    }
    list->size++;
}

void cfd_list_push_front(cfd_list_t *list, void *data) {
    cfd_list_node_t *n = node_new(data);
    if (!list->head) {
        list->head = list->tail = n;
    } else {
        n->next = list->head;
        list->head->prev = n;
        list->head = n;
    }
    list->size++;
}

void *cfd_list_pop_back(cfd_list_t *list) {
    if (!list->tail) return NULL;
    cfd_list_node_t *n = list->tail;
    void *data = n->data;
    list->tail = n->prev;
    if (list->tail) list->tail->next = NULL;
    else list->head = NULL;
    cfd_free(n);
    list->size--;
    return data;
}

void *cfd_list_pop_front(cfd_list_t *list) {
    if (!list->head) return NULL;
    cfd_list_node_t *n = list->head;
    void *data = n->data;
    list->head = n->next;
    if (list->head) list->head->prev = NULL;
    else list->tail = NULL;
    cfd_free(n);
    list->size--;
    return data;
}

void *cfd_list_get(cfd_list_t *list, size_t index) {
    cfd_list_node_t *n = list->head;
    for (size_t i = 0; i < index && n; i++) n = n->next;
    return n ? n->data : NULL;
}

bool cfd_list_remove(cfd_list_t *list, void *data, cfd_list_cmp_fn cmp) {
    for (cfd_list_node_t *n = list->head; n; n = n->next) {
        if (cmp ? cmp(n->data, data) == 0 : n->data == data) {
            if (n->prev) n->prev->next = n->next; else list->head = n->next;
            if (n->next) n->next->prev = n->prev; else list->tail = n->prev;
            if (list->free_fn) list->free_fn(n->data);
            cfd_free(n);
            list->size--;
            return true;
        }
    }
    return false;
}

void cfd_list_clear(cfd_list_t *list) {
    cfd_list_node_t *n = list->head;
    while (n) {
        cfd_list_node_t *next = n->next;
        if (list->free_fn) list->free_fn(n->data);
        cfd_free(n);
        n = next;
    }
    list->head = list->tail = NULL;
    list->size = 0;
}

bool cfd_list_empty(const cfd_list_t *list) {
    return !list || list->size == 0;
}

size_t cfd_list_size(const cfd_list_t *list) {
    return list ? list->size : 0;
}

void cfd_list_foreach(cfd_list_t *list, void (*fn)(void *data, void *ctx), void *ctx) {
    for (cfd_list_node_t *n = list->head; n; n = n->next) fn(n->data, ctx);
}

void cfd_list_sort(cfd_list_t *list, cfd_list_cmp_fn cmp) {
    if (!list || list->size < 2) return;
    /* Collect into array, sort, rebuild */
    size_t n = list->size;
    void **arr = cfd_malloc(n * sizeof(void *));
    size_t i = 0;
    for (cfd_list_node_t *nd = list->head; nd; nd = nd->next) arr[i++] = nd->data;
    /* Bubble sort for simplicity */
    for (size_t a = 0; a < n - 1; a++)
        for (size_t b = 0; b < n - a - 1; b++)
            if (cmp(arr[b], arr[b+1]) > 0) { void *t = arr[b]; arr[b] = arr[b+1]; arr[b+1] = t; }
    i = 0;
    for (cfd_list_node_t *nd = list->head; nd; nd = nd->next) nd->data = arr[i++];
    cfd_free(arr);
}
