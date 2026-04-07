#include "mem.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void *cfd_malloc(size_t size) {
    void *p = malloc(size);
    if (!p && size > 0) {
        fprintf(stderr, "cFd: out of memory\n");
        exit(1);
    }
    return p;
}

void *cfd_calloc(size_t n, size_t size) {
    void *p = calloc(n, size);
    if (!p && n > 0 && size > 0) {
        fprintf(stderr, "cFd: out of memory\n");
        exit(1);
    }
    return p;
}

void *cfd_realloc(void *ptr, size_t size) {
    void *p = realloc(ptr, size);
    if (!p && size > 0) {
        fprintf(stderr, "cFd: out of memory\n");
        exit(1);
    }
    return p;
}

void cfd_free(void *ptr) {
    if (ptr) free(ptr);
}

char *cfd_strdup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char *dup = cfd_malloc(len);
    memcpy(dup, s, len);
    return dup;
}

char *cfd_strndup(const char *s, size_t n) {
    if (!s) return NULL;
    size_t len = strlen(s);
    if (len > n) len = n;
    char *dup = cfd_malloc(len + 1);
    memcpy(dup, s, len);
    dup[len] = '\0';
    return dup;
}
