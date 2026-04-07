#ifndef CFD_READLINE_H
#define CFD_READLINE_H

#include "../../include/types.h"
#include "../../include/config.h"
#include "history.h"
#include "completion.h"
#include <stddef.h>

typedef struct cfd_readline {
    char         buf[CFD_MAX_INPUT];
    int          len;
    int          cursor;
    cfd_history_t *history;
    cfd_session_t *sess;
    char         kill_ring[CFD_MAX_INPUT]; /* for Ctrl-K / Ctrl-Y */
    char         search_buf[CFD_MAX_INPUT];
    int          search_mode;
} cfd_readline_t;

cfd_readline_t *cfd_readline_new(cfd_session_t *sess, cfd_history_t *history);
void            cfd_readline_free(cfd_readline_t *rl);

/* Read a full line with editing. Returns malloc'd string (caller frees), NULL on EOF */
char *cfd_readline_read(cfd_readline_t *rl, const char *prompt);

#endif /* CFD_READLINE_H */
