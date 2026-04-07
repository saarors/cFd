#include "repl.h"
#include "config.h"
#include "../ui/prompt.h"
#include "../ui/display.h"
#include "../utils/mem.h"
#include "../utils/str_utils.h"
#include <stdio.h>
#include <string.h>

cfd_repl_t *cfd_repl_new(cfd_session_t *sess) {
    cfd_repl_t *r = CFD_NEW(cfd_repl_t);
    r->sess  = sess;
    r->input = cfd_input_new(sess);
    return r;
}

void cfd_repl_free(cfd_repl_t *repl) {
    if (!repl) return;
    cfd_input_free(repl->input);
    cfd_free(repl);
}

int cfd_repl_eval(cfd_repl_t *repl, const char *line) {
    if (!line) return 0;
    const char *trimmed = line;
    while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
    if (!*trimmed || *trimmed == '#') return 0;
    return cfd_session_exec_string(repl->sess, trimmed);
}

int cfd_repl_run(cfd_repl_t *repl) {
    while (repl->sess->running) {
        /* Build prompt */
        char *prompt = cfd_prompt_build(repl->sess);

        char *line = cfd_input_readline(repl->input, prompt);
        cfd_free(prompt);

        if (!line) {
            /* EOF / Ctrl-D */
            printf("\n");
            break;
        }

        int ret = cfd_repl_eval(repl, line);
        cfd_free(line);
        (void)ret;
    }
    return repl->sess->exit_code;
}
