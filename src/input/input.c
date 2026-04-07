#include "input.h"
#include "../utils/mem.h"
#include "../utils/path.h"
#include "../platform/platform.h"
#include "../../include/config.h"
#include <string.h>
#include <stdio.h>

cfd_input_t *cfd_input_new(cfd_session_t *sess) {
    cfd_input_t *inp = CFD_NEW(cfd_input_t);
    inp->sess = sess;

    /* Determine history file path */
    char *home = cfd_platform_get_home();
    char *histfile = cfd_path_join(home, CFD_HISTFILE);
    cfd_free(home);

    inp->history  = cfd_history_new(CFD_MAX_HISTORY, histfile);
    cfd_free(histfile);
    cfd_history_load(inp->history);

    inp->readline = cfd_readline_new(sess, inp->history);
    return inp;
}

void cfd_input_free(cfd_input_t *inp) {
    if (!inp) return;
    cfd_history_save(inp->history);
    cfd_history_free(inp->history);
    cfd_readline_free(inp->readline);
    cfd_free(inp);
}

char *cfd_input_readline(cfd_input_t *inp, const char *prompt) {
    char *line = cfd_readline_read(inp->readline, prompt);
    if (line && *line) {
        cfd_history_add(inp->history, line);
    }
    return line;
}

void cfd_input_inject(cfd_input_t *inp, const char *line) {
    if (line && *line) cfd_history_add(inp->history, line);
}
