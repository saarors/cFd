#ifndef CFD_INPUT_H
#define CFD_INPUT_H

#include "../../include/types.h"
#include "readline.h"
#include "history.h"
#include "completion.h"

typedef struct cfd_input {
    cfd_readline_t *readline;
    cfd_history_t  *history;
    cfd_session_t  *sess;
} cfd_input_t;

cfd_input_t *cfd_input_new(cfd_session_t *sess);
void         cfd_input_free(cfd_input_t *inp);

/* Read a line from the user */
char *cfd_input_readline(cfd_input_t *inp, const char *prompt);

/* Programmatic line injection (for scripting) */
void  cfd_input_inject(cfd_input_t *inp, const char *line);

#endif /* CFD_INPUT_H */
