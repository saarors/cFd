#include "terminal.h"
#include "../commands/registry.h"
#include "../platform/common.h"
#include "../platform/platform.h"
#include "../scripting/script.h"
#include "../ui/ui.h"
#include "../utils/mem.h"
#include "../utils/log.h"
#include <stdio.h>
#include <string.h>

cfd_terminal_t *cfd_terminal_new(void) {
    cfd_terminal_t *t = CFD_NEW(cfd_terminal_t);
    t->config = cfd_config_new();
    t->sess   = cfd_session_new();
    t->repl   = cfd_repl_new(t->sess);
    return t;
}

void cfd_terminal_free(cfd_terminal_t *term) {
    if (!term) return;
    cfd_repl_free(term->repl);
    cfd_session_free(term->sess);
    cfd_config_free(term->config);
    cfd_free(term);
}

int cfd_terminal_init(cfd_terminal_t *term) {
    /* Platform init (enable ANSI on Windows) */
    cfd_platform_init();

    /* UI init */
    cfd_ui_init();

    /* Command registry */
    cfd_registry_init();

    /* Load config */
    cfd_config_load(term->config, term->config->rcfile);
    cfd_config_apply(term->config, term->sess);

    /* Set terminal title */
    cfd_ui_set_title(CFD_NAME);

    return 0;
}

int cfd_terminal_run(cfd_terminal_t *term) {
    /* Run startup RC script */
    cfd_script_run_rc(term->sess);

    /* Print banner */
    if (term->config->show_banner)
        cfd_ui_print_banner();

    /* Start REPL */
    int ret = cfd_repl_run(term->repl);
    return ret;
}

int cfd_terminal_run_file(cfd_terminal_t *term, const char *script) {
    return cfd_script_run_file(term->sess, script);
}

void cfd_terminal_shutdown(cfd_terminal_t *term) {
    cfd_ui_shutdown();
    cfd_platform_shutdown();
    cfd_registry_shutdown();
    cfd_log_close();
    (void)term;
}
