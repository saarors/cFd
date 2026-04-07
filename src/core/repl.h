#ifndef CFD_REPL_H
#define CFD_REPL_H

#include "session.h"
#include "../input/input.h"

typedef struct cfd_repl {
    cfd_session_t *sess;
    cfd_input_t   *input;
} cfd_repl_t;

cfd_repl_t *cfd_repl_new(cfd_session_t *sess);
void        cfd_repl_free(cfd_repl_t *repl);

/* Run the read-eval-print loop until session exits */
int cfd_repl_run(cfd_repl_t *repl);

/* Process a single line */
int cfd_repl_eval(cfd_repl_t *repl, const char *line);

#endif /* CFD_REPL_H */
