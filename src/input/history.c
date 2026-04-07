#include "history.h"
#include "../utils/mem.h"
#include "../utils/str_utils.h"
#include "../../include/config.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

cfd_history_t *cfd_history_new(int capacity, const char *histfile) {
    cfd_history_t *h = CFD_NEW(cfd_history_t);
    h->capacity = capacity > 0 ? capacity : CFD_MAX_HISTORY;
    h->entries  = cfd_calloc(h->capacity, sizeof(char *));
    h->histfile = histfile ? cfd_strdup(histfile) : NULL;
    h->pos      = -1;
    return h;
}

void cfd_history_free(cfd_history_t *h) {
    if (!h) return;
    for (int i = 0; i < h->count; i++) cfd_free(h->entries[i]);
    cfd_free(h->entries);
    cfd_free(h->histfile);
    cfd_free(h);
}

void cfd_history_add(cfd_history_t *h, const char *line) {
    if (!line || !*line) return;
    /* Avoid duplicate consecutive entries */
    if (h->count > 0 && strcmp(h->entries[h->count - 1], line) == 0) {
        h->pos = h->count;
        return;
    }
    if (h->count >= h->capacity) {
        /* shift out oldest */
        cfd_free(h->entries[0]);
        memmove(h->entries, h->entries + 1, (h->capacity - 1) * sizeof(char *));
        h->count--;
    }
    h->entries[h->count++] = cfd_strdup(line);
    h->pos = h->count;
}

const char *cfd_history_prev(cfd_history_t *h) {
    if (h->count == 0) return NULL;
    if (h->pos > 0) h->pos--;
    return h->entries[h->pos];
}

const char *cfd_history_next(cfd_history_t *h) {
    if (h->pos < h->count - 1) {
        h->pos++;
        return h->entries[h->pos];
    }
    h->pos = h->count;
    return NULL;
}

void cfd_history_reset_pos(cfd_history_t *h) {
    h->pos = h->count;
}

int cfd_history_load(cfd_history_t *h) {
    if (!h->histfile) return -1;
    FILE *f = fopen(h->histfile, "r");
    if (!f) return -1;
    char line[CFD_MAX_INPUT];
    while (fgets(line, sizeof(line), f)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) line[--len] = '\0';
        if (*line) cfd_history_add(h, line);
    }
    fclose(f);
    return 0;
}

int cfd_history_save(cfd_history_t *h) {
    if (!h->histfile) return -1;
    FILE *f = fopen(h->histfile, "w");
    if (!f) return -1;
    int start = h->count > CFD_MAX_HISTORY ? h->count - CFD_MAX_HISTORY : 0;
    for (int i = start; i < h->count; i++)
        fprintf(f, "%s\n", h->entries[i]);
    fclose(f);
    return 0;
}

int cfd_history_count(const cfd_history_t *h) { return h ? h->count : 0; }

const char *cfd_history_get(const cfd_history_t *h, int index) {
    if (!h || index < 0 || index >= h->count) return NULL;
    return h->entries[index];
}

void cfd_history_search(cfd_history_t *h, const char *prefix,
                        char **results, int max, int *found) {
    *found = 0;
    if (!h || !prefix) return;
    size_t plen = strlen(prefix);
    for (int i = h->count - 1; i >= 0 && *found < max; i--) {
        if (strncmp(h->entries[i], prefix, plen) == 0)
            results[(*found)++] = h->entries[i];
    }
}
