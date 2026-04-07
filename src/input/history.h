#ifndef CFD_HISTORY_H
#define CFD_HISTORY_H

#include <stdbool.h>
#include <stddef.h>

typedef struct cfd_history {
    char  **entries;
    int     capacity;
    int     count;
    int     pos;       /* current browsing position */
    char   *histfile;
} cfd_history_t;

cfd_history_t *cfd_history_new(int capacity, const char *histfile);
void           cfd_history_free(cfd_history_t *h);

void           cfd_history_add(cfd_history_t *h, const char *line);
const char    *cfd_history_prev(cfd_history_t *h);
const char    *cfd_history_next(cfd_history_t *h);
void           cfd_history_reset_pos(cfd_history_t *h);

int            cfd_history_load(cfd_history_t *h);
int            cfd_history_save(cfd_history_t *h);

int            cfd_history_count(const cfd_history_t *h);
const char    *cfd_history_get(const cfd_history_t *h, int index);

void           cfd_history_search(cfd_history_t *h, const char *prefix,
                                  char **results, int max, int *found);

#endif /* CFD_HISTORY_H */
