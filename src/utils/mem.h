#ifndef CFD_MEM_H
#define CFD_MEM_H

#include <stddef.h>

void *cfd_malloc(size_t size);
void *cfd_calloc(size_t n, size_t size);
void *cfd_realloc(void *ptr, size_t size);
void  cfd_free(void *ptr);
char *cfd_strdup(const char *s);
char *cfd_strndup(const char *s, size_t n);

/* Safe macros */
#define CFD_FREE(p)  do { cfd_free(p); (p) = NULL; } while(0)
#define CFD_NEW(T)   ((T *)cfd_calloc(1, sizeof(T)))

#endif /* CFD_MEM_H */
