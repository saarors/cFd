#ifndef CFD_TERMINAL_H
#define CFD_TERMINAL_H

#include "session.h"
#include "repl.h"
#include "config.h"

typedef struct cfd_terminal {
    cfd_session_t *sess;
    cfd_repl_t    *repl;
    cfd_config_t  *config;
} cfd_terminal_t;

cfd_terminal_t *cfd_terminal_new(void);
void            cfd_terminal_free(cfd_terminal_t *term);

int cfd_terminal_init(cfd_terminal_t *term);
int cfd_terminal_run(cfd_terminal_t *term);
int cfd_terminal_run_file(cfd_terminal_t *term, const char *script);
void cfd_terminal_shutdown(cfd_terminal_t *term);

#endif /* CFD_TERMINAL_H */
