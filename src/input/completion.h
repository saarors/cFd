#ifndef CFD_COMPLETION_H
#define CFD_COMPLETION_H

#include "../../include/types.h"
#include <stddef.h>

typedef struct cfd_completion_result {
    char  **matches;
    int     count;
    int     common_prefix_len;
} cfd_completion_result_t;

void                    cfd_completion_init(cfd_session_t *sess);
cfd_completion_result_t cfd_completion_get(cfd_session_t *sess,
                                           const char *line, int cursor);
void                    cfd_completion_result_free(cfd_completion_result_t *r);

/* Register extra completions */
void cfd_completion_add_word(const char *word);

/* Complete filenames */
char **cfd_complete_filenames(const char *prefix, int *count);

/* Complete command names */
char **cfd_complete_commands(cfd_session_t *sess, const char *prefix, int *count);

#endif /* CFD_COMPLETION_H */
